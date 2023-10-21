/*
 * This file is part of the "Lua Extentions" distribution
 * (https://github.com/yrasik/lproc_queue).
 * Copyright (c) 2023 Yuri Stepanenko.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "fifo.h"
#include "ul_time.h"

#define VERSION		"{ version = '0.1' }"


#include "debug.h"

unsigned int _logout_ = 0;

#define DBG_SYSTEM  DBG_SYSTEM__LPROC_QUEUE
DEBUG_ON(stdout, 0);
//DEBUG_OFF(stdout, DBG_INFO_0 | DBG_INFO_1 | DBG_INFO_2 | DBG_INFO_3);

#ifndef true
	#define true  1
	#define false 0
#endif

#define NSEC_PER_SEC 1000000000

#define ARRAY_SIZE     256
#define QUEUE_SIZE       256


struct array_s {
	size_t size;
	char array[ARRAY_SIZE];
};


static void copy_to_array(size_t from_size, const char *from_array,
		struct array_s *to) {
	to->size = from_size;
	memcpy(to->array, from_array, to->size);
}


static void ref_from_array(struct array_s *from, size_t *to_size,
		char **to_array) {
	*to_size = from->size;
	*to_array = from->array;
}


FIFO_TYPE(QUEUE_SIZE, struct array_s, queue_t);


struct semaphore_s {
	sem_t semaphore;
	struct semaphore_s *prev, *next;
};


struct queue_s {
	queue_t queue_messages;
	struct queue_s *prev, *next;
};


struct proc_s {
	lua_State *L;
	pthread_t thread;
};


struct semaphore_s *semaphore_enter_point = NULL;
static pthread_mutex_t semaphore_access = PTHREAD_MUTEX_INITIALIZER;

struct queue_s *queue_enter_point = NULL;
static pthread_mutex_t kernel_access = PTHREAD_MUTEX_INITIALIZER;


/* from https://github.com/solemnwarning/timespec.git */
/** \fn struct timespec timespec_normalise(struct timespec ts)
 *  \brief Normalises a timespec structure.
 *
 * Returns a normalised version of a timespec structure, according to the
 * following rules:
 *
 * 1) If tv_nsec is >=1,000,000,00 or <=-1,000,000,000, flatten the surplus
 *    nanoseconds into the tv_sec field.
 *
 * 2) If tv_nsec is negative, decrement tv_sec and roll tv_nsec up to represent
 *    the same value attainable by ADDING nanoseconds to tv_sec.
*/
static struct timespec timespec_normalise(struct timespec ts)
{
	while(ts.tv_nsec >= NSEC_PER_SEC)
	{
		++(ts.tv_sec);
		ts.tv_nsec -= NSEC_PER_SEC;
	}

	while(ts.tv_nsec <= -NSEC_PER_SEC)
	{
		--(ts.tv_sec);
		ts.tv_nsec += NSEC_PER_SEC;
	}

	if(ts.tv_nsec < 0)
	{
		/* Negative nanoseconds isn't valid according to POSIX.
		 * Decrement tv_sec and roll tv_nsec over.
		*/

		--(ts.tv_sec);
		ts.tv_nsec = (NSEC_PER_SEC + ts.tv_nsec);
	}

	return ts;
}

/* from https://github.com/solemnwarning/timespec.git */
/** \fn struct timespec timespec_add(struct timespec ts1, struct timespec ts2)
 *  \brief Returns the result of adding two timespec structures.
*/
static struct timespec timespec_add(struct timespec ts1, struct timespec ts2)
{
	/* Normalise inputs to prevent tv_nsec rollover if whole-second values
	 * are packed in it.
	*/
	ts1 = timespec_normalise(ts1);
	ts2 = timespec_normalise(ts2);

	ts1.tv_sec  += ts2.tv_sec;
	ts1.tv_nsec += ts2.tv_nsec;

	return timespec_normalise(ts1);
}

/* from https://github.com/solemnwarning/timespec.git */
/** \fn struct timespec timespec_from_ms(long milliseconds)
 *  \brief Converts an integer number of milliseconds to a timespec.
*/
static struct timespec timespec_from_ms(long milliseconds)
{
	struct timespec ts = {
		.tv_sec  = (milliseconds / 1000),
		.tv_nsec = (milliseconds % 1000) * 1000000,
	};

	return timespec_normalise(ts);
}


static struct semaphore_s* semaphore_search(struct semaphore_s *semaphore_point) {
	int is_finded = false;
	struct semaphore_s *semaphore_node = semaphore_enter_point;
	do {
		if (semaphore_node == semaphore_point) {
			is_finded = true;
			break;
		}
		semaphore_node = semaphore_node->next;
	} while (semaphore_node != semaphore_enter_point);

	if (is_finded == true) {
		return semaphore_point;
	}

	return NULL;
}


static struct queue_s* queue_search(struct queue_s *queue_point) {
	int is_finded = false;
	struct queue_s *queue_node = queue_enter_point;
	do {
		if (queue_node == queue_point) {
			is_finded = true;
			break;
		}
		queue_node = queue_node->next;
	} while (queue_node != queue_enter_point);

	if (is_finded == true) {
		return queue_point;
	}

	return NULL;
}


static int ll_sem_create(lua_State *L) {
	pthread_mutex_lock(&semaphore_access);
	int inited;
	struct semaphore_s *semaphore_node = NULL;
	if (semaphore_enter_point == NULL) {
		semaphore_node = (struct semaphore_s*) malloc(sizeof(struct semaphore_s));
		if (semaphore_node == NULL) {
			luaL_error(L, "error if( semaphore_node == NULL ): %s",
					lua_tostring(L, -1));
			lua_pushinteger(L, 0);
			pthread_mutex_unlock(&semaphore_access);
			return 1;
		}
		memset(semaphore_node, 0, sizeof(struct semaphore_s));
		semaphore_enter_point = semaphore_node;
		semaphore_node->prev = semaphore_enter_point;
		semaphore_node->next = semaphore_enter_point;

		inited = sem_init(&semaphore_node->semaphore, 0, 0);
		if(inited < 0) {
			luaL_error(L, "error if( pthread_mutex_init(&semaphore_node->semaphore, NULL) < 0 ): %s",
					lua_tostring(L, -1));
			lua_pushinteger(L, 0);
			pthread_mutex_unlock(&semaphore_access);
			return 1;
		}

		lua_pushinteger(L, (lua_Integer) semaphore_node);
		pthread_mutex_unlock(&semaphore_access);
		return 1;
	}

	semaphore_node = (struct semaphore_s*) malloc(sizeof(struct semaphore_s));
	if (semaphore_node == NULL) {
		luaL_error(L, "error if( semaphore_node == NULL ): %s",
				lua_tostring(L, -1));
		lua_pushinteger(L, 0);
		pthread_mutex_unlock(&semaphore_access);
		return 1;
	}

	memset(semaphore_node, 0, sizeof(struct semaphore_s));
	struct semaphore_s *prev = semaphore_enter_point->prev;
	prev->next = semaphore_node;
	semaphore_node->prev = prev;
	semaphore_node->next = semaphore_enter_point;
	semaphore_enter_point->prev = semaphore_node;

	inited = sem_init(&semaphore_node->semaphore, 0, 0);
	if(inited < 0) {
		luaL_error(L, "error if( pthread_mutex_init(&semaphore_node->semaphore, NULL) < 0 ): %s",
				lua_tostring(L, -1));
		lua_pushinteger(L, 0);
		pthread_mutex_unlock(&semaphore_access);
		return 1;
	}

	lua_pushinteger(L, (lua_Integer) semaphore_node);
	pthread_mutex_unlock(&semaphore_access);
	return 1;
}


static int ll_sem_destroy(lua_State *L) {
	pthread_mutex_lock(&semaphore_access);
	if (semaphore_enter_point == NULL) {
		luaL_error(L, "error if(semaphore_enter_point == NULL): %s",
				lua_tostring(L, -1));
		lua_pushboolean(L, 0);
		pthread_mutex_unlock(&semaphore_access);
		return 1;
	}

	struct semaphore_s *semaphore_point = (struct semaphore_s*) luaL_checkinteger(L, 1);
	semaphore_point = semaphore_search(semaphore_point);

	if (semaphore_point == NULL) {
		//luaL_error(L, "error if( semaphore_point == NULL ): %s", lua_tostring(L, -1));
		info_3("WARNING: if (semaphore_point == NULL)\n");
		lua_pushboolean(L, 0);
		pthread_mutex_unlock(&semaphore_access);
		return 1;
	}

	struct semaphore_s *prev = semaphore_point->prev;
	struct semaphore_s *next = semaphore_point->next;

	prev->next = next;
	next->prev = prev;

	if (semaphore_point == semaphore_enter_point) { /* INFO: Мы хотим удалить элемент - точку входа */
		semaphore_enter_point = semaphore_enter_point->next;
	}

	sem_destroy(&semaphore_point->semaphore);
	free(semaphore_point);
	if (prev == next) { /* INFO: Мы удалили единственный оставшийся элемент */
		semaphore_enter_point = NULL;
	}

	lua_pushboolean(L, 1);
	pthread_mutex_unlock(&semaphore_access);
	return 1;
}


static int ll_sem_post(lua_State *L) {
	pthread_mutex_lock(&semaphore_access);
	int ret;
	if (semaphore_enter_point == NULL) {
		luaL_error(L, "error if(semaphore_enter_point == NULL): %s",
				lua_tostring(L, -1));
		lua_pushboolean(L, 0);
		pthread_mutex_unlock(&semaphore_access);
		return 1;
	}
	struct semaphore_s *semaphore_point = (struct semaphore_s*) luaL_checkinteger(L, 1);
	semaphore_point = semaphore_search(semaphore_point);

	if (semaphore_point == NULL) {
		//luaL_error(L, "error if( semaphore_point == NULL ): %s", lua_tostring(L, -1));
		info_3("WARNING: if (semaphore_point == NULL)\n");
		lua_pushboolean(L, 0);
		pthread_mutex_unlock(&semaphore_access);
		return 1;
	}
	pthread_mutex_unlock(&semaphore_access);

	ret = sem_post(&semaphore_point->semaphore);
	if(ret < 0) {
		luaL_error(L, "error sem_post(&semaphore_point->semaphore) < 0 ): %s",
				lua_tostring(L, -1));
		lua_pushboolean(L, 0);
		return 1;
	}

	lua_pushboolean(L, 1);
	return 1;
}


static int ll_sem_timedwait_ms(lua_State *L) {
	pthread_mutex_lock(&semaphore_access);
	int ret;
	struct timespec Time;
	struct timespec SemTime;
	if (semaphore_enter_point == NULL) {
		luaL_error(L, "error if(semaphore_enter_point == NULL): %s",
				lua_tostring(L, -1));
		lua_pushboolean(L, 0);
		lua_pushstring(L, "error");
		pthread_mutex_unlock(&semaphore_access);
		return 2;
	}
	struct semaphore_s *semaphore_point = (struct semaphore_s*) luaL_checkinteger(L, 1);
	semaphore_point = semaphore_search(semaphore_point);

	if (semaphore_point == NULL) {
		//luaL_error(L, "error if( semaphore_point == NULL ): %s", lua_tostring(L, -1));
		info_3("WARNING: if (semaphore_point == NULL)\n");
		lua_pushboolean(L, 0);
		lua_pushstring(L, "error");
		pthread_mutex_unlock(&semaphore_access);
		return 2;
	}

	clock_gettime(CLOCK_REALTIME, &Time);

	long milliseconds = (long) luaL_checkinteger(L, 2);
	SemTime = timespec_from_ms(milliseconds);
	SemTime = timespec_add(Time, SemTime);
	pthread_mutex_unlock(&semaphore_access);

	ret = sem_timedwait(&semaphore_point->semaphore, &SemTime);
	if(ret < 0) {
		if( (ret == -1) && (errno == ETIMEDOUT)) {
			lua_pushboolean(L, 0);
			lua_pushstring(L, "timeout");
		}
		else {
			luaL_error(L, "error sem_timedwait(&semaphore_point->semaphore, &SemTime) < 0 ): %s",
				lua_tostring(L, -1));
			lua_pushboolean(L, 0);
			lua_pushstring(L, "error");
		}
		return 2;
	}

	lua_pushboolean(L, 1);
	return 1;
}


static int ll_queue_create(lua_State *L) {
	pthread_mutex_lock(&kernel_access);
	struct queue_s *queue_node = NULL;
	if (queue_enter_point == NULL) {
		queue_node = (struct queue_s*) malloc(sizeof(struct queue_s));
		if (queue_node == NULL) {
			luaL_error(L, "error if( queue_node == NULL ): %s",
					lua_tostring(L, -1));
			lua_pushinteger(L, 0);
			pthread_mutex_unlock(&kernel_access);
			return 1;
		}
		memset(queue_node, 0, sizeof(struct queue_s));
		queue_enter_point = queue_node;
		queue_node->prev = queue_enter_point;
		queue_node->next = queue_enter_point;
		FIFO_FLUSH(queue_node->queue_messages);
		lua_pushinteger(L, (lua_Integer) queue_node);
		pthread_mutex_unlock(&kernel_access);
		return 1;
	}

	queue_node = (struct queue_s*) malloc(sizeof(struct queue_s));
	if (queue_node == NULL) {
		luaL_error(L, "error if( queue_node == NULL ): %s",
				lua_tostring(L, -1));
		lua_pushinteger(L, 0);
		pthread_mutex_unlock(&kernel_access);
		return 1;
	}

	memset(queue_node, 0, sizeof(struct queue_s));
	struct queue_s *prev = queue_enter_point->prev;
	prev->next = queue_node;
	queue_node->prev = prev;
	queue_node->next = queue_enter_point;
	queue_enter_point->prev = queue_node;

	FIFO_FLUSH(queue_node->queue_messages);
	lua_pushinteger(L, (lua_Integer) queue_node);
	pthread_mutex_unlock(&kernel_access);
	return 1;
}


static int ll_queue_destroy(lua_State *L) {
	pthread_mutex_lock(&kernel_access);
	if (queue_enter_point == NULL) {
		luaL_error(L, "error if(queue_enter_point == NULL): %s",
				lua_tostring(L, -1));
		lua_pushboolean(L, 0);
		pthread_mutex_unlock(&kernel_access);
		return 1;
	}

	struct queue_s *queue_point = (struct queue_s*) luaL_checkinteger(L, 1);
	queue_point = queue_search(queue_point);

	if (queue_point == NULL) {
		//luaL_error(L, "error if( queue_point == NULL ): %s", lua_tostring(L, -1));
		info_3("WARNING: if (queue_point == NULL)\n");
		lua_pushboolean(L, 0);
		pthread_mutex_unlock(&kernel_access);
		return 1;
	}

	struct queue_s *prev = queue_point->prev;
	struct queue_s *next = queue_point->next;

	prev->next = next;
	next->prev = prev;

	if (queue_point == queue_enter_point) { /* INFO: Мы хотим удалить элемент - точку входа */
		queue_enter_point = queue_enter_point->next;
	}

	free(queue_point);
	if (prev == next) { /* INFO: Мы удалили единственный оставшийся элемент */
		queue_enter_point = NULL;
	}

	lua_pushboolean(L, 1);
	pthread_mutex_unlock(&kernel_access);
	return 1;
}


static int ll_queue_push(lua_State *L) {
	pthread_mutex_lock(&kernel_access);
	struct queue_s *queue_point = (struct queue_s*) luaL_checkinteger(L, 1);
	queue_point = queue_search(queue_point);

	if (queue_point == NULL) {
		//luaL_error(L, "error if( queue_point == NULL ): %s", lua_tostring(L, -1));
		info_3("WARNING: if (queue_point == NULL)\n");
		lua_pushboolean(L, 0);
		pthread_mutex_unlock(&kernel_access);
		return 1;
	}

	size_t from_size;
	const char *from_array = luaL_checklstring(L, 2, &from_size);
	from_size = (from_size > ARRAY_SIZE) ? ARRAY_SIZE : from_size;

	if ( FIFO_SPACE(queue_point->queue_messages) == 0) {
		//luaL_error(L, "error fifo is full: %s", lua_tostring(L, -1));
		info_3("WARNING: fifo is full\n");
		lua_pushboolean(L, 0);
		pthread_mutex_unlock(&kernel_access);
		return 1;
	}

	FIFO_PUSH(queue_point->queue_messages, copy_to_array, from_size, from_array);
	lua_pushboolean(L, 1);
	pthread_mutex_unlock(&kernel_access);
	return 1;
}


static int ll_queue_nb_pop(lua_State *L) {
	pthread_mutex_lock(&kernel_access);
	struct queue_s *queue_point = (struct queue_s*) luaL_checkinteger(L, 1);
	//info_3("INFO: queue_point = 0x%016jX\n", (long long int)queue_point);
	queue_point = queue_search(queue_point);

	if (queue_point == NULL) {
		//luaL_error(L, "error if( queue_point == NULL ): %s", lua_tostring(L, -1));
		info_3("WARNING: if (queue_point == NULL)\n");
		lua_pushboolean(L, 0);
		pthread_mutex_unlock(&kernel_access);
		return 1;
	}

	size_t size = FIFO_COUNT(queue_point->queue_messages);
	if (size == 0) {
		lua_pushboolean(L, 0);
		pthread_mutex_unlock(&kernel_access);
		return 1;
	}

	size_t to_size;
	char *to_array;
	FIFO_POP(queue_point->queue_messages, ref_from_array, &to_size, &to_array);
	size--;
	//info_3("INFO: size = %d  to_size = %d\n", (int)size, (int)to_size);
	lua_pushboolean(L, 1);
	lua_pushlstring(L, to_array, to_size);
	lua_pushinteger(L, (lua_Integer) size);
	pthread_mutex_unlock(&kernel_access);
	return 3;
}


static void* ll_thread(void *arg);


static int ll_proc_start(lua_State *L) {
	pthread_t thread;
	const char *chunk = luaL_checkstring(L, 1);
	lua_State *L1 = luaL_newstate();

	if (L1 == NULL)
		luaL_error(L, "unable to create new state");

	if (luaL_loadstring(L1, chunk) != 0)
		luaL_error(L, "error starting thread: %s", lua_tostring(L1, -1));

	if (pthread_create(&thread, NULL, ll_thread, L1) != 0)
		luaL_error(L, "unable to create new thread");

	pthread_detach(thread);
	return 0;
}


int luaopen_lproc_queue(lua_State *L);


static void* ll_thread(void *arg) {
	lua_State *L = (lua_State*) arg;
	luaL_openlibs(L); /* open standard libraries */
	luaL_requiref(L, "lproc_queue", luaopen_lproc_queue, 1);
	lua_pop(L, 1);
	if (lua_pcall(L, 0, 0, 0) != 0) /* call main chunk */
		fprintf(stderr, "thread error: %s", lua_tostring(L, -1));
//  pthread_cond_destroy(&getself(L)->cond);
	lua_close(L);
	return NULL;
}


static int ll_proc_exit(lua_State *L) {
	pthread_exit(NULL);
	return 0;
}


LOGOUT__PARSE;

static int ll_debug_mode(lua_State *L) {
	const char *str_logout = luaL_checkstring(L, 1);
	_logout_ = logout_parse(str_logout);
	PRINT_INFO_SETTINGS;
	lua_pushstring(L, _str_);
	return 1;
}


static int ll_version(lua_State *L) {
	lua_pushstring(L, VERSION);
	return 1;
}


static const luaL_Reg ll_funcs[] = {
  { "delay_us", ul_time_delay_us },
  { "delay_ms", ul_time_delay_ms },
  { "delay_s", ul_time_delay_s },
  { "timeofday", ul_timeofday },
  { "sem_create", ll_sem_create },
  { "sem_destroy", ll_sem_destroy },
  { "sem_post", ll_sem_post },
  { "sem_timedwait_ms", ll_sem_timedwait_ms },
  { "queue_create", ll_queue_create },
  { "queue_destroy", ll_queue_destroy },
  { "queue_push", ll_queue_push },
  { "queue_nb_pop", ll_queue_nb_pop },
  { "proc_start", ll_proc_start },
  { "proc_exit", ll_proc_exit },
  { "debug_mode", ll_debug_mode },
  { "version", ll_version },
  { NULL, NULL }
};


int luaopen_lproc_queue(lua_State *L) {
	/* create own control block */
	struct proc_s *self = (struct proc_s*) lua_newuserdata(L,
			sizeof(struct proc_s));
	lua_setfield(L, LUA_REGISTRYINDEX, "_SELF");
	self->L = L;
	self->thread = pthread_self();
	luaL_newlib(L, ll_funcs); /* open library */
	return 1;
}


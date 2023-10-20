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

#include <stdint.h>
#include <unistd.h>

#include <sys/time.h>

#if defined( _WIN32 )

#else
  #include <time.h>
#endif


#include "ul_time.h"

// converts timeval structure to 64-bit unsigned
#define TV_TO_64(tv) ( (uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec )


inline void delay_us(uint64_t del) { //INFO 3 ��� = 3 000 000 �����
	struct timespec tsp;
	//printf("---> del = %d\n", (int)del);
	tsp.tv_sec = del / 1000000;
	tsp.tv_nsec = (del - (tsp.tv_sec * 1000000)) * 1000;
	nanosleep(&tsp, NULL);
}


// delay in microseconds
int ul_time_delay_us(lua_State *L) {
	if (lua_isinteger(L, 1)) { // 1st parameter is number ?
		delay_us((uint64_t) lua_tointeger(L, 1));
	}
	return 0;
}


// delay in milliseconds
int ul_time_delay_ms(lua_State *L) {
	if (lua_isinteger(L, 1)) { // 1st parameter is number ?
		delay_us((uint64_t) (lua_tointeger(L, 1) * 1000));
	}
	return 0;
}


// delay in seconds
int ul_time_delay_s(lua_State *L) {
	if (lua_isinteger(L, 1)) { // 1st parameter is number ?
		delay_us((uint64_t) (lua_tointeger(L, 1) * 1000000));
	}
	return 0;
}


// returns current seconds and microseconds
int ul_timeofday(lua_State *L) {
	struct timeval tv;             // seconds/useconds structure
	gettimeofday(&tv, NULL);       // get current time
	lua_pushinteger(L, (lua_Integer) tv.tv_sec);  // 1st result
	lua_pushinteger(L, (lua_Integer) tv.tv_usec); // second result
	return 2;                      // number of results
}


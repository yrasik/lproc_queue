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

#ifndef UL_TIME_H
#define UL_TIME_H

#include <stdint.h>

#include "lua.h"
#include "lauxlib.h"


void delay_us( uint64_t del );

// delay in microseconds
int ul_time_delay_us(lua_State *L);

// delay in milliseconds
int ul_time_delay_ms(lua_State *L);

// delay in seconds
int ul_time_delay_s(lua_State *L);

// returns current seconds and microseconds
int ul_timeofday(lua_State *L);

#endif  /* UL_TIME_H */

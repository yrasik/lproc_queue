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

#ifndef FIFO__H
#define FIFO__H

#include <stdint.h>


/*! @file 
	@brief fifo простая очередь
*/

/*!
	@defgroup fifo FIFO
	@brief Простая очередь FIFO
	@{
*/


//!размер должен быть степенью двойки: 4,8,16,32...128...
#define FIFO_TYPE( size , type, name )\
  typedef struct {\
    type buf[size];\
    uint16_t tail;\
    uint16_t head;\
    int16_t count;\
  } name


//!очистить fifo
#define FIFO_FLUSH(fifo)   \
  {\
    fifo.tail = 0;\
    fifo.head = 0;\
    fifo.count = 0;\
  }

//!количество элементов в очереди
#define FIFO_COUNT(fifo)     (fifo.count)
 
//!размер fifo
#define FIFO_SIZE(fifo)      ( sizeof(fifo.buf)/sizeof(fifo.buf[0]) )
 
//!количество свободного места в fifo
#define FIFO_SPACE(fifo)     (FIFO_SIZE(fifo) - FIFO_COUNT(fifo))
 
//!поместить элемент в fifo
#define FIFO_PUSH(fifo, _copy_func_, _from_size_, _from_array_) \
  {\
    _copy_func_(_from_size_, _from_array_, &(fifo.buf[ fifo.head ]));\
    fifo.head++;\
    fifo.head = fifo.head & (FIFO_SIZE(fifo) - 1);\
    fifo.count++;\
  }
 
//!взять первый элемент из fifo
#define FIFO_POP(fifo, _copy_func_, _to_size_, _to_array_) \
  {  _copy_func_(&(fifo.buf[fifo.tail]), _to_size_, _to_array_); \
     fifo.tail++;\
     fifo.tail = fifo.tail & (FIFO_SIZE(fifo) - 1);\
     fifo.count--;\
  }
 

//! @}
#endif //FIFO__H

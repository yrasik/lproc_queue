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

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>



#define DBG_SYSTEM__LPROC_QUEUE				(1 << 31)


#define DBG_INFO_3							(1 << 3)
#define DBG_INFO_2							(1 << 2)
#define DBG_INFO_1							(1 << 1)
#define DBG_INFO_0							(1 << 0)

#define PRINT_INFO_SETTINGS                                                               \
char _str_[4096];                                                                         \
if(_logout_)                                                                              \
{                                                                                         \
  int pos = 0;                                                                            \
  pos += sprintf(&_str_[pos], "{\n");                                                     \
  pos += sprintf(&_str_[pos], "  {bit = 31, name = \"DBG_SYSTEM__LPROC_QUEUE\",     value = %s},\n", (_logout_ & DBG_SYSTEM__LPROC_QUEUE)     ? "true " : "false");\
\
  pos += sprintf(&_str_[pos], "  {bit =  3, name = \"DBG_INFO_3\", value = %s},\n", (_logout_ & DBG_INFO_3) ? "true " : "false");\
  pos += sprintf(&_str_[pos], "  {bit =  2, name = \"DBG_INFO_2\", value = %s},\n", (_logout_ & DBG_INFO_2) ? "true " : "false");\
  pos += sprintf(&_str_[pos], "  {bit =  1, name = \"DBG_INFO_1\", value = %s},\n", (_logout_ & DBG_INFO_1) ? "true " : "false");\
  pos += sprintf(&_str_[pos], "  {bit =  0, name = \"DBG_INFO_0\", value = %s},\n", (_logout_ & DBG_INFO_0) ? "true " : "false");\
  pos += sprintf(&_str_[pos], "}\n");                                                     \
};                                                                                        \


#define LOGOUT__PARSE                                                                     \
unsigned int logout_parse(const char *str_logout)                                         \
{                                                                                         \
    unsigned int _logout_ = 0;                                                            \
                                                                                          \
    char str[100];                                                                        \
    char *s = str;                                                                        \
    char *end = NULL;                                                                     \
                                                                                          \
    for (int i = 0; str_logout[i] != '\0'; i++) {                                         \
        if(str_logout[i] != '_') {                                                        \
            *s = str_logout[i];                                                           \
            s++;                                                                          \
        }                                                                                 \
    }                                                                                     \
	*s = '\0';                                                                            \
                                                                                          \
    if( str[0] != '0' ) {                                                                 \
        return _logout_;                                                                  \
    }                                                                                     \
                                                                                          \
    if( str[1] == 'b' ) {                                                                 \
        _logout_ = (uint32_t)strtoul( &str[2], &end, 2 );                                 \
        if ( *end != 0 ) {                                                                \
            return 0;                                                                     \
        } else {                                                                          \
            info_3("INFO: str_logout = 0x%08X\n", _logout_);                              \
        }                                                                                 \
    }                                                                                     \
                                                                                          \
    if( (str[1] == 'x') || (str[1] == 'h') ) {                                            \
        _logout_ = (uint32_t)strtoul( &str[2], &end, 16 );                                \
        if ( *end != 0 ) {                                                                \
            return 0;                                                                     \
        } else {                                                                          \
            info_3("INFO: str_logout_ = 0x%08X\n", _logout_);                             \
        }                                                                                 \
    }                                                                                     \
                                                                                          \
    /*PRINT_INFO_SETTINGS; */                                                             \
    return _logout_;                                                                      \
}                                                                                         \



#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ?                                  \
  __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)                                        \


#define info_0(format, arg...)                                                            \
  do {                                                                                    \
    if ((_flags_ & DBG_INFO_0) || ((_logout_ & DBG_INFO_0) && (_logout_ & DBG_SYSTEM)))   \
      printf_fflush(format, ## arg);                                                      \
  } while (0)


#define info_1(format, arg...)                                                            \
  do {                                                                                    \
    if ((_flags_ & DBG_INFO_1) || ((_logout_ & DBG_INFO_1) && (_logout_ & DBG_SYSTEM)))   \
      printf_fflush(format, ## arg);                                                      \
  } while (0)                                                                             \


#define info_2(format, arg...)                                                            \
  do {                                                                                    \
    if ((_flags_ & DBG_INFO_2) || ((_logout_ & DBG_INFO_2) && (_logout_ & DBG_SYSTEM)))   \
      printf_fflush(format, ## arg);                                                      \
  } while (0)                                                                             \


#define info_3(format, arg...)                                                            \
  do {                                                                                    \
    if ((_flags_ & DBG_INFO_3) || ((_logout_ & DBG_INFO_3) && (_logout_ & DBG_SYSTEM)))   \
      printf_fflush("%s:%d %s(): " format,                                                \
        __FILENAME__, __LINE__, __func__, ## arg);                                        \
  } while (0)                                                                             \


#define info_3_dump(_pointer_, _size_, format, arg...)                                    \
  do {                                                                                    \
    if ((_flags_ & DBG_INFO_3) || ((_logout_ & DBG_INFO_3) && (_logout_ & DBG_SYSTEM))) { \
      int i;                                                                              \
      unsigned char *p = (unsigned char *)_pointer_;                                      \
      printf_fflush("%s:%d %s(): " format,                                                \
        __FILENAME__, __LINE__, __func__, ## arg);                                        \
      printf_fflush("  ");                                                                \
      for(i = 0; i < (int)_size_; i++) {                                                  \
        if ( ((i % 16) == 0) && (i != 0)) {                                               \
          printf_fflush("\n  ");                                                          \
        }                                                                                 \
        printf_fflush("0x%02X ", (unsigned char)(*p));                                    \
        p++;                                                                              \
      }                                                                                   \
      printf_fflush("\n");                                                                \
    }                                                                                     \
  } while (0)                                                                             \


#define DEBUG_ON(_FD_, flags)                                                             \
  extern unsigned int _logout_;                                                           \
  static unsigned int _flags_ = flags;                                                    \
  static void printf_fflush(const char *msg, ...) {                                       \
    va_list arg;                                                                          \
      va_start(arg, msg);                                                                 \
      vfprintf(_FD_, msg, arg);                                                           \
      va_end(arg);                                                                        \
      fflush(_FD_);                                                                       \
  }                                                                                       \


#define DEBUG_OFF(_FD_, flags)                                                            \
  static unsigned int _flags_ = 0;                                                        \
  static inline void printf_fflush(const char *msg, ...) {}                               \


#endif  /* DEBUG_H */


// ============================================================ //
//
//   d88888D db   d8b   db  .d8b.  db    db
//   YP  d8' 88   I8I   88 d8' `8b `8b  d8'
//      d8'  88   I8I   88 88ooo88  `8bd8'
//     d8'   Y8   I8I   88 88~~~88    88
//    d8' db `8b d8'8b d8' 88   88    88
//   d88888P  `8b8' `8d8'  YP   YP    YP
//
//   open-source, cross-platform, crypto-messenger
//
//   Copyright (C) 2018 Marc Weiler
//
//   This library is free software; you can redistribute it and/or
//   modify it under the terms of the GNU Lesser General Public
//   License as published by the Free Software Foundation; either
//   version 2.1 of the License, or (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   Lesser General Public License for more details.
//
// ============================================================ //

#ifndef __ERASE_FROM_MEMORY_H__
#define __ERASE_FROM_MEMORY_H__ 1

#define __STDC_WANT_LIB_EXT1__ 1
#include <stdlib.h>
#include <string.h>

// ============================================================ //

void *erase_from_memory(void *pointer, size_t size_data, size_t size_to_remove) {
  #ifdef __STDC_LIB_EXT1__
   memset_s(pointer, size_data, 0, size_to_remove);
  #else
   if(size_to_remove > size_data) size_to_remove = size_data;
     volatile unsigned char *p = (volatile unsigned char*) pointer;
     while (size_to_remove--){
         *p++ = 0;
     }
  #endif
    return pointer;
}

// ============================================================ //

#endif // __ERASE_FROM_MEMORY_H__

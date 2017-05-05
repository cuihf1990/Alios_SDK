/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef YOC_MEM_IMPL_H
#define YOC_MEM_IMPL_H

#include <stdlib.h>

void *_debug_malloc(unsigned int num_bytes,
                    const char *file, unsigned int line);
void *_debug_realloc(void *ptr, unsigned int num_bytes,
                     const char *file, unsigned int line);
void *_debug_calloc(size_t n, size_t size,
                    const char *file, unsigned int line);
char *_debug_strdup(const char *s,
                    const char* file, unsigned int line);
void  _debug_free(void *ptr);
void  _debug_mem_dump(void);
int   _debug_mem_used_size(void);
int   _debug_mem_used_pointers(void);
void  _debug_mem_usage(void);

#ifdef CONFIG_YOC_DEBUG_MEMORY
#define impl_debug_malloc(n)          _debug_malloc(n, __FUNCTION__, __LINE__)
#define impl_debug_realloc(ptr, n)    _debug_realloc(ptr, n, __FUNCTION__, __LINE__)
#define impl_debug_calloc(n, s)       _debug_calloc(n, s, __FUNCTION__, __LINE__)
#define impl_debug_strdup(s)          _debug_strdup(s, __FUNCTION__, __LINE__)
#define impl_debug_free(p)            _debug_free(p)
#define impl_debug_mem_dump           _debug_mem_dump
#define impl_debug_mem_used_size      _debug_mem_used_size
#define impl_debug_mem_used_pointers  _debug_mem_used_pointers
#ifdef HEAP_CHECK
#define impl_debug_mem_usage()        HEAP_CHECK
#else
#define impl_debug_mem_usage          _debug_mem_usage
#endif

#else
#define impl_debug_malloc              malloc
#define impl_debug_realloc             realloc
#define impl_debug_calloc              calloc
#define impl_debug_strdup              strdup
#define impl_debug_free                free
#define impl_debug_mem_dump()
#define impl_debug_mem_used_size()     0
#define impl_debug_mem_used_pointers() 0
#ifdef HEAP_CHECK
#define impl_debug_mem_usage()         HEAP_CHECK
#else
#define impl_debug_mem_usage           _debug_mem_usage
#endif

#endif /* YOC_DEBUG_MEMORY */

#endif /* YOC_MEM_IMPL_H */


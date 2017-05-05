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

/**
 * @file yoc/mem.h
 * @brief memory allocation tracing APIs
 * @version since 5.5.0
 *
 * using APIs inlcuding in this file, and enable CONFIG_YOC_DEBUG_MEMORY,
 * will provide a simple memory tracing function
 * basically all of the APIs has some sematics as its counterparts, that is:
 *    DEBUG_MALLOC  -> malloc,
 *    DEBUG_CALLOC  -> calloc,
 *    DEBUG_REALLOC -> realloc,
 *    DEBUG_STRDUP  -> strdup,
 *    DEBUG_FREE    -> free,
 */

#ifndef YOC_MEM_H
#define YOC_MEM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <debug_mem_impl.h>

#define DEBUG_MALLOC(n)          impl_debug_malloc(n)
#define DEBUG_REALLOC(ptr, n)    impl_debug_realloc(ptr, n)
#define DEBUG_CALLOC(n, s)       impl_debug_calloc(n, s)
#define DEBUG_STRDUP(s)          impl_debug_strdup(s)
#define DEBUG_FREE(p)            impl_debug_free(p)
#define DEBUG_MEMDUMP            impl_debug_mem_dump
#define DEBUG_MEM_USED_SIZE      impl_debug_mem_used_size
#define DEBUG_MEM_USED_POINTERS  impl_debug_mem_used_pointers
#define DEBUG_HEAP_CHECK         impl_debug_mem_usage

#ifdef __cplusplus
}
#endif

#endif /* YOC_MEM_H */


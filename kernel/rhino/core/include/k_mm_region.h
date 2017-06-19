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

#ifndef K_MM_REGION_H
#define K_MM_REGION_H

#ifdef HAVE_VALGRIND_H
#include <valgrind.h>
#include <memcheck.h>
#define VGF(X) X
#undef VALGRIND_MAKE_MEM_NOACCESS
#define VALGRIND_MAKE_MEM_NOACCESS(x,y)
#elif defined(HAVE_VALGRIND_VALGRIND_H)
#include <valgrind/valgrind.h>
#include <valgrind/memcheck.h>
#define VGF(X) X
#undef VALGRIND_MAKE_MEM_NOACCESS
#define VALGRIND_MAKE_MEM_NOACCESS(x,y)
#else
#define VGF(X)
#endif

#define YUNOS_MM_REGION_FREE         0
#define YUNOS_MM_REGION_ALLOCED      1
#define YUNOS_MM_REGION_PREVFREE     1
#define YUNOS_MM_REGION_MAX          10
#define YUNOS_MM_REGION_CORRUPT_DYE  0xfefe
#define YUNOS_MM_REGION_MAX_FRAGSIZE 0x0fffffff

typedef struct {
    uint8_t *start;
    size_t   len;
} k_mm_region_t;

typedef struct {
#if (YUNOS_CONFIG_MM_DEBUG > 0)
    size_t   dye;
    size_t   owner;
#endif
    klist_t  list;
    size_t   type: 1;
size_t   len:
    (8 * sizeof(unsigned int) - 1);
} k_mm_region_list_t;

typedef struct {
    klist_t     probe;
    klist_t     alloced;
    klist_t     regionlist;
    size_t      frag_num;
    size_t      freesize;
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 1)
    kmutex_t            mm_region_mutex;
#endif
} k_mm_region_head_t;

/**
 * This function will init the mm region list.
 * @param[in]  regions   pointer to mem regions
 * @param[in]  len       number of  mem regions
 * @param[in]  blk_start blk_pool head for small size malloc
 * @param[in]  blk_size  blk_pool size for small size malloc
 * @param[in]  blk_total blk_pool total size for small size malloc
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_mm_region_init(k_mm_region_head_t *region_head,
                             k_mm_region_t *regions, size_t size);
/**
 * This function will insert the momory to free list.
 * @param[in]  regions   pointer to mem regions
 * @param[in]  node      node need be inserted to free list
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */

kstat_t yunos_mm_region_insert2freelist(k_mm_region_head_t *region_head,
                                        klist_t *node);

/**
 * This function will get the free size of mem
 * @return  the free size of the mem
 */
size_t yunos_mm_region_get_free_size(k_mm_region_head_t *region_head);

/**
 * This function will check if the mem has been corrupted.
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t check_mm_info_func();

/**
 * This function will alloc a mem from a region best fit algorithm
 * @param[in]       region_head     pointer to the memory region
 * @param[out]      mem             pointer to the mem
 * @param[in]       mem_size        size of the mem to malloc
 * @param[in]       allocator      allocator of memory
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
#if (YUNOS_CONFIG_MM_BESTFIT > 0)
kstat_t yunos_mm_bf_alloc(k_mm_region_head_t *region_head, void **mem,
                          size_t size, size_t allocator);
#endif
/**
 * This function will alloc a mem from a region with first fit algorithm
 * @param[in]       region_head     pointer to the memory region
 * @param[out]      mem             pointer to the mem
 * @param[in]       mem_size        size of the mem to malloc
 * @param[in]       allocator      allocator of memory
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
#if (YUNOS_CONFIG_MM_FIRSTFIT > 0)
kstat_t yunos_mm_ff_alloc(k_mm_region_head_t *region_head, void **mem,
                          size_t size, size_t allocator);
#endif

/**
 * This function will free mem of a region which malloc by first-fit or best-fit
 * @param[in]  pool  pointer to the pool
 * @param[in]  mem   pointer to the mem
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
#if (YUNOS_CONFIG_MM_BESTFIT > 0 || YUNOS_CONFIG_MM_FIRSTFIT > 0)
kstat_t yunos_mm_xf_free(k_mm_region_head_t *region_head, void *mem);
#endif

#endif /* K_MM_REGION_H */


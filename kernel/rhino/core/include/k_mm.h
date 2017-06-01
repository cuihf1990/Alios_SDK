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

#ifndef K_MM_H
#define K_MM_H


/*use two level bit map to find free memory block*/

#if (YUNOS_CONFIG_MM_TLF > 0)

#define YUNOS_MM_CORRUPT_DYE  0xfefe

#define MAX_MM_BIT          20 /*2^20=1M,will change it to k_config.h*/
#define FIX_BLK_BIT         5 /*32 bytes*/
#define DEF_FIX_BLK_SIZE    (1<<FIX_BLK_BIT) /*32 bit*/
#define MIN_FLT_BIT         9 /*<2^9 only need one level mapping */
#define MIN_FLT_SIZE        (1<<MIN_FLT_BIT)
#define MAX_LOG2_SLT        5
#define SLT_SIZE            (1<<MAX_LOG2_SLT)
#define FLT_SIZE            (MAX_MM_BIT - MIN_FLT_BIT + 1)


#define MM_ALIGN_MASK     (sizeof(void*) -1)
#define MM_ALIGN_UP(a)   (((a) + MM_ALIGN_MASK) & ~MM_ALIGN_MASK)
#define MM_ALIGN_DOWN(a) ((a) & ~MM_ALIGN_MASK)
#define YUNOS_MM_BLKSIZE_MASK (0xFFFFFFFF - MM_ALIGN_MASK)

#define DEF_TOTAL_FIXEDBLK_SIZE     2048 /*by default, total 2k momory fo fix size block */
#define MIN_FREE_MEMORY_SIZE        1024 /*at least need 1k for user alloced*/

#define MM_ALIGN_MASK     (sizeof(void*) -1)
#define MM_ALIGN_UP(a)   (((a) + MM_ALIGN_MASK) & ~MM_ALIGN_MASK)
#define MM_ALIGN_DOWN(a) ((a) & ~MM_ALIGN_MASK)

/*bit 0 and bit 1 mask*/
#define YUNOS_MM_CURSTAT_MASK 0x1
#define YUNOS_MM_PRESTAT_MASK 0x2

/*bit 0*/
#define YUNOS_MM_FREE         1
#define YUNOS_MM_ALLOCED      0

/*bit 1*/
#define YUNOS_MM_PREVFREE     2
#define YUNOS_MM_PREVALLOCED  0

#define NEXT_MM_BLK(_addr, _r) ((k_mm_list_t *) ((void *) (_addr) + (_r)))

#define MMLIST_HEAD_SIZE   (sizeof(k_mm_list_t) -  sizeof(free_ptr_t))
/*
-------------------------------------------------------------------
| k_mm_list_t |k_mm_region_info_t|k_mm_list_t|free space      |k_mm_list_t|
-------------------------------------------------------------------
*/
#define MMREGION_USED_SIZE (MM_ALIGN_UP(sizeof(k_mm_region_info_t)) + 3 * MMLIST_HEAD_SIZE )

/*struct of memory list ,every memory block include this information*/
typedef struct free_ptr_struct {
    struct k_mm_list_struct *prev;
    struct k_mm_list_struct *next;
} free_ptr_t;

typedef struct k_mm_list_struct{
#if (YUNOS_CONFIG_MM_DEBUG > 0)
    size_t       dye;
    size_t       owner;
#endif
    struct k_mm_list_struct *prev;
    size_t       size;
    /* bit 0 indicates whether the block is used and */
    /* bit 1 allows to know whether the previous block is free */
    union {
        struct free_ptr_struct free_ptr;
        uint8_t                buffer[1];
    } mbinfo;
} k_mm_list_t;

typedef struct k_mm_region_info_struct{
    k_mm_list_t                    *end;
    struct k_mm_region_info_struct *next;
}k_mm_region_info_t;


typedef struct {
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 1)
    kmutex_t            mm_mutex;
#endif
    k_mm_region_info_t *regioninfo;
    k_mm_list_t        *fixedmblk;

#if (K_MM_STATISTIC > 0)
    size_t              used_size;
    size_t              maxused_size;
    size_t              mm_size_stats[MAX_MM_BIT];
#endif

    uint32_t            fl_bitmap;
    uint32_t            sl_bitmap[FLT_SIZE]; /* the second-level bitmap */
    k_mm_list_t        *mm_tbl[FLT_SIZE][SLT_SIZE];
} k_mm_head;


kstat_t yunos_init_mm_head(k_mm_head **ppmmhead, void * addr, size_t len );

kstat_t yunos_add_mm_region(k_mm_head *mmhead, void * addr, size_t len);


void* k_mm_alloc(k_mm_head *mmhead, size_t size);
void  k_mm_free(k_mm_head *mmhead, void *ptr);
void* k_mm_realloc(k_mm_head *mmhead, void *oldmem, size_t new_size);

#endif


#endif /* K_MM_BESTFIT_H */


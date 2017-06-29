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

#include <k_api.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#if (YUNOS_CONFIG_MM_TLF > 0)

#define YOS_MM_ALLOC_DEPTH  2
#define YOS_MM_TLF_ALLOC_MIN_LENGTH  2*sizeof(void *)

typedef enum {
    ACTION_INSERT,
    ACTION_GET
} SEARCH_ACTION;

YUNOS_INLINE k_mm_list_t *init_mm_region(void *regionaddr, size_t len)
{
    k_mm_list_t        *curblk, *lastblk, *firstblk;
    k_mm_region_info_t *region;

    /*mmblk for region info*/
    firstblk = (k_mm_list_t *) regionaddr;
    firstblk->size = MM_ALIGN_UP(sizeof(k_mm_region_info_t)) | YUNOS_MM_ALLOCED |
                     YUNOS_MM_PREVALLOCED;

    curblk = (k_mm_list_t *) ((char *)firstblk->mbinfo.buffer +
                              (firstblk->size & YUNOS_MM_BLKSIZE_MASK));
    curblk->size = MM_ALIGN_DOWN(len - MMREGION_USED_SIZE) | YUNOS_MM_ALLOCED |
                   YUNOS_MM_PREVALLOCED;
    curblk->mbinfo.free_ptr.prev = curblk->mbinfo.free_ptr.next = 0;
    lastblk = NEXT_MM_BLK(curblk->mbinfo.buffer,
                          curblk->size & YUNOS_MM_BLKSIZE_MASK);
    lastblk->prev = curblk;
    lastblk->size = 0 | YUNOS_MM_ALLOCED | YUNOS_MM_PREVFREE;

#if (YUNOS_CONFIG_MM_DEBUG > 0u)
    lastblk->dye  = YUNOS_MM_CORRUPT_DYE;
    firstblk->dye = YUNOS_MM_CORRUPT_DYE;
#endif
    region = (k_mm_region_info_t *) firstblk->mbinfo.buffer;
    region->next = 0;
    region->end = lastblk;
    return firstblk;
}

#if(K_MM_STATISTIC > 0)

static size_t sizetoindex(size_t size)
{
    size_t cnt      = 0;
    cnt = 31 - yunos_find_first_bit(&size);
    return cnt;
}
static void addsize(k_mm_head *mmhead, size_t size, size_t req_size)
{
    size_t index ;

    mmhead->used_size += size;
    mmhead->free_size -= size;
    if (mmhead->used_size > mmhead->maxused_size) {
        mmhead->maxused_size = mmhead->used_size;
    }

    index = sizetoindex(req_size - 1);
    if (index > MAX_MM_BIT) {
        index = MAX_MM_BIT;
    }
    if (index < 1) {
        index = 1;
    }
    mmhead->mm_size_stats[index - 1]++;
}

static void removesize(k_mm_head *mmhead, size_t size)
{
    mmhead->used_size -= size;
    mmhead->free_size += size;

}

#define stats_addsize(mmhead,size, req_size)    addsize(mmhead,size, req_size)
#define stats_removesize(mmhead,size) removesize(mmhead,size)
#else
#define stats_addsize(mmhead,size, req_size)    do{}while(0)
#define stats_removesize(mmhead,size) do{}while(0)
#endif

kstat_t yunos_init_mm_head(k_mm_head **ppmmhead, void *addr, size_t len )
{
    k_mm_list_t *nextblk, *curblk, *firstblk;
    k_mm_head   *pmmhead;
    mblk_pool_t *mmblk_pool;
    kstat_t      stat;
    void        *orig_addr;

    NULL_PARA_CHK(ppmmhead);
    NULL_PARA_CHK(addr);

    /*check paramters, addr need algin with 4 and len should be multiple of 4
      1.  the length at least need DEF_TOTAL_FIXEDBLK_SIZE for fixed size memory block
      2.  and also ast least have 1k for user alloced
    */
    orig_addr = addr;
    addr = (void *) MM_ALIGN_UP((size_t)addr);
    len -= (addr - orig_addr);
    len = MM_ALIGN_DOWN(len);

    if (((unsigned long) addr & MM_ALIGN_MASK) || (len != MM_ALIGN_DOWN(len))) {
        return YUNOS_INV_ALIGN;
    }

    if ( !len || len < MIN_FREE_MEMORY_SIZE + DEF_TOTAL_FIXEDBLK_SIZE
         || len > MAX_MM_SIZE) {
        return YUNOS_MM_POOL_SIZE_ERR;
    }

    pmmhead = (k_mm_head *)addr;

    /* Zeroing the memory head */
    memset(pmmhead, 0, sizeof(k_mm_head));
#if (YUNOS_CONFIG_MM_REGION_MUTEX > 0)
    yunos_mutex_create(&pmmhead->mm_mutex, "mm_mutex");
#endif

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    CPSR_ALLOC();
    YUNOS_CRITICAL_ENTER();
#else
    yunos_mutex_lock(&(pmmhead->mm_mutex), YUNOS_WAIT_FOREVER);
#endif

    if ((VGF(VALGRIND_MEMPOOL_EXISTS(addr)) + 0) == 0) {
#if defined(__VALGRIND_MAJOR__) && defined(__VALGRIND_MINOR__)   \
                        && (__VALGRIND_MAJOR__ > 3                                   \
                            || (__VALGRIND_MAJOR__ == 3 && __VALGRIND_MINOR__ >= 12))
            /*valgrind support VALGRIND_CREATE_MEMPOOL_EXT from 3.12.0*/
            VGF(VALGRIND_CREATE_MEMPOOL_EXT(addr, 0, 0,
                                            VALGRIND_MEMPOOL_METAPOOL | VALGRIND_MEMPOOL_AUTO_FREE));
#else
            VGF(VALGRIND_CREATE_MEMPOOL((uint8_t *)addr, 0, 0));
#endif
    }

    firstblk = init_mm_region(addr + MM_ALIGN_UP(sizeof(k_mm_head)),
                              MM_ALIGN_DOWN(len - sizeof(k_mm_head)));
    pmmhead->regioninfo = (k_mm_region_info_t *) firstblk->mbinfo.buffer;

    nextblk = NEXT_MM_BLK(firstblk->mbinfo.buffer,
                          firstblk->size & YUNOS_MM_BLKSIZE_MASK);

    *ppmmhead = pmmhead;

    /*mark it as free and set it to bitmap*/

    VGF(VALGRIND_MALLOCLIKE_BLOCK(nextblk->mbinfo.buffer, nextblk->size & YUNOS_MM_BLKSIZE_MASK,0 , 0));
    VGF(VALGRIND_MAKE_MEM_DEFINED(nextblk->mbinfo.buffer, nextblk->size & YUNOS_MM_BLKSIZE_MASK));

    nextblk->dye = YUNOS_MM_CORRUPT_DYE;
    k_mm_free(pmmhead, nextblk->mbinfo.buffer);

#if (K_MM_STATISTIC > 0)
    pmmhead->free_size = nextblk->size & YUNOS_MM_BLKSIZE_MASK;
    pmmhead->used_size = len - (nextblk->size & YUNOS_MM_BLKSIZE_MASK);
    pmmhead->maxused_size = pmmhead->used_size;
#endif

    mmblk_pool = k_mm_alloc(pmmhead,
                            DEF_TOTAL_FIXEDBLK_SIZE + MM_ALIGN_UP(sizeof(mblk_pool_t)));
    if (mmblk_pool) {
        curblk = (k_mm_list_t *) ((char *) mmblk_pool - MMLIST_HEAD_SIZE);

        VGF(VALGRIND_FREELIKE_BLOCK(mmblk_pool, 0));
        VGF(VALGRIND_MAKE_MEM_DEFINED(mmblk_pool, curblk->size & YUNOS_MM_BLKSIZE_MASK));

        stat = yunos_mblk_pool_init(mmblk_pool, "fixed_mm_blk",
                                    (void *)mmblk_pool + MM_ALIGN_UP(sizeof(mblk_pool_t)),
                                    DEF_FIX_BLK_SIZE, DEF_TOTAL_FIXEDBLK_SIZE);
        if (stat == YUNOS_SUCCESS) {
            pmmhead->fixedmblk = curblk;
        } else {
            k_mm_free(pmmhead, mmblk_pool);
        }
#if (K_MM_STATISTIC > 0)
        stats_removesize(pmmhead, DEF_TOTAL_FIXEDBLK_SIZE);
#endif
    }


#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
        YUNOS_CRITICAL_EXIT();
#else
        yunos_mutex_unlock(&(mmhead->mm_mutex));
#endif

    return YUNOS_SUCCESS;
}


kstat_t yunos_add_mm_region(k_mm_head *mmhead, void *addr, size_t len)
{
    k_mm_region_info_t *ptr, *ptr_prev, *ai;
    k_mm_list_t        *ib0, *b0, *lb0, *ib1, *b1, *lb1, *next_b;

    NULL_PARA_CHK(mmhead);
    NULL_PARA_CHK(addr);

    if (((unsigned long) addr & MM_ALIGN_MASK) || (len != MM_ALIGN_DOWN(len))) {
        return YUNOS_INV_ALIGN;
    }

    if ( !len || len < sizeof(k_mm_region_info_t) + sizeof(k_mm_list_t) * 2) {
        return YUNOS_MM_POOL_SIZE_ERR;
    }

    memset(addr, 0, len);
    ptr = mmhead->regioninfo;
    ptr_prev = 0;

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    CPSR_ALLOC();
    YUNOS_CRITICAL_ENTER();
#else
    yunos_mutex_lock(&(mmhead->mm_mutex), YUNOS_WAIT_FOREVER);
#endif

    ib0 = init_mm_region(addr, len);
    b0  = NEXT_MM_BLK(ib0->mbinfo.buffer, ib0->size & YUNOS_MM_BLKSIZE_MASK);
    lb0 = NEXT_MM_BLK(b0->mbinfo.buffer, b0->size & YUNOS_MM_BLKSIZE_MASK);

    /* Before inserting the new area, we have to merge this area with the
       already existing ones */

    while (ptr) {
        ib1 = (k_mm_list_t *) ((char *) ptr - MMLIST_HEAD_SIZE);
        b1 = NEXT_MM_BLK(ib1->mbinfo.buffer, ib1->size & YUNOS_MM_BLKSIZE_MASK);
        lb1 = ptr->end;

        /* Merging the new area with the next physically contigous one */
        if ((unsigned long) ib1 == (unsigned long) lb0 + MMLIST_HEAD_SIZE) {
            if (mmhead->regioninfo == ptr) {
                mmhead->regioninfo = ptr->next;
                ptr = ptr->next;
            } else {
                ptr_prev->next = ptr->next;
                ptr = ptr->next;
            }

            b0->size = MM_ALIGN_DOWN((b0->size & YUNOS_MM_BLKSIZE_MASK) +
                                     (ib1->size & YUNOS_MM_BLKSIZE_MASK)
                                     + 2 * MMLIST_HEAD_SIZE) | YUNOS_MM_ALLOCED | YUNOS_MM_PREVALLOCED;

            b1->prev = b0;
            lb0 = lb1;

            continue;
        }

        /* Merging the new area with the previous physically contigous
           one */
        if ((unsigned long) lb1->mbinfo.buffer == (unsigned long) ib0) {
            if (mmhead->regioninfo == ptr) {
                mmhead->regioninfo = ptr->next;
                ptr = ptr->next;
            } else {
                ptr_prev->next = ptr->next;
                ptr = ptr->next;
            }

            lb1->size = MM_ALIGN_DOWN((b0->size & YUNOS_MM_BLKSIZE_MASK) +
                                      (ib0->size & YUNOS_MM_BLKSIZE_MASK)
                                      + 2 * MMLIST_HEAD_SIZE) | YUNOS_MM_ALLOCED | (lb1->size &
                                                                                    YUNOS_MM_PRESTAT_MASK);
            next_b = NEXT_MM_BLK(lb1->mbinfo.buffer, lb1->size & YUNOS_MM_BLKSIZE_MASK);
            next_b->prev = lb1;
            b0 = lb1;
            ib0 = ib1;

            continue;
        }
        ptr_prev = ptr;
        ptr = ptr->next;
    }

    /* Inserting the area in the list of linked areas */
    ai = (k_mm_region_info_t *) ib0->mbinfo.buffer;
    ai->next = mmhead->regioninfo;
    ai->end = lb0;
    mmhead->regioninfo = ai;

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    YUNOS_CRITICAL_EXIT();
#else
    yunos_mutex_unlock(&(mmhead->mm_mutex));
#endif

#if (YUNOS_CONFIG_MM_DEBUG > 0u)
    b0->dye = YUNOS_MM_CORRUPT_DYE;
#endif
    k_mm_free(mmhead, b0->mbinfo.buffer);

#if (K_MM_STATISTIC > 0)
    mmhead->free_size += b0->size & YUNOS_MM_BLKSIZE_MASK;
#endif

    return YUNOS_SUCCESS;
}

static void *k_mm_smallblk_alloc(k_mm_head *mmhead, size_t size)
{
    kstat_t sta;
    void   *tmp;

    if (!mmhead) {
        return NULL;
    }

    sta = yunos_mblk_alloc((mblk_pool_t *)mmhead->fixedmblk->mbinfo.buffer, &tmp);
    if (sta != YUNOS_SUCCESS) {
        return NULL;
    }

#if (K_MM_STATISTIC > 0)
    stats_addsize(mmhead,DEF_FIX_BLK_SIZE, size);
#endif

    VGF(VALGRIND_MALLOCLIKE_BLOCK(tmp, size, 0, 0));
    VGF(VALGRIND_MAKE_MEM_DEFINED(tmp, size));

    stats_addsize(mmhead,DEF_FIX_BLK_SIZE, size);

    return tmp;
}
static void k_mm_smallblk_free(k_mm_head *mmhead, void *ptr)
{
    kstat_t sta;

    if (!mmhead || !ptr) {
        return;
    }

    VGF(VALGRIND_FREELIKE_BLOCK(ptr, 0));
    VGF(VALGRIND_MAKE_MEM_DEFINED(ptr, DEF_FIX_BLK_SIZE));

    sta = yunos_mblk_free((mblk_pool_t *)mmhead->fixedmblk->mbinfo.buffer, ptr);
    if (sta != YUNOS_SUCCESS) {
        assert(0);
    }

    stats_removesize(mmhead, DEF_FIX_BLK_SIZE);
}

static kstat_t bitmap_search(size_t size , size_t *flt, size_t *slt,
                             SEARCH_ACTION action)
{
    size_t tmp_size     = 0;
    size_t padding_size = 0;
    size_t firstbit     = 0;

    if (size < MIN_FLT_SIZE) {
        *flt = 0;
        *slt = size >> (MIN_FLT_BIT - MAX_LOG2_SLT);
    } else {
        *flt = 0;
        firstbit = 31 - (size_t)yunos_find_first_bit(&size);
        tmp_size = size;
        if (action == ACTION_GET) {
            padding_size = (1 << (firstbit - MAX_LOG2_SLT)) - 1;
            tmp_size = size + padding_size;
            firstbit = 31 - (size_t)yunos_find_first_bit(&tmp_size);
        }
        *flt = firstbit - MIN_FLT_BIT + 1;
        tmp_size = tmp_size - (1 << firstbit);
        *slt = tmp_size >> (firstbit - MAX_LOG2_SLT);
    }
    if (*flt >= FLT_SIZE || *slt >= SLT_SIZE) {
        return YUNOS_MM_FREE_ADDR_ERR;
    }
    return YUNOS_SUCCESS;
}

static size_t find_last_bit(int bitmap)
{
    size_t x, lsbit;

    if (bitmap == 0) {
        return 0;
    }

    x = bitmap & -bitmap;
    lsbit = (size_t)yunos_find_first_bit(&x);
    /*yunos find fist bit return value is left->right as 0-31, but we need left->right as 31 -0*/
    return 31 - lsbit;
}

static  void insert_block (k_mm_head *mmhead, k_mm_list_t *blk, int flt,
                           int slt)
{
    blk->mbinfo.free_ptr.prev = NULL;
    blk->mbinfo.free_ptr.next = mmhead->mm_tbl [flt][slt];

    if (mmhead->mm_tbl[flt][slt]) {
        mmhead->mm_tbl[flt][slt]->mbinfo.free_ptr.prev = blk;
    }

    mmhead->mm_tbl[flt][slt] = blk;
    yunos_bitmap_set (&mmhead->sl_bitmap[flt], 31 - slt);
    yunos_bitmap_set (&mmhead->fl_bitmap , 31 - flt);
}

static  void get_block(k_mm_head *mmhead, k_mm_list_t *blk, int flt, int slt)
{
    if (blk->mbinfo.free_ptr.next) {
        blk->mbinfo.free_ptr.next->mbinfo.free_ptr.prev = blk->mbinfo.free_ptr.prev;
    }
    if (blk->mbinfo.free_ptr.prev) {
        blk->mbinfo.free_ptr.prev->mbinfo.free_ptr.next = blk->mbinfo.free_ptr.next;
    }
    if (mmhead->mm_tbl[flt][slt] == blk) {
        mmhead->mm_tbl[flt][slt] = blk->mbinfo.free_ptr.next;
        if (!mmhead->mm_tbl [flt][slt]) {
            yunos_bitmap_clear (&mmhead->sl_bitmap[flt], 31 - slt);
            if (!mmhead->sl_bitmap[flt]) {
                yunos_bitmap_clear(&mmhead->fl_bitmap, 31 - flt);
            }
        }
    }
    blk->mbinfo.free_ptr.prev = NULL;
    blk->mbinfo.free_ptr.next = NULL;
}

static k_mm_list_t *findblk_byidx(k_mm_head *mmhead, size_t *flt, size_t *slt)
{
    uint32_t     tmp  = mmhead->sl_bitmap[*flt] & (~0 << *slt);
    k_mm_list_t *find = NULL;

    if (tmp) {
        /*find last bit*/
        *slt = find_last_bit(tmp);
        find = mmhead->mm_tbl[*flt][*slt];
    } else {
        *flt = find_last_bit(mmhead->fl_bitmap & (~0 << (*flt + 1)));
        if (*flt > 0) {         /* likely */
            *slt = find_last_bit(mmhead->sl_bitmap[*flt]);
            find = mmhead->mm_tbl[*flt][*slt];
        }
    }
    return find;
}

void *k_mm_alloc(k_mm_head *mmhead, size_t size)
{
    void        *retptr;
    k_mm_list_t *b, *b2, *next_b;
    size_t       fl, sl;
    size_t       tmp_size;
    size_t       req_size = size;
    mblk_pool_t  *mm_pool;

#if (YUNOS_CONFIG_MM_DEBUG > 0u)
    size_t       allocator = 0;
#endif

    if (!mmhead) {
        return NULL;
    }

    if (size == 0) {
        return NULL;
    }

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    CPSR_ALLOC();
    YUNOS_CRITICAL_ENTER();
#else
    yunos_mutex_lock(&(mmhead->mm_mutex), YUNOS_WAIT_FOREVER);
#endif
    mm_pool = (mblk_pool_t *)mmhead->fixedmblk->mbinfo.buffer;
    if (size <= DEF_FIX_BLK_SIZE && mm_pool->blk_avail > 0) {
        retptr =  k_mm_smallblk_alloc(mmhead, size);
        if (retptr) {

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
            YUNOS_CRITICAL_EXIT();
#else
            yunos_mutex_unlock(&(mmhead->mm_mutex));
#endif
            return retptr;
        }
    }

    size = MM_ALIGN_UP(size);
    size = (size < YOS_MM_TLF_ALLOC_MIN_LENGTH) ? YOS_MM_TLF_ALLOC_MIN_LENGTH:size;

    /* Rounding up the requested size and calculating fl and sl */
    if (bitmap_search(size, &fl, &sl, ACTION_GET) != YUNOS_SUCCESS) {
        retptr = NULL;
        goto ALLOCEXIT;
    }

    /* Searching a free block, recall that this function changes the values of fl and sl,
       so they are not longer valid when the function fails */
    b = findblk_byidx(mmhead, &fl, &sl);
    if (!b || size > (b->size & YUNOS_MM_BLKSIZE_MASK )) {
        retptr = NULL;
        goto ALLOCEXIT;
    }

    mmhead->mm_tbl[fl][sl] = b->mbinfo.free_ptr.next;
    if (mmhead->mm_tbl[fl][sl]) {
        mmhead->mm_tbl[fl][sl]->mbinfo.free_ptr.prev = NULL;
    } else {
        yunos_bitmap_clear(&mmhead->sl_bitmap[fl], 31 - sl) ;
        if (!mmhead->sl_bitmap[fl]) {
            yunos_bitmap_clear (&mmhead->fl_bitmap, 31 - fl);
        }
    }
    b->mbinfo.free_ptr.prev =  NULL;
    b->mbinfo.free_ptr.next =  NULL;

    /*-- found: */
    next_b = NEXT_MM_BLK(b->mbinfo.buffer, b->size & YUNOS_MM_BLKSIZE_MASK);
    /* Should the block be split? */
    tmp_size = (b->size & YUNOS_MM_BLKSIZE_MASK) - size;
    if (tmp_size >= sizeof(k_mm_list_t)) {
        tmp_size -= MMLIST_HEAD_SIZE;
        b2 = NEXT_MM_BLK(b->mbinfo.buffer, size);
        b2->size = tmp_size | YUNOS_MM_FREE | YUNOS_MM_PREVALLOCED;
#if (YUNOS_CONFIG_MM_DEBUG > 0u)
        b2->dye  = YUNOS_MM_FREE_DYE;
#endif
        next_b->prev = b2;
        bitmap_search(tmp_size, &fl, &sl, ACTION_INSERT);
        insert_block(mmhead, b2, fl, sl);

        b->size = size | (b->size & YUNOS_MM_PRESTAT_MASK);
    } else {
        next_b->size &= (~YUNOS_MM_PREVFREE);
        b->size &= (~YUNOS_MM_FREE);       /* Now it's used */
    }

    VGF(VALGRIND_MALLOCLIKE_BLOCK(b->mbinfo.buffer, size, 0, 0));
    VGF(VALGRIND_MAKE_MEM_DEFINED(b->mbinfo.buffer, size));

#if (YUNOS_CONFIG_MM_DEBUG > 0u)
#if (YUNOS_CONFIG_GCC_RETADDR > 0u)
    allocator = (size_t)__builtin_return_address(YOS_MM_ALLOC_DEPTH);
#endif
    b->dye   = YUNOS_MM_CORRUPT_DYE;
    b->owner = allocator;
#endif
    retptr = (void *) b->mbinfo.buffer;
    if(retptr != NULL) {
        stats_addsize(mmhead, ((b->size & YUNOS_MM_BLKSIZE_MASK)
                           + MMLIST_HEAD_SIZE), req_size);
    }

    ALLOCEXIT:

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    YUNOS_CRITICAL_EXIT();
#else
    yunos_mutex_unlock(&(mmhead->mm_mutex));
#endif

    return retptr ;

}

void  k_mm_free(k_mm_head *mmhead, void *ptr)
{
    k_mm_list_t *b,      *tmp_b;
    size_t       fl = 0, sl = 0;

    if (!ptr || !mmhead) {
        return;
    }
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    CPSR_ALLOC();
    YUNOS_CRITICAL_ENTER();
#else
    yunos_mutex_lock(&(mmhead->mm_mutex), YUNOS_WAIT_FOREVER);
#endif
    if (mmhead->fixedmblk && (ptr > (void *)mmhead->fixedmblk->mbinfo.buffer)
        && (ptr < (void *)mmhead->fixedmblk->mbinfo.buffer + mmhead->fixedmblk->size)) {

        /*it's fixed size memory block*/
        k_mm_smallblk_free(mmhead, ptr);
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
        YUNOS_CRITICAL_EXIT();
#else
        yunos_mutex_unlock(&(mmhead->mm_mutex));
#endif

        return;
    }
    b = (k_mm_list_t *) ((char *) ptr - MMLIST_HEAD_SIZE);

#if (YUNOS_CONFIG_MM_DEBUG > 0u)
    if(b->dye == YUNOS_MM_FREE_DYE) {
        printf("WARNING!! memory maybe double free!!\r\n");
    }
    if(b->dye != YUNOS_MM_CORRUPT_DYE) {
        printf("WARNING,memory maybe corrupt!!\r\n");
    }
    b->dye = YUNOS_MM_FREE_DYE;
#endif
    b->size |= YUNOS_MM_FREE;

    VGF(VALGRIND_FREELIKE_BLOCK(ptr, 0));
    VGF(VALGRIND_MAKE_MEM_DEFINED(ptr, b->size & YUNOS_MM_BLKSIZE_MASK));

    stats_removesize(mmhead,
                     ((b->size & YUNOS_MM_BLKSIZE_MASK) + MMLIST_HEAD_SIZE));

    b->mbinfo.free_ptr.prev = NULL;
    b->mbinfo.free_ptr.next = NULL;
    tmp_b = NEXT_MM_BLK(b->mbinfo.buffer, b->size & YUNOS_MM_BLKSIZE_MASK);
    if (tmp_b->size & YUNOS_MM_FREE) {
        bitmap_search(tmp_b->size & YUNOS_MM_BLKSIZE_MASK, &fl, &sl, ACTION_INSERT);
        get_block( mmhead, tmp_b, fl, sl);
        b->size += (tmp_b->size & YUNOS_MM_BLKSIZE_MASK) + MMLIST_HEAD_SIZE;
    }
    if (b->size & YUNOS_MM_PREVFREE) {
        tmp_b = b->prev;
        bitmap_search(tmp_b->size & YUNOS_MM_BLKSIZE_MASK, &fl, &sl, ACTION_INSERT);
        get_block(mmhead, tmp_b, fl, sl);
        tmp_b->size += (b->size & YUNOS_MM_BLKSIZE_MASK) + MMLIST_HEAD_SIZE;
        b = tmp_b;
    }
    bitmap_search(b->size & YUNOS_MM_BLKSIZE_MASK, &fl, &sl, ACTION_INSERT);
    insert_block(mmhead, b, fl, sl);

    tmp_b = NEXT_MM_BLK(b->mbinfo.buffer, b->size & YUNOS_MM_BLKSIZE_MASK);
    tmp_b->size |= YUNOS_MM_PREVFREE;
    tmp_b->prev = b;
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    YUNOS_CRITICAL_EXIT();
#else
    yunos_mutex_unlock(&mmhead->mm_mutex);
#endif
}




void *k_mm_realloc(k_mm_head *mmhead, void *oldmem, size_t new_size)
{
    void        *ptr_aux = NULL;
    unsigned int cpsize;
    k_mm_list_t *b, *tmp_b, *next_b;
    size_t       fl, sl;
    size_t       tmp_size;
    size_t       req_size;

#if (YUNOS_CONFIG_MM_DEBUG > 0u)
    size_t       allocator = 0;
#endif

    if (!oldmem) {
        if (new_size) {
            return (void *) k_mm_alloc(mmhead, new_size);
        }

        if (!new_size) {
            return NULL;
        }
    } else if (!new_size) {
        k_mm_free(mmhead, oldmem);
        return NULL;
    }

    req_size =  new_size;

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    CPSR_ALLOC();
    YUNOS_CRITICAL_ENTER();
#else
    yunos_mutex_lock(&mmhead->mm_mutex, YUNOS_WAIT_FOREVER);
#endif

    if (mmhead->fixedmblk && (oldmem > (void *)mmhead->fixedmblk->mbinfo.buffer)
        && (oldmem < (void *)mmhead->fixedmblk->mbinfo.buffer + mmhead->fixedmblk->size)) {

        /*it's fixed size memory block*/
        if(new_size <= DEF_FIX_BLK_SIZE) {
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
        YUNOS_CRITICAL_EXIT();
#else
        yunos_mutex_unlock(&(mmhead->mm_mutex));
#endif
            return oldmem;
        }
        else {
            k_mm_smallblk_free(mmhead, oldmem);
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
            YUNOS_CRITICAL_EXIT();
#else
            yunos_mutex_unlock(&(mmhead->mm_mutex));
#endif
            return k_mm_alloc(mmhead, new_size);
        }
    }

    b        = (k_mm_list_t *) ((char *) oldmem - MMLIST_HEAD_SIZE);

    VGF(VALGRIND_FREELIKE_BLOCK(oldmem, 0));
    VGF(VALGRIND_MAKE_MEM_DEFINED(oldmem, b->size & YUNOS_MM_BLKSIZE_MASK));


    next_b   = NEXT_MM_BLK(b->mbinfo.buffer, b->size & YUNOS_MM_BLKSIZE_MASK);
    new_size = (new_size < sizeof(k_mm_list_t)) ? sizeof(k_mm_list_t) : MM_ALIGN_UP(
                   new_size);
    tmp_size = (b->size & YUNOS_MM_BLKSIZE_MASK);

    if (new_size <= tmp_size) {
        stats_removesize(mmhead,
                         ((b->size & YUNOS_MM_BLKSIZE_MASK) + MMLIST_HEAD_SIZE));
        if (next_b->size & YUNOS_MM_FREE) {
            bitmap_search(next_b->size & YUNOS_MM_BLKSIZE_MASK, &fl, &sl, ACTION_INSERT);
            get_block(mmhead, next_b, fl, sl);
            tmp_size += (next_b->size & YUNOS_MM_BLKSIZE_MASK) + MMLIST_HEAD_SIZE;
            next_b = NEXT_MM_BLK(next_b->mbinfo.buffer,
                                 next_b->size & YUNOS_MM_BLKSIZE_MASK);
            /* We allways reenter this free block because tmp_size will
               be greater then sizeof (bhdr_t) */
        }
        tmp_size -= new_size;
        if (tmp_size >= sizeof(k_mm_list_t)) {
            tmp_size -= MMLIST_HEAD_SIZE;
            tmp_b = NEXT_MM_BLK(b->mbinfo.buffer, new_size);
            tmp_b->size = tmp_size | YUNOS_MM_FREE | YUNOS_MM_PREVALLOCED;
#if (YUNOS_CONFIG_MM_DEBUG > 0u)
            tmp_b->dye  = YUNOS_MM_FREE_DYE;
#endif
            next_b->prev = tmp_b;
            next_b->size |= YUNOS_MM_PREVFREE;
            bitmap_search(tmp_size, &fl, &sl, ACTION_INSERT);
            insert_block(mmhead, tmp_b, fl, sl);
            b->size = new_size | (b->size & YUNOS_MM_PRESTAT_MASK);
        }
        stats_addsize(mmhead, ((b->size & YUNOS_MM_BLKSIZE_MASK)
                               + MMLIST_HEAD_SIZE), req_size);
        ptr_aux = (void *) b->mbinfo.buffer;
    }
    if ((next_b->size & YUNOS_MM_FREE)) {
        if (new_size <= (tmp_size + (next_b->size & YUNOS_MM_BLKSIZE_MASK))) {

            stats_removesize(mmhead,
                             ((b->size & YUNOS_MM_BLKSIZE_MASK) + MMLIST_HEAD_SIZE));
            bitmap_search(next_b->size & YUNOS_MM_BLKSIZE_MASK, &fl, &sl, ACTION_INSERT);
            get_block(mmhead, next_b, fl, sl);
            b->size += (next_b->size & YUNOS_MM_BLKSIZE_MASK) + MMLIST_HEAD_SIZE;
            next_b = NEXT_MM_BLK(b->mbinfo.buffer, b->size & YUNOS_MM_BLKSIZE_MASK);
            next_b->prev = b;
            next_b->size &= ~YUNOS_MM_PREVFREE;
            tmp_size = (b->size & YUNOS_MM_BLKSIZE_MASK) - new_size;
            if (tmp_size >= sizeof(k_mm_list_t)) {
                tmp_size -= MMLIST_HEAD_SIZE;
                tmp_b = NEXT_MM_BLK(b->mbinfo.buffer, new_size);
                tmp_b->size = tmp_size | YUNOS_MM_FREE | YUNOS_MM_PREVALLOCED;
#if (YUNOS_CONFIG_MM_DEBUG > 0u)
                tmp_b->dye  = YUNOS_MM_FREE_DYE;
#endif
                next_b->prev = tmp_b;
                next_b->size |= YUNOS_MM_PREVFREE;
                bitmap_search(tmp_size, &fl, &sl, ACTION_INSERT);
                insert_block(mmhead, tmp_b, fl, sl);
                b->size = new_size | (b->size & YUNOS_MM_PRESTAT_MASK);
            }
            stats_addsize(mmhead, ((b->size & YUNOS_MM_BLKSIZE_MASK)
                                   + MMLIST_HEAD_SIZE), req_size);
            ptr_aux = (void *) b->mbinfo.buffer;
        }
    }

    VGF(VALGRIND_MAKE_MEM_DEFINED(b->mbinfo.buffer, new_size));
    VGF(VALGRIND_MALLOCLIKE_BLOCK(b->mbinfo.buffer, new_size, 0, 0));

#if (YUNOS_CONFIG_MM_DEBUG > 0u)
#if (YUNOS_CONFIG_GCC_RETADDR > 0u)
    allocator = (size_t)__builtin_return_address(YOS_MM_ALLOC_DEPTH);
#endif
    b->dye   = YUNOS_MM_CORRUPT_DYE;
    b->owner = allocator;
#endif


    if (ptr_aux) {
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
        YUNOS_CRITICAL_EXIT();
#else
        yunos_mutex_unlock(&mmhead->mm_mutex);
#endif
        return ptr_aux;
    }

    if (!(ptr_aux = k_mm_alloc(mmhead, new_size))) {
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
        YUNOS_CRITICAL_EXIT();
#else
        yunos_mutex_unlock(&mmhead->mm_mutex);
#endif
        return NULL;
    }
    cpsize = ((b->size & YUNOS_MM_BLKSIZE_MASK) > new_size) ? new_size :
             (b->size & YUNOS_MM_BLKSIZE_MASK);

    memcpy(ptr_aux, oldmem, cpsize);

    k_mm_free(mmhead, oldmem);
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    YUNOS_CRITICAL_EXIT();
#else
    yunos_mutex_unlock(&mmhead->mm_mutex);
#endif
    return ptr_aux;

}

void *yunos_mm_alloc(size_t size)
{
    void *tmp;

    if (size == 0) {
        printf("WARNING, malloc size = 0\r\n");
        return NULL;
    }

    tmp = k_mm_alloc(g_kmm_head, size);
    if (tmp == NULL) {
#if (YUNOS_CONFIG_MM_DEBUG > 0)
        dumpsys_mm_info_func(NULL, 0);
        printf("WARNING, malloc failed!!!!\r\n");
        //exit(0);
#endif
    }

#if (YUNOS_CONFIG_USER_HOOK > 0)
    yunos_mm_alloc_hook(tmp,size);
#endif

    return tmp;

}
void yunos_mm_free(void *ptr)
{
    return k_mm_free(g_kmm_head, ptr);
}

void *yunos_mm_realloc(void *oldmem, size_t newsize)
{
    return k_mm_realloc(g_kmm_head, oldmem, newsize);
}

#endif


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

#if (YUNOS_CONFIG_MM_BESTFIT > 0)

kstat_t yunos_mm_bf_alloc(k_mm_region_head_t * region_head, void **mem,size_t size, size_t allocator)
{
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    CPSR_ALLOC();
#endif
    k_mm_region_list_t *pcur            = NULL;
    k_mm_region_list_t *pregion         = NULL;
    k_mm_region_list_t *find            = NULL;
    klist_t            *head            = NULL;
    klist_t            *end             = NULL;
    klist_t            *next            = NULL;
    klist_t            *tmp             = NULL;
    size_t              addr_align_mask = 0u;
    size_t              left            = 0u;

    if (size == 0u) {
        return YUNOS_MM_ALLOC_SIZE_ERR;
    }

    NULL_PARA_CHK(region_head);
    NULL_PARA_CHK(mem);

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    YUNOS_CRITICAL_ENTER();
#else
    yunos_mutex_lock(&(region_head->mm_region_mutex), YUNOS_WAIT_FOREVER);
#endif

    addr_align_mask = sizeof(void *) - 1u;

    /* padding the memory size to alignment */
    if ((size & addr_align_mask) != 0u) {
        size = ((size & (~addr_align_mask)) + sizeof(void *));
    }

    VGF(VALGRIND_MAKE_MEM_DEFINED(region_head, sizeof(k_mm_region_head_t)));

    head = &region_head->probe;
    end  = head;

    if (is_klist_empty(head) || size >= region_head->freesize) {
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
        YUNOS_CRITICAL_EXIT();
#else
        yunos_mutex_unlock(&(region_head->mm_region_mutex));
#endif
        *mem = NULL;
        return YUNOS_NO_MEM;
    }

    left = YUNOS_MM_REGION_MAX_FRAGSIZE;


    for (tmp = head->next; tmp != end; ) {

        pcur = yunos_list_entry(tmp, k_mm_region_list_t, list);

        VGF(VALGRIND_MAKE_MEM_DEFINED(pcur,sizeof(k_mm_region_list_t)));

        if (pcur->len >= size) {

            if (left > pcur->len - size) {
                left = pcur->len - size;
                find = pcur;
            }

            if (0 == left) {
                break;
            }
        }
        tmp = tmp->next;
        VGF(VALGRIND_MAKE_MEM_NOACCESS(pcur,sizeof(k_mm_region_list_t)));

    }

    if (NULL == find) {
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
        YUNOS_CRITICAL_EXIT();
#else
        yunos_mutex_unlock(&(region_head->mm_region_mutex));
#endif
        *mem = NULL;
        return YUNOS_NO_MEM;
    }

    VGF(VALGRIND_MAKE_MEM_DEFINED(find,sizeof(k_mm_region_list_t)));

    next = find->list.next;

    VGF(VALGRIND_MAKE_MEM_DEFINED(next,sizeof(klist_t)));
    VGF(VALGRIND_MAKE_MEM_DEFINED(find->list.prev,sizeof(klist_t)));

    klist_rm(&(find->list));

    VGF(VALGRIND_MAKE_MEM_NOACCESS(next,sizeof(klist_t)));
    VGF(VALGRIND_MAKE_MEM_NOACCESS(find->list.prev,sizeof(klist_t)));

    if (find->len < size + sizeof(k_mm_region_list_t) + sizeof(void *)) {
        find->type  = YUNOS_MM_REGION_ALLOCED;

#if (YUNOS_CONFIG_MM_DEBUG > 0)
        find->owner = allocator;
#endif

        region_head->freesize -= find->len;

        VGF(VALGRIND_MAKE_MEM_DEFINED(&region_head->alloced,sizeof(klist_t)));
        VGF(VALGRIND_MAKE_MEM_DEFINED(region_head->alloced.next,sizeof(klist_t)));

        klist_add(&region_head->alloced, &(find->list));

        VGF(VALGRIND_MAKE_MEM_NOACCESS(find->list.next,sizeof(klist_t)));
        VGF(VALGRIND_MAKE_MEM_NOACCESS(find,sizeof(k_mm_region_list_t)));

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
        YUNOS_CRITICAL_EXIT();
#else
        yunos_mutex_unlock(&region_head->mm_region_mutex);
#endif
        *mem = (void *)((size_t)find + sizeof(k_mm_region_list_t));
        VGF(VALGRIND_MAKE_MEM_DEFINED(*mem,size));
        VGF(VALGRIND_MALLOCLIKE_BLOCK(*mem, size,0,0));

        return YUNOS_SUCCESS;
    }

    pregion = (k_mm_region_list_t *)((size_t)find + sizeof(k_mm_region_list_t) + size);

    VGF(VALGRIND_MAKE_MEM_DEFINED(pregion,sizeof(k_mm_region_list_t)));
    VGF(VALGRIND_MAKE_MEM_DEFINED(next,sizeof(klist_t)));
    VGF(VALGRIND_MAKE_MEM_DEFINED(next->prev,sizeof(klist_t)));

    pregion->type  = YUNOS_MM_REGION_FREE;
    pregion->len   = find->len - sizeof(k_mm_region_list_t) - size;

    klist_insert(next, &(pregion->list));


    find->type  = YUNOS_MM_REGION_ALLOCED;
    find->len   = size;

#if (YUNOS_CONFIG_MM_DEBUG > 0u)
    pregion->dye = YUNOS_MM_REGION_CORRUPT_DYE;
    find->owner  = allocator;
#endif

    VGF(VALGRIND_MAKE_MEM_DEFINED(&region_head->alloced,sizeof(klist_t)));
    VGF(VALGRIND_MAKE_MEM_DEFINED(region_head->alloced.next,sizeof(klist_t)));

    klist_add(&region_head->alloced, &(find->list));

    VGF(VALGRIND_MAKE_MEM_NOACCESS(next,sizeof(klist_t)));
    VGF(VALGRIND_MAKE_MEM_NOACCESS(pregion,sizeof(k_mm_region_list_t)));
    VGF(VALGRIND_MAKE_MEM_NOACCESS(find->list.next,sizeof(klist_t)));
    VGF(VALGRIND_MAKE_MEM_NOACCESS(find,sizeof(k_mm_region_list_t)));

    region_head->frag_num++;
    region_head->freesize -= (size + sizeof(k_mm_region_list_t));

    VGF(VALGRIND_MAKE_MEM_DEFINED(region_head, sizeof(k_mm_region_head_t)));

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    YUNOS_CRITICAL_EXIT();
#else
    yunos_mutex_unlock(&(region_head->mm_region_mutex));
#endif

    *mem = (void *)((size_t)find + sizeof(k_mm_region_list_t));
    VGF(VALGRIND_MAKE_MEM_DEFINED(*mem,size));
    VGF(VALGRIND_MALLOCLIKE_BLOCK(*mem, size, 0, 0));

    return YUNOS_SUCCESS;
}
#endif

#if (YUNOS_CONFIG_MM_FIRSTFIT > 0)

kstat_t yunos_mm_ff_alloc(k_mm_region_head_t * region_head, void **mem,size_t size, size_t allocator)
{
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    CPSR_ALLOC();
#endif
    k_mm_region_list_t *pcur            = NULL;
    k_mm_region_list_t *pregion         = NULL;
    k_mm_region_list_t *find            = NULL;
    klist_t            *head            = NULL;
    klist_t            *end             = NULL;
    klist_t            *next            = NULL;
    klist_t            *tmp             = NULL;
    size_t              addr_align_mask = 0u;

    if (size == 0u) {
        return YUNOS_MM_ALLOC_SIZE_ERR;
    }
    NULL_PARA_CHK(region_head);
    NULL_PARA_CHK(mem);

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    YUNOS_CRITICAL_ENTER();
#else
    yunos_mutex_lock(&(region_head->mm_region_mutex), YUNOS_WAIT_FOREVER);
#endif

    addr_align_mask = sizeof(void *) - 1u;

    /* padding the memory size to alignment */
    if ((size & addr_align_mask) != 0u) {
        size = ((size & (~addr_align_mask)) + sizeof(void *));
    }

    VGF(VALGRIND_MAKE_MEM_DEFINED(region_head, sizeof(k_mm_region_head_t)));

    head = &region_head->probe;
    end  = head;
    if (is_klist_empty(head) || size >= region_head->freesize) {
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
        YUNOS_CRITICAL_EXIT();
#else
        yunos_mutex_unlock(&(region_head->mm_region_mutex));
#endif
        *mem = NULL;
        return YUNOS_NO_MEM;
    }

    for (tmp = head->next; tmp != end; ) {
        pcur = yunos_list_entry(tmp, k_mm_region_list_t, list);

        VGF(VALGRIND_MAKE_MEM_DEFINED(pcur,sizeof(k_mm_region_list_t)));

        if (pcur->len >= size) {
            find = pcur;
            break;
        }
        tmp = tmp->next;
        VGF(VALGRIND_MAKE_MEM_NOACCESS(pcur,sizeof(k_mm_region_list_t)));
    }

    if (NULL == find) {
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
        YUNOS_CRITICAL_EXIT();
#else
        yunos_mutex_unlock(&(region_head->mm_region_mutex));
#endif
        *mem = NULL;
        return YUNOS_NO_MEM;
    }

    VGF(VALGRIND_MAKE_MEM_DEFINED(find,sizeof(k_mm_region_list_t)));

    next = find->list.next;

    VGF(VALGRIND_MAKE_MEM_DEFINED(next,sizeof(klist_t)));
    VGF(VALGRIND_MAKE_MEM_DEFINED(find->list.prev,sizeof(klist_t)));

    klist_rm(&(find->list));

    VGF(VALGRIND_MAKE_MEM_NOACCESS(next,sizeof(klist_t)));
    VGF(VALGRIND_MAKE_MEM_NOACCESS(find->list.prev,sizeof(klist_t)));

    if (find->len < size + sizeof(k_mm_region_list_t) + sizeof(void *)) {
        find->type  = YUNOS_MM_REGION_ALLOCED;

#if (YUNOS_CONFIG_MM_DEBUG > 0)
        find->owner = allocator;
#endif

        region_head->freesize -= find->len;

        VGF(VALGRIND_MAKE_MEM_DEFINED(&region_head->alloced,sizeof(klist_t)));
        VGF(VALGRIND_MAKE_MEM_DEFINED(region_head->alloced.next,sizeof(klist_t)));

        klist_add(&region_head->alloced, &(find->list));

        VGF(VALGRIND_MAKE_MEM_NOACCESS(find->list.next,sizeof(klist_t)));
        VGF(VALGRIND_MAKE_MEM_NOACCESS(find,sizeof(k_mm_region_list_t)));

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
        YUNOS_CRITICAL_EXIT();
#else
        yunos_mutex_unlock(&region_head->mm_region_mutex);
#endif
        *mem = (void *)((size_t)find + sizeof(k_mm_region_list_t));
        VGF(VALGRIND_MAKE_MEM_DEFINED(*mem,size));
        VGF(VALGRIND_MALLOCLIKE_BLOCK(*mem, size,0,0));

        return YUNOS_SUCCESS;
    }

    pregion = (k_mm_region_list_t *)((size_t)find + sizeof(k_mm_region_list_t) + size);

    VGF(VALGRIND_MAKE_MEM_DEFINED(pregion,sizeof(k_mm_region_list_t)));
    VGF(VALGRIND_MAKE_MEM_DEFINED(next,sizeof(klist_t)));
    VGF(VALGRIND_MAKE_MEM_DEFINED(next->prev,sizeof(klist_t)));

    pregion->type  = YUNOS_MM_REGION_FREE;
    pregion->len   = find->len - sizeof(k_mm_region_list_t) - size;

    klist_insert(next, &(pregion->list));


    find->type  = YUNOS_MM_REGION_ALLOCED;
    find->len   = size;

#if (YUNOS_CONFIG_MM_DEBUG > 0u)
    pregion->dye = YUNOS_MM_REGION_CORRUPT_DYE;
    find->owner  = allocator;
#endif

    VGF(VALGRIND_MAKE_MEM_DEFINED(&region_head->alloced,sizeof(klist_t)));
    VGF(VALGRIND_MAKE_MEM_DEFINED(region_head->alloced.next,sizeof(klist_t)));

    klist_add(&region_head->alloced, &(find->list));

    VGF(VALGRIND_MAKE_MEM_NOACCESS(next,sizeof(klist_t)));
    VGF(VALGRIND_MAKE_MEM_NOACCESS(pregion,sizeof(k_mm_region_list_t)));
    VGF(VALGRIND_MAKE_MEM_NOACCESS(find->list.next,sizeof(klist_t)));
    VGF(VALGRIND_MAKE_MEM_NOACCESS(find,sizeof(k_mm_region_list_t)));

    region_head->frag_num++;
    region_head->freesize -= (size + sizeof(k_mm_region_list_t));

    VGF(VALGRIND_MAKE_MEM_DEFINED(region_head, sizeof(k_mm_region_head_t)));

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    YUNOS_CRITICAL_EXIT();
#else
    yunos_mutex_unlock(&(region_head->mm_region_mutex));
#endif

    *mem = (void *)((size_t)find + sizeof(k_mm_region_list_t));
    VGF(VALGRIND_MAKE_MEM_DEFINED(*mem,size));
    VGF(VALGRIND_MALLOCLIKE_BLOCK(*mem, size, 0, 0));

    return YUNOS_SUCCESS;
}
#endif /* YUNOS_CONFIG_MM_FIRSTFIT */

#if (YUNOS_CONFIG_MM_BESTFIT > 0 || YUNOS_CONFIG_MM_FIRSTFIT > 0)
kstat_t yunos_mm_xf_free(k_mm_region_head_t * region_head, void *mem)
{
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    CPSR_ALLOC();
#endif

    k_mm_region_list_t *pcur  = NULL;
    k_mm_region_list_t *prev  = NULL;
    k_mm_region_list_t *pnext = NULL;
    size_t              size  = 0u;

    NULL_PARA_CHK(region_head);
    NULL_PARA_CHK(mem);

    VGF(VALGRIND_MAKE_MEM_DEFINED(region_head, sizeof(k_mm_region_head_t)));

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    YUNOS_CRITICAL_ENTER();
#else
    yunos_mutex_lock(&(region_head->mm_region_mutex), YUNOS_WAIT_FOREVER);
#endif

    pcur = (k_mm_region_list_t *)((size_t)mem - sizeof(k_mm_region_list_t));

    VGF(VALGRIND_MAKE_MEM_DEFINED(pcur,sizeof(k_mm_region_list_t)));

    if (NULL == pcur) {
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
        YUNOS_CRITICAL_EXIT();
#else
        yunos_mutex_unlock(&(region_head->mm_region_mutex));
#endif
        k_err_proc(YUNOS_MM_FREE_ADDR_ERR);
        return YUNOS_MM_FREE_ADDR_ERR;
    }
    /*before rm pcur->list, we need make pcur->list.prev and pcur->list.next as accessable*/
    VGF(VALGRIND_MAKE_MEM_DEFINED(pcur->list.prev,sizeof(klist_t)));
    VGF(VALGRIND_MAKE_MEM_DEFINED(pcur->list.next,sizeof(klist_t)));

    klist_rm(&(pcur->list));

    /*after rm pcur, we need protect prev and next again*/
    VGF(VALGRIND_MAKE_MEM_DEFINED(pcur->list.prev,sizeof(klist_t)));
    VGF(VALGRIND_MAKE_MEM_DEFINED(pcur->list.next,sizeof(klist_t)));

    size        = pcur->len;
    pcur->type  = YUNOS_MM_REGION_FREE;
#if (YUNOS_CONFIG_MM_DEBUG > 0u)
    pcur->owner = 0u;
#endif

    yunos_mm_region_insert2freelist(region_head,&(pcur->list));

    pnext = yunos_list_entry(pcur->list.next, k_mm_region_list_t, list);
    prev  = yunos_list_entry(pcur->list.prev, k_mm_region_list_t, list);

    VGF(VALGRIND_MAKE_MEM_DEFINED(pnext, sizeof(k_mm_region_list_t)));
    VGF(VALGRIND_MAKE_MEM_DEFINED(prev, sizeof(k_mm_region_list_t)));
    VGF(VALGRIND_MAKE_MEM_DEFINED(pnext->list.next,sizeof(klist_t)));

    if (pnext->type == YUNOS_MM_REGION_FREE
        && (size_t)pcur + sizeof(k_mm_region_list_t) + pcur->len == (size_t)pnext) {
        klist_rm(&pnext->list);
        pcur->len += (pnext->len + sizeof(k_mm_region_list_t));
        region_head->frag_num--;
        region_head->freesize += sizeof(k_mm_region_list_t);
    }

    if (NULL != prev && prev->type == YUNOS_MM_REGION_FREE
        && (size_t)prev + sizeof(k_mm_region_list_t) + prev->len == (size_t)pcur) {
        klist_rm(&pcur->list);
        prev->len  += (pcur->len + sizeof(k_mm_region_list_t));
        region_head->frag_num--;
        region_head->freesize += sizeof(k_mm_region_list_t);
    }

    VGF(VALGRIND_MAKE_MEM_NOACCESS(pnext->list.next,sizeof(klist_t)));
    VGF(VALGRIND_MAKE_MEM_NOACCESS(pnext, sizeof(k_mm_region_list_t)));
    VGF(VALGRIND_MAKE_MEM_NOACCESS(prev, sizeof(k_mm_region_list_t)));
    VGF(VALGRIND_MAKE_MEM_NOACCESS(pcur,sizeof(k_mm_region_list_t)));

    region_head->freesize += size;

    VGF(VALGRIND_FREELIKE_BLOCK(mem, 0));
    VGF(VALGRIND_MAKE_MEM_DEFINED(region_head, sizeof(k_mm_region_head_t)));

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    YUNOS_CRITICAL_EXIT();
#else
    yunos_mutex_unlock(&(region_head->mm_region_mutex));
#endif

    return YUNOS_SUCCESS;
}

#endif


void *yunos_mm_alloc(size_t size)
{
    void   *tmp      = NULL;
    kstat_t err      = YUNOS_SUCCESS;
    size_t  alloctor = 0;

#if (YUNOS_CONFIG_MM_BESTFIT > 0)
    err = yunos_mm_bf_alloc(&g_kmm_region_head, &tmp, size, alloctor);
#elif(YUNOS_CONFIG_MM_FIRSTFIT > 0)
    err = yunos_mm_ff_alloc(&g_kmm_region_head, &tmp, size, alloctor);
#else
    err = YUNOS_NO_MEM;
#endif
    if (err != YUNOS_SUCCESS) {
#if (YUNOS_CONFIG_MM_DEBUG > 0)
        dumpsys_mm_info_func(NULL, 0);
#endif
        tmp = NULL;
    }
    return tmp;

}

void yunos_mm_free(void * ptr)
{
    kstat_t ret;

    ret = yunos_mm_xf_free(&g_kmm_region_head,ptr);

    /*halt when free failed?*/
    assert(ret == YUNOS_SUCCESS);
}



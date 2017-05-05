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


kstat_t yunos_mm_region_init(k_mm_region_head_t * region_head, k_mm_region_t *regions, size_t size)
{
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    CPSR_ALLOC();
#endif

    k_mm_region_t      *pregion         = NULL;
    k_mm_region_list_t *pcur            = NULL;
    klist_t            *prev            = NULL;
    size_t              count           = 0u;
    size_t              addr_align_mask = 0u;
    size_t              start_adr       = 0u;
    size_t              region_len      = 0u;

    NULL_PARA_CHK(region_head);
    NULL_PARA_CHK(regions);

    if (size == 0u) {
        return YUNOS_INV_PARAM;
    }

    addr_align_mask = sizeof(void *) - 1u;

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    YUNOS_CRITICAL_ENTER();
#else
    yunos_mutex_create(&region_head->mm_region_mutex, "mm_region_mutex");
    yunos_mutex_lock(&region_head->mm_region_mutex, YUNOS_WAIT_FOREVER);
#endif

    VGF(VALGRIND_MAKE_MEM_DEFINED(region_head, sizeof(k_mm_region_head_t)));

    klist_init(&(region_head->probe));
    klist_init(&(region_head->alloced));

    for (; count < size; count++) {
        pregion = &regions[count];

        if (pregion->len < sizeof(k_mm_region_list_t)) {
            continue;
        }

        start_adr  = (size_t)pregion->start;
        region_len = pregion->len;

        if ((start_adr & addr_align_mask) != 0) {
            start_adr += addr_align_mask;
            start_adr &= ~addr_align_mask;
            region_len -= start_adr - (size_t) pregion->start;
        }
#if defined(__VALGRIND_MAJOR__) && defined(__VALGRIND_MINOR__)   \
                    && (__VALGRIND_MAJOR__ > 3                                   \
                        || (__VALGRIND_MAJOR__ == 3 && __VALGRIND_MINOR__ >= 12))
                    /*valgrind support VALGRIND_CREATE_MEMPOOL_EXT from 3.12.0*/
                VGF(VALGRIND_CREATE_MEMPOOL_EXT(start_adr, 0, 0,
                                                VALGRIND_MEMPOOL_METAPOOL | VALGRIND_MEMPOOL_AUTO_FREE));
#else
                VGF(VALGRIND_CREATE_MEMPOOL((uint8_t*)start_adr, 0, 0));
#endif

        pcur = (k_mm_region_list_t *)start_adr;

        VGF(VALGRIND_MAKE_MEM_DEFINED((uint8_t*)pcur,sizeof(k_mm_region_list_t)));

        pcur->len  = region_len - sizeof(k_mm_region_list_t);
        pcur->type = YUNOS_MM_REGION_FREE;

#if (YUNOS_CONFIG_MM_DEBUG > 0)
        pcur->dye = YUNOS_MM_REGION_CORRUPT_DYE;
        pcur->owner = 0u;
#endif
        if (prev == NULL) {
            klist_add(&(region_head->probe), &(pcur->list));
        } else {
            VGF(VALGRIND_MAKE_MEM_DEFINED(prev, sizeof(klist_t)));
            klist_add(prev, &(pcur->list));
        }

        prev = &pcur->list;

        region_head->frag_num++;
        region_head->freesize += pcur->len;

        VGF(VALGRIND_MAKE_MEM_NOACCESS(start_adr,sizeof(k_mm_region_list_t)));
        VGF(VALGRIND_MAKE_MEM_DEFINED(region_head, sizeof(k_mm_region_head_t)));

    }

    /* first time */
    if(g_mm_region_list_head.next ==  NULL || g_mm_region_list_head.prev == NULL){
        klist_init(&g_mm_region_list_head);
#if (YUNOS_CONFIG_MM_REGION_MUTEX > 0)
        yunos_mutex_create(&g_mm_region_mutex, "g_mm_region_mutex");
#endif
    }
#if (YUNOS_CONFIG_MM_REGION_MUTEX > 0)
    yunos_mutex_lock(&g_mm_region_mutex, YUNOS_WAIT_FOREVER);
#endif
    klist_add(&g_mm_region_list_head,&(region_head->regionlist));
#if (YUNOS_CONFIG_MM_REGION_MUTEX > 0)
    yunos_mutex_unlock(&g_mm_region_mutex);
#endif

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    YUNOS_CRITICAL_EXIT();
#else
    yunos_mutex_unlock(&region_head->mm_region_mutex);
#endif

    TRACE_MM_REGION_CREATE(g_active_task, regions);

    return YUNOS_SUCCESS;
}


kstat_t yunos_mm_region_insert2freelist(k_mm_region_head_t * region_head,klist_t* node)
{
    klist_t            *head = NULL;
    klist_t            *end  = NULL;
    klist_t            *tmp;

    head = &region_head->probe;
    end  = head;

    NULL_PARA_CHK(region_head);
    NULL_PARA_CHK(node);

    VGF(VALGRIND_MAKE_MEM_DEFINED(region_head, sizeof(k_mm_region_head_t)));

    for (tmp = head->next; tmp != end; tmp = tmp->next) {
        VGF(VALGRIND_MAKE_MEM_DEFINED(tmp,sizeof(klist_t)));
        VGF(VALGRIND_MAKE_MEM_NOACCESS(tmp->prev,sizeof(klist_t)));
        if (tmp > node) {
            break;
        }
    }
    if(tmp != end){
        VGF(VALGRIND_MAKE_MEM_DEFINED(tmp->prev,sizeof(klist_t)));
        klist_insert(tmp,node);
        /*node->prev is old tmp->prev*/
        VGF(VALGRIND_MAKE_MEM_NOACCESS(node->prev,sizeof(klist_t)));
    }
    else{
        VGF(VALGRIND_MAKE_MEM_DEFINED(tmp->next,sizeof(klist_t)));
        klist_add(tmp, node);
        /*node->prev is old tmp->prev*/
        VGF(VALGRIND_MAKE_MEM_NOACCESS(node->next,sizeof(klist_t)));
    }

    VGF(VALGRIND_MAKE_MEM_NOACCESS(tmp, sizeof(klist_t)));

    /*need make sure head shou l accessable*/
    VGF(VALGRIND_MAKE_MEM_DEFINED(region_head, sizeof(k_mm_region_head_t)));

    return YUNOS_SUCCESS;
}



size_t yunos_mm_region_get_free_size(k_mm_region_head_t * region_head)
{

    size_t size;

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    CPSR_ALLOC();
#endif

    if(region_head == NULL){
        return 0;
    }

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    YUNOS_CRITICAL_ENTER();
#else
    yunos_mutex_lock(&(region_head->mm_region_mutex), YUNOS_WAIT_FOREVER);
#endif

    VGF(VALGRIND_MAKE_MEM_DEFINED(region_head, sizeof(k_mm_region_head_t)));

    size = region_head->freesize;

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
    YUNOS_CRITICAL_EXIT();
#else
    yunos_mutex_unlock(&(region_head->mm_region_mutex));
#endif
    return size;
}

#if (YUNOS_CONFIG_MM_DEBUG > 0)
kstat_t check_mm_info_func()
{
    k_mm_region_list_t *min              = NULL;
    klist_t            *head             = NULL;
    klist_t            *end              = NULL;
    klist_t            *cur              = NULL;
    klist_t            *region_list_cur  = NULL;
    klist_t            *region_list_head = NULL;
    k_mm_region_head_t *cur_region;

    NULL_PARA_CHK(g_mm_region_list_head.next);
    NULL_PARA_CHK(g_mm_region_list_head.prev);

    region_list_head = &g_mm_region_list_head;

    VGF(VALGRIND_MAKE_MEM_DEFINED(region_list_head, sizeof(k_mm_region_head_t)));

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
        CPSR_ALLOC();
#endif

#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
        YUNOS_CRITICAL_ENTER();
#endif
    for(region_list_cur = region_list_head->next; region_list_cur != region_list_head;){
        cur_region = yunos_list_entry(region_list_cur,k_mm_region_head_t,regionlist);

        VGF(VALGRIND_MAKE_MEM_DEFINED(cur_region,sizeof(k_mm_region_head_t)));

        head = &cur_region->probe;
        end = head;

        VGF(VALGRIND_MAKE_MEM_DEFINED(head,sizeof(klist_t)));

#if (YUNOS_CONFIG_MM_REGION_MUTEX > 0)
        yunos_mutex_lock(&(cur_region->mm_region_mutex), YUNOS_WAIT_FOREVER);
#endif
        for (cur = head->next; cur != end; ) {
            min = yunos_list_entry(cur, k_mm_region_list_t, list);

            VGF(VALGRIND_MAKE_MEM_DEFINED(min,sizeof(k_mm_region_list_t)));

            if ((YUNOS_MM_REGION_CORRUPT_DYE & min->dye) != YUNOS_MM_REGION_CORRUPT_DYE) {
                VGF(VALGRIND_MAKE_MEM_NOACCESS(min,sizeof(k_mm_region_list_t)));
                VGF(VALGRIND_MAKE_MEM_NOACCESS(cur_region,sizeof(k_mm_region_head_t)));
                k_err_proc(YUNOS_MM_CORRUPT_ERR);
                return YUNOS_MM_CORRUPT_ERR;
            }

            cur  = cur->next;
            VGF(VALGRIND_MAKE_MEM_NOACCESS(min,sizeof(k_mm_region_list_t)));

        }

        region_list_cur = region_list_cur->next;

        VGF(VALGRIND_MAKE_MEM_NOACCESS(cur_region,sizeof(k_mm_region_head_t)));

#if (YUNOS_CONFIG_MM_REGION_MUTEX > 0)
        yunos_mutex_unlock(&(cur_region->mm_region_mutex));
#endif
    }
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 0)
        YUNOS_CRITICAL_EXIT();
#endif
    return YUNOS_SUCCESS;
}
#endif



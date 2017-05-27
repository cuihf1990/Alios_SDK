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

#include <stdio.h>

#include <k_api.h>
#include "k_mm_region.h"
#include "k_mm_debug.h"
#include "yos/log.h"

#if (YUNOS_CONFIG_MM_DEBUG > 0)

extern klist_t g_mm_region_list_head;

#if (YUNOS_CONFIG_MM_LEAKCHECK > 0)
static mm_scan_region_t g_mm_scan_region[YOS_MM_SCAN_REGION_MAX];
static void           **g_leak_match;
static uint32_t         g_recheck_flag = 0;

uint32_t yunos_mm_leak_region_init(void *start, void *end)
{
    static uint32_t i = 0;

    if (i >= YOS_MM_SCAN_REGION_MAX) {
        return -1;
    }

    if ((start == NULL) || (end == NULL)) {
        return -1;
    }

    g_mm_scan_region[i].start = start;
    g_mm_scan_region[i].end   = end;

    i++;
    return i;
}

static uint32_t check_non_active_task_stack(ktask_t *task, void ** p)
{
    uint32_t offset = 0;
    kstat_t  rst    = YUNOS_SUCCESS;
    void    *start;

    rst =  yunos_task_stack_cur_free(task, &offset);
    if (rst == YUNOS_SUCCESS) {
        start = task->task_stack_base + task->stack_size - offset;
    } else {
        k_err_proc(YUNOS_SYS_SP_ERR);
        return 1;
    }
    if ((uint32_t)p >= (uint32_t)start &&
        (uint32_t)p  < (uint32_t)task->task_stack) {
        return 0;
    }
    return 1;
}

static uint32_t check_active_task_stack(void ** p)
{
    uint32_t offset = 0;
    kstat_t  rst    = YUNOS_SUCCESS;
    void    *end;

    if (NULL == g_active_task) {
        return 1;
    }

    rst =  yunos_task_stack_cur_free(g_active_task, &offset);
    if (rst == YUNOS_SUCCESS) {
        end = g_active_task->task_stack_base + g_active_task->stack_size - offset;
    } else {
        k_err_proc(YUNOS_SYS_SP_ERR);
        return 0;
    }

    if ((uint32_t)p >= (uint32_t)g_active_task->task_stack_base &&
        (uint32_t)p  < (uint32_t)end) {
        return 0;
    }
    return 1;
}

static uint32_t check_if_in_stack(void **p)
{
    klist_t *taskhead = &g_kobj_list.task_head;
    klist_t *taskend  = taskhead;
    klist_t *tmp;
    ktask_t *task;

    if (1 == check_active_task_stack(p)) {
        return 1;
    }

    for (tmp = taskhead->next; tmp != taskend; tmp = tmp->next) {
        task = yunos_list_entry(tmp, ktask_t, task_stats_item);
        if (1 == check_non_active_task_stack(task, p)) {
            return 1;
        }
    }
    return 0;
}

static uint32_t scan_region(void *start, void *end, void *adress)
{
    void **p = (void **)((uint32_t)start & ~(sizeof(size_t) - 1));

    while ((void *)p <= end) {
        if (NULL != p && adress  == *p) {
            g_leak_match = p;
            return 1;
        }

        p++;
    }

    return 0;
}

static uint32_t check_malloc_region(void *adress)
{
    uint32_t            rst = 0;
    void               *start  = NULL;
    k_mm_region_list_t *tmp = NULL;
    klist_t            *head;
    klist_t            *end;
    klist_t            *cur;
    klist_t            *region_list_cur;
    klist_t            *region_list_head;
    k_mm_region_head_t *cur_region;

    NULL_PARA_CHK(g_mm_region_list_head.next);
    NULL_PARA_CHK(g_mm_region_list_head.prev);

    region_list_head = &g_mm_region_list_head;

    for(region_list_cur = region_list_head->next; region_list_cur != region_list_head;region_list_cur = region_list_cur->next){
        cur_region = yunos_list_entry(region_list_cur,k_mm_region_head_t,regionlist);
        head = &cur_region->alloced;
        end = head;

        for (cur = head->next; cur != end; cur = cur->next) {
            tmp = yunos_list_entry(cur, k_mm_region_list_t, list);
            start  = (void*)((uint32_t)tmp + sizeof(k_mm_region_list_t));
            if (0 == g_recheck_flag && YUNOS_MM_REGION_ALLOCED == tmp->type) {
                rst = scan_region(start, (void*)((uint32_t)start + tmp->len), adress);
                if (1 == rst) {
                    return check_if_in_stack(g_leak_match);
                }
            }

            if (1 == g_recheck_flag &&
                (uint32_t)adress >= (uint32_t)start &&
                (uint32_t)adress < (uint32_t)start + tmp->len) {
                return 1;
            }
        }
    }
    return 0;
}

uint32_t check_mm_leak(uint8_t *adress)
{
    uint32_t rst = 0;
    uint32_t i;

    for (i = 0; i < YOS_MM_SCAN_REGION_MAX; i++) {

        if ((NULL == g_mm_scan_region[i].start) || (NULL == g_mm_scan_region[i].end)) {
            continue;
        }

        if (1 == scan_region(g_mm_scan_region[i].start, g_mm_scan_region[i].end, adress)) {
            return 1;
        }
    }

    rst = check_malloc_region(adress);
    if (1 == rst) {
        return 1;
    }

    return 0;
}

uint32_t if_adress_is_valid(void *adress)
{
    void               *start  = NULL;
    k_mm_region_list_t *tmp = NULL;
    klist_t            *head;
    klist_t            *end;
    klist_t            *cur;
    klist_t            *region_list_cur;
    klist_t            *region_list_head;
    k_mm_region_head_t * cur_region;

    NULL_PARA_CHK(g_mm_region_list_head.next);
    NULL_PARA_CHK(g_mm_region_list_head.prev);

    region_list_head =  &g_mm_region_list_head;

    for(region_list_cur = region_list_head->next; region_list_cur != region_list_head;region_list_cur = region_list_cur->next){
        cur_region = yunos_list_entry(region_list_cur,k_mm_region_head_t,regionlist);
        head = &cur_region->alloced;
        end = head;

        for (cur = head->next; cur != end; cur = cur->next) {
            tmp = yunos_list_entry(cur, k_mm_region_list_t, list);
            start  = (void*)((uint32_t)tmp + sizeof(k_mm_region_list_t));
            if ((uint32_t)adress >= (uint32_t)start &&
                (uint32_t)adress < (uint32_t)start + tmp->len) {
                return 1;
           }
        }
    }
    return 0;
}

static uint32_t recheck(k_mm_region_list_t *node)
{
    void *start = (void*)(node + 1);
    void *end   = (void*)((uint32_t)start + node->len);
    void **p    = (void **)((uint32_t)start & ~(sizeof(size_t) - 1));

    g_recheck_flag = 1;

    while ((void *)p <= end) {
        if (NULL != p && 1 == if_adress_is_valid(*p)) {
            if ( 1 == check_mm_leak(*p)) {
                g_recheck_flag = 0;
                return 1;
            }
        }
        p++;
    }

    g_recheck_flag = 0;

    return 0;
}

uint32_t dump_mmleak()
{
    uint32_t            i    = 0;
    k_mm_region_list_t *min  = NULL;
    klist_t            *head = NULL;
    klist_t            *end  = NULL;
    klist_t            *cur;
    klist_t            *region_list_cur;
    klist_t            *region_list_head;
    k_mm_region_head_t *cur_region;

    yunos_sched_disable();

    NULL_PARA_CHK(g_mm_region_list_head.next);
    NULL_PARA_CHK(g_mm_region_list_head.prev);

    region_list_head = &g_mm_region_list_head;

    VGF(VALGRIND_MAKE_MEM_DEFINED(region_list_head, sizeof(k_mm_region_head_t)));

    for(region_list_cur = region_list_head->next; region_list_cur != region_list_head;){
        cur_region = yunos_list_entry(region_list_cur,k_mm_region_head_t,regionlist);

        VGF(VALGRIND_MAKE_MEM_DEFINED(cur_region,sizeof(k_mm_region_head_t)));

        head = &cur_region->alloced;
        end = head;

        if (1 == is_klist_empty(head)) {
            yunos_sched_enable();
            return 0;
        }

        for (cur = head->next; cur != end;) {
            min = yunos_list_entry(cur, k_mm_region_list_t, list);

            VGF(VALGRIND_MAKE_MEM_DEFINED(min,sizeof(k_mm_region_list_t)));

            if (YUNOS_MM_REGION_ALLOCED ==  min->type && 0 == check_mm_leak((uint8_t *)(min + 1)) && 0 == recheck(min)) {
#if (YUNOS_CONFIG_MM_DEBUG > 0)
                printf("[%-4d]:adress:0x%0x owner:0x%0x len:%-5d type:%s\r\n", i,
                           (uint32_t)min + sizeof(k_mm_region_list_t), min->owner,  min->len, "leak");
#else
                printf("[%-4d]:adress:0x%0x  len:%-5d type:%s\r\n", i,
                            (uint32_t)min + sizeof(k_mm_region_list_t), min->len, "leak");
#endif
            }
            i++;
            cur = cur->next;
            VGF(VALGRIND_MAKE_MEM_NOACCESS(min,sizeof(k_mm_region_list_t)));

        }

        region_list_cur = region_list_cur->next;
        VGF(VALGRIND_MAKE_MEM_NOACCESS(cur_region,sizeof(k_mm_region_head_t)));

    }
    yunos_sched_enable();

    return 1;
}
#endif


uint32_t dumpsys_mm_info_func(char *buf, uint32_t len)
{
    (void)buf;
    (void)len;
    uint32_t i = 0;
    k_mm_region_list_t *min  = NULL;
    klist_t *head = NULL;
    klist_t *end  = NULL;
    klist_t *cur  = NULL;
    klist_t * region_list_cur = NULL , *region_list_head = NULL;
    k_mm_region_head_t * cur_region;

    NULL_PARA_CHK(g_mm_region_list_head.next);
    NULL_PARA_CHK(g_mm_region_list_head.prev);

    region_list_head =  &g_mm_region_list_head;

    VGF(VALGRIND_MAKE_MEM_DEFINED(region_list_head, sizeof(k_mm_region_head_t)));

    for(region_list_cur = region_list_head->next; region_list_cur != region_list_head;){
        cur_region = yunos_list_entry(region_list_cur,k_mm_region_head_t,regionlist);

        VGF(VALGRIND_MAKE_MEM_DEFINED(cur_region,sizeof(k_mm_region_head_t)));

        printf("----------------------------------------------------------------------\r\n");
        printf("region info frag number:%d free size:%d\r\n", cur_region->frag_num, cur_region->freesize);

        head = &cur_region->probe;
        end = head;

        if (1 == is_klist_empty(head)) {
            printf("the memory list is empty\r\n");
            continue;
        }

        printf("free list: \r\n");

        for (cur = head->next; cur != end; ) {
            min = yunos_list_entry(cur, k_mm_region_list_t, list);

            VGF(VALGRIND_MAKE_MEM_DEFINED(min,sizeof(k_mm_region_list_t)));

#if (YUNOS_CONFIG_MM_DEBUG > 0)
            printf("[%-4d]:adress:0x%0x                  len:%-5d type:%s  flag:0x%0x\r\n", i,
                       (uint32_t)min + sizeof(k_mm_region_list_t), min->len, "free", min->dye);
#else
            printf("[%-4d]:adress:0x%0x                  len:%-5d type:%s\r\n", i,
                       (uint32_t)min + sizeof(k_mm_region_list_t), min->len, "free");
#endif
            i++;
            cur = cur->next;

            VGF(VALGRIND_MAKE_MEM_NOACCESS(min,sizeof(k_mm_region_list_t)));

        }
        i=0;
        head = &cur_region->alloced;
        end = head;
        printf("alloced list: \r\n");
        for (cur = head->next; cur != end;) {
            min = yunos_list_entry(cur, k_mm_region_list_t, list);

            VGF(VALGRIND_MAKE_MEM_DEFINED(min,sizeof(k_mm_region_list_t)));

#if (YUNOS_CONFIG_MM_DEBUG > 0)
            if ((YUNOS_MM_REGION_CORRUPT_DYE & min->dye) != YUNOS_MM_REGION_CORRUPT_DYE) {
                printf("[%-4d]:adress:0x%0x owner:0x%0x len:%-5d type:%s flag:0x%0x\r\n", i,
                           (uint32_t)min + sizeof(k_mm_region_list_t), min->owner,  min->len, "corrupt", min->dye);
            }
            printf("[%-4d]:adress:0x%0x owner:0x%0x len:%-5d type:%s flag:0x%0x\r\n", i,
                       (uint32_t)min + sizeof(k_mm_region_list_t), min->owner,  min->len, "taken", min->dye);
#else
            printf("[%-4d]:adress:0x%0x len:%-5d type:%s\r\n", i,
                       (uint32_t)min + sizeof(k_mm_region_list_t), min->len, "taken");

#endif
            i++;
            cur = cur->next;

            VGF(VALGRIND_MAKE_MEM_NOACCESS(min,sizeof(k_mm_region_list_t)));

        }

        region_list_cur = region_list_cur->next;

        VGF(VALGRIND_MAKE_MEM_NOACCESS(cur_region,sizeof(k_mm_region_head_t)));

    }
    return YUNOS_SUCCESS;
}



#endif


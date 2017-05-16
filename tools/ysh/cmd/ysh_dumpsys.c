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
#include <malloc.h>
#include <string.h>
#include <yos/log.h>
#include "k_api.h"
#include "ysh.h"

#include "ysh_dumpsys.h"

#define MM_LEAK_CHECK_ROUND_SCOND 10*60*5*1000
#define YUNOS_BACKTRACE_DEPTH     10

extern uint32_t dump_mmleak(void);

ktimer_t g_mm_leak_check_timer;

static uint32_t dumpsys_task_func(char *buf, uint32_t len, int detail)
{
    kstat_t    rst;
    size_t     free_size   = 0;
    sys_time_t time_total  = 0;
    char      *cpu_stat[9] = {"RDY", "PEND", "PEND_TO", "TO_SUS", "SUS",
                              "PEND_SUS", "DLY", "DLY_SUS", "DELETED"
                             };

    klist_t *taskhead = &g_kobj_list.task_head;
    klist_t *taskend  = taskhead;
    klist_t *tmp;
    ktask_t  *task;
    ktask_t  *candidate;
    const name_t  *task_name;
    char  yes = 'N';
    size_t pc = 0;
    size_t c_frame = 0;
    size_t n_frame = 0;
    int depth = YUNOS_BACKTRACE_DEPTH;

    yunos_sched_disable();
    preferred_ready_task_get(&g_ready_queue);
    candidate = g_preferred_ready_task;


    csp_printf("---------------------------------------------------------------------\n");

#if (YUNOS_CONFIG_CPU_USAGE_STATS > 0)
    csp_printf("CPU usage :%-10d   MAX:%-10d                 \n",
               g_cpu_usage / 100, g_cpu_usage_max / 100);
    csp_printf("---------------------------------------------------------------------\n");
#endif
    csp_printf("Name               State    Prio StackSize Freesize Runtime Candidate\n");
    csp_printf("---------------------------------------------------------------------\n");

    for (tmp = taskhead->next; tmp != taskend; tmp = tmp->next) {
        task       = yunos_list_entry(tmp, ktask_t, task_stats_item);
        rst        = yunos_task_stack_min_free(task, &free_size);

        if (rst != YUNOS_SUCCESS) {
            free_size = 0;
        }

#if (YUNOS_CONFIG_TASK_SCHED_STATS > 0)
        time_total = (sys_time_t)(task->task_time_total_run / 20);
#endif

        if (task->task_name != NULL)
        { task_name = task->task_name; }
        else
        { task_name = "anonym"; }

        if (candidate == task)
        { yes = 'Y'; }
        else
        { yes = 'N'; }

#ifndef HAVE_NOT_ADVANCED_FORMATE
        csp_printf("%-19.18s%-9s%-5d%-10d%-9zu%-9llu%-11c\n",
                   task_name, cpu_stat[task->task_state - K_RDY], task->prio,
                   task->stack_size, free_size, (unsigned long long)time_total, yes);
#else

        /* if not support %-N.Ms,cut it manually*/
        if (strlen(task_name) > 18) {
            char name_cut[19];
            memset(name_cut, 0, sizeof(name_cut));
            memcpy(name_cut, task->task_name, 18);
            task_name = name_cut;
        }

        csp_printf("%-19s%-9s%-5d%-10d%-9u%-9u%-11c\n",
                   task_name, cpu_stat[task->task_state - K_RDY], task->prio,
                   task->stack_size, (unsigned long)free_size, (unsigned long)time_total, yes);
#endif

        /* for chip not support stack frame interface,do nothing*/
        if (detail == true && task != g_active_task && soc_get_first_frame_info && soc_get_subs_frame_info) {
            depth = YUNOS_BACKTRACE_DEPTH;
            csp_printf("Task %s Call Stack Dump:\n", task_name);
            c_frame = (size_t)task->task_stack;
            soc_get_first_frame_info(c_frame, &n_frame, &pc);

            for (; (n_frame != 0) && (pc != 0) && (depth >= 0); --depth) {

                csp_printf("PC:0x%-12xSP:0x%-12x\n", c_frame, pc);
                c_frame = n_frame;
                soc_get_subs_frame_info(c_frame, &n_frame, &pc);
            }
        }
    }

    csp_printf("----------------------------------------------------------\n");
    yunos_sched_enable();

    return YUNOS_CMD_SUCCESS;
}

static uint32_t dumpsys_info_func(char *buf, uint32_t len)
{
    int16_t plen = 0;

    plen += sprintf(buf + plen, "---------------------------------------------\n");
#if (YUNOS_CONFIG_CPU_USAGE_STATS > 0)
    plen += sprintf(buf + plen, "CPU usage :%-10d     MAX:%-10d\n",
                    g_cpu_usage / 100, g_cpu_usage_max / 100);
#endif
#if (YUNOS_CONFIG_DISABLE_SCHED_STATS > 0)
    plen += sprintf(buf + plen, "Max sched disable time  :%-10d\n",
                    g_sched_disable_max_time);
#else
    plen += sprintf(buf + plen, "Max sched disable time  :%-10d\n", 0);
#endif
#if (YUNOS_CONFIG_DISABLE_INTRPT_STATS > 0)
    plen += sprintf(buf + plen, "Max intrpt disable time :%-10d\n",
                    g_intrpt_disable_max_time);
#else
    plen += sprintf(buf + plen, "Max intrpt disable time :%-10d\n", 0);
#endif
    plen += sprintf(buf + plen, "---------------------------------------------\n");

    return YUNOS_CMD_SUCCESS;
}


uint32_t dumpsys_mm_leak_func(char *buf, uint32_t len)
{
    dump_mmleak();
    return YUNOS_CMD_SUCCESS;
}

uint8_t mm_leak_timer_cb(void *timer, void *arg)
{
    dumpsys_mm_info_func(NULL, 0);
    return 0;
}

uint32_t dumpsys_mm_leak_check_func(cmd_item_t *item, char *buf, uint32_t len)
{
    static uint32_t run_flag = 0;
    sys_time_t round_sec = MM_LEAK_CHECK_ROUND_SCOND;

    if (NULL != item->items[2] && 0 == strcmp(item->items[2], "start")) {
        if (0 == run_flag) {
            if (NULL != item->items[3]) {
                round_sec = atoi(item->items[3]) * 1000;
            }

            yunos_timer_create(&g_mm_leak_check_timer, "mm_leak_check_timer", (timer_cb_t)mm_leak_timer_cb,
                               10, yunos_ms_to_ticks(round_sec), NULL, 0);
        } else {
            if (NULL != item->items[3]) {
                round_sec = atoi(item->items[3]) * 1000;

                if (1 == run_flag) {
                    yunos_timer_stop(&g_mm_leak_check_timer);
                }

                yunos_timer_change(&g_mm_leak_check_timer, 10, yunos_ms_to_ticks(round_sec));
            }
        }

        run_flag = 1;
        yunos_timer_start(&g_mm_leak_check_timer);
        return YUNOS_CMD_SUCCESS;
    }

    if (NULL != item->items[2] && 0 == strcmp(item->items[2], "stop")) {
        yunos_timer_stop(&g_mm_leak_check_timer);
        run_flag  = 2;
    }

    return YUNOS_CMD_SUCCESS;
}
static uint32_t cmd_dumpsys_func(char *buf, uint32_t len, cmd_item_t *item, cmd_info_t *info)
{
    ysh_stat_t ret;

    if ((NULL != item->items[1]) &&
        (0 == strcmp(item->items[1], "help") || 0 == strcmp(item->items[1], "?"))) {
        snprintf(buf, len, "%s\r\n", info->help_info);
        return YUNOS_CMD_SUCCESS;
    } else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "task")) {
        if ((NULL != item->items[2]) && (0 == strcmp(item->items[2], "detail"))) {
            ret = dumpsys_task_func(buf, len, true);
        } else
        { ret = dumpsys_task_func(buf, len, false); }

        return ret;
    }else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "info")) {
        ret = dumpsys_info_func(buf, len);
        return ret;
    } else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "mm_info")) {
        ret = dumpsys_mm_info_func(buf, len);
        return ret;
    } else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "mm_leak")) {
        ret = dumpsys_mm_leak_func(buf, len);
        return ret;
    } else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "leak_check")) {
        ret = dumpsys_mm_leak_check_func(item, buf, len);
        return ret;
    } else {
        snprintf(buf, len, "%s\r\n", info->help_info);
        return YUNOS_CMD_SUCCESS;
    }
}

void ysh_reg_cmd_dumpsys(void)
{
    cmd_info_t *tmp = NULL;

    tmp = (cmd_info_t*) yos_malloc(sizeof(cmd_info_t));

    if (tmp == NULL) {
        return;
    }

    tmp->cmd       = "dumpsys";
    tmp->info      = "dumpsys command show the system's runing info.";
    tmp->help_info = "dumpsys :\r\n"
                     "\tdumpsys task       : show the task info.\r\n"
                     "\tdumpsys mm_info    : show the memory has alloced.\r\n"
                     "\tdumpsys mm_leak    : show the memory maybe leak.\r\n"
                     "\tdumpsys leak_check : leak check control comand.\r\n"
                     "\tdumpsys info       : show the system info";
    tmp->func      = cmd_dumpsys_func;

    ysh_register_cmd(tmp);

    return;
}


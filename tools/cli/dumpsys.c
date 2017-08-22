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
#include <yos/kernel.h>
#include "k_api.h"

#define MM_LEAK_CHECK_ROUND_SCOND 10*60*5*1000
#define YUNOS_BACKTRACE_DEPTH     10

#if (YUNOS_CONFIG_MM_LEAKCHECK > 0)
extern uint32_t dump_mmleak(void);
#endif

ktimer_t g_mm_leak_check_timer;

#define safesprintf(buf,totallen,offset,string) do {\
    if((totallen - offset) < strlen(string)) { \
        printf("%s",buf); \
        offset = 0; \
    } \
    sprintf(buf+offset,"%s",string); \
    offset += strlen(string); \
    } while(0)

uint32_t dumpsys_task_func(char *buf, uint32_t len, int detail)
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

    char *printbuf = NULL;
    char  tmpbuf[256] ={0};
    int   offset   = 0;
    int   totallen = 2048;

    printbuf = yos_malloc(totallen);
    if(printbuf ==  NULL) {
        return YUNOS_NO_MEM;
    }
    memset(printbuf, 0, totallen);

    yunos_sched_disable();
    preferred_cpu_ready_task_get(&g_ready_queue, cpu_cur_get());
    candidate = g_preferred_ready_task[cpu_cur_get()];

    safesprintf(printbuf, totallen, offset, "---------------------------------------------------------------------\r\n");

#if (YUNOS_CONFIG_CPU_USAGE_STATS > 0)
    snprintf(tmpbuf, 255, "CPU usage :%-10d   MAX:%-10d                 \n");
               g_cpu_usage / 100, g_cpu_usage_max / 100);
    safesprintf(printbuf, totallen, offset,tmpbuf);
    safesprintf(printbuf, totallen, offset, "---------------------------------------------------------------------\r\n",255);

#endif
    safesprintf(printbuf, totallen, offset, "Name               State    Prio StackSize Freesize Runtime Candidate\r\n");
    safesprintf(printbuf, totallen, offset, "---------------------------------------------------------------------\r\n");

    for (tmp = taskhead->next; tmp != taskend; tmp = tmp->next) {
        task       = yunos_list_entry(tmp, ktask_t, task_stats_item);
        rst        = yunos_task_stack_min_free(task, &free_size);

        if (rst != YUNOS_SUCCESS) {
            free_size = 0;
        }

#if (YUNOS_CONFIG_TASK_SCHED_STATS > 0)
        time_total = (sys_time_t)(task->task_time_total_run / 20);
#endif

        if (task->task_name != NULL) {
            task_name = task->task_name;
        } else {
            task_name = "anonym";
        }

        if (candidate == task) {
            yes = 'Y';
        } else {
            yes = 'N';
        }

#ifndef HAVE_NOT_ADVANCED_FORMATE
        snprintf(tmpbuf, 255, "%-19.18s%-9s%-5d%-10d%-9zu%-9llu%-11c\r\n",
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

        snprintf(tmpbuf,255,"%-19s%-9s%-5d%-10d%-9u%-9u%-11c\r\n",
                   task_name, cpu_stat[task->task_state - K_RDY], task->prio,
                   task->stack_size, free_size, (unsigned int)time_total, yes);
#endif
        safesprintf(printbuf, totallen, offset,tmpbuf);

        /* for chip not support stack frame interface,do nothing*/
        if (detail == true && task != yunos_cur_task_get() && soc_get_first_frame_info &&
            soc_get_subs_frame_info) {
            depth = YUNOS_BACKTRACE_DEPTH;
            snprintf(tmpbuf, 255, "Task %s Call Stack Dump:\r\n", task_name);
            safesprintf(printbuf, totallen, offset,tmpbuf);
            c_frame = (size_t)task->task_stack;
            soc_get_first_frame_info(c_frame, &n_frame, &pc);

            for (; (n_frame != 0) && (pc != 0) && (depth >= 0); --depth) {

                snprintf(tmpbuf, 255, "PC:0x%-12xSP:0x%-12x\r\n", c_frame, pc);
                safesprintf(printbuf, totallen, offset,tmpbuf);
                c_frame = n_frame;
                soc_get_subs_frame_info(c_frame, &n_frame, &pc);
            }
        }
    }


    safesprintf(printbuf, totallen, offset,"----------------------------------------------------------\r\n");
    yunos_sched_enable();

    printf("%s",printbuf);
    yos_free(printbuf);
    return YUNOS_SUCCESS;
}

static uint32_t dumpsys_info_func(char *buf, uint32_t len)
{
    int16_t plen = 0;

    plen += sprintf(buf + plen,
                    "---------------------------------------------\r\n");
#if (YUNOS_CONFIG_CPU_USAGE_STATS > 0)
    plen += sprintf(buf + plen, "CPU usage :%-10d     MAX:%-10d\r\n",
                    g_cpu_usage / 100, g_cpu_usage_max / 100);
#endif
#if (YUNOS_CONFIG_DISABLE_SCHED_STATS > 0)
    plen += sprintf(buf + plen, "Max sched disable time  :%-10d\r\n",
                    g_sched_disable_max_time);
#else
    plen += sprintf(buf + plen, "Max sched disable time  :%-10d\r\n", 0);
#endif
#if (YUNOS_CONFIG_DISABLE_INTRPT_STATS > 0)
    plen += sprintf(buf + plen, "Max intrpt disable time :%-10d\r\n",
                    g_intrpt_disable_max_time);
#else
    plen += sprintf(buf + plen, "Max intrpt disable time :%-10d\r\n", 0);
#endif
    plen += sprintf(buf + plen,
                    "---------------------------------------------\r\n");

    return YUNOS_SUCCESS;
}

#if (YUNOS_CONFIG_MM_LEAKCHECK > 0)

uint32_t dumpsys_mm_leak_func(char *buf, uint32_t len)
{
    dump_mmleak();
    return YUNOS_SUCCESS;
}

uint8_t mm_leak_timer_cb(void *timer, void *arg)
{
    dumpsys_mm_info_func(NULL, 0);
    return 0;
}

uint32_t dumpsys_mm_leak_check_func(char *pcWriteBuffer, int xWriteBufferLen,
                                    int argc, char **argv)
{
    static uint32_t run_flag = 0;
    sys_time_t round_sec = MM_LEAK_CHECK_ROUND_SCOND;

    if (argc > 2 && 0 == strcmp(argv[2], "start")) {
        if (0 == run_flag) {
            if ( argc > 3 && NULL != argv[3]) {
                round_sec = atoi(argv[3]) * 1000;
            }

            yunos_timer_create(&g_mm_leak_check_timer, "mm_leak_check_timer",
                               (timer_cb_t)mm_leak_timer_cb,
                               10, yunos_ms_to_ticks(round_sec), NULL, 0);
        } else {
            if (NULL != argv[3]) {
                round_sec = atoi(argv[3]) * 1000;

                if (1 == run_flag) {
                    yunos_timer_stop(&g_mm_leak_check_timer);
                }

                yunos_timer_change(&g_mm_leak_check_timer, 10, yunos_ms_to_ticks(round_sec));
            }
        }

        run_flag = 1;
        yunos_timer_start(&g_mm_leak_check_timer);
        return YUNOS_SUCCESS;
    }

    if (argc == 2 && 0 == strcmp(argv[2], "stop")) {
        yunos_timer_stop(&g_mm_leak_check_timer);
        run_flag  = 2;
    }

    return YUNOS_SUCCESS;
}
#endif

uint32_t dumpsys_func(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                      char **argv)
{
    kstat_t ret;
    char *helpinfo = "dumpsys :\r\n"
                     "\tdumpsys task       : show the task info.\r\n"
                     "\tdumpsys task_stack : show the task stack info.\r\n"
                     "\tdumpsys mm_info    : show the memory has alloced.\r\n"
#if (YUNOS_CONFIG_MM_LEAKCHECK > 0)
                     "\tdumpsys mm_leak    : show the memory maybe leak.\r\n"
                     "\tdumpsys leak_check : leak check control comand.\r\n"
#endif
#if (YUNOS_CONFIG_CPU_USAGE_STATS > 0)
                     "\tdumpsys info       : show the system info\r\n"
#endif
                     ;
    if (argc >= 2  && 0 == strcmp(argv[1], "task")) {
        if (argc == 3 && (0 == strcmp(argv[2], "detail"))) {
            ret = dumpsys_task_func(pcWriteBuffer, xWriteBufferLen, true);
        } else {
            ret = dumpsys_task_func(pcWriteBuffer, xWriteBufferLen, false);
        }

        return ret;
    }
    else if (argc >= 2  && 0 == strcmp(argv[1], "task_stack")) {
        if (argc == 3) {
            ret = dump_task_stack_byname(argv[2]);
        } else {
            ret = dump_task_stack_byname(yunos_cur_task_get()->task_name);
        }

        return ret;
    }
    else if (argc == 2 && 0 == strcmp(argv[1], "info")) {
        ret = dumpsys_info_func(pcWriteBuffer, xWriteBufferLen);
        return ret;
    }

#if (YUNOS_CONFIG_MM_DEBUG> 0)
    else if (argc == 2 && 0 == strcmp(argv[1], "mm_info")) {
        ret = dumpsys_mm_info_func(NULL, 0);
        return ret;
    }
#endif

#if (YUNOS_CONFIG_MM_LEAKCHECK > 0)
    else if (argc == 2 && 0 == strcmp(argv[1], "mm_leak")) {
        ret = dumpsys_mm_leak_func(NULL, 0);
        return ret;
    } else if (argc > 2 && 0 == strcmp(argv[1], "leak_check")) {
        ret = dumpsys_mm_leak_check_func(pcWriteBuffer, xWriteBufferLen, argc, argv);
        return ret;
    }
#endif
    else {
        snprintf(pcWriteBuffer, xWriteBufferLen, "%s\r\n", helpinfo);
        return YUNOS_SUCCESS;
    }
}


int dump_task_stack(ktask_t *task)
{
    uint32_t offset = 0;
    kstat_t  rst    = YUNOS_SUCCESS;
    void    *cur, *end;
    int      i=0;
    int     *p;
    char     tmp[256]={0};

    char *printbuf = NULL;
    char  tmpbuf[256] ={0};
    int   bufoffset   = 0;
    int   totallen = 2048;

    printbuf = yos_malloc(totallen);
    if(printbuf ==  NULL) {
        return YUNOS_NO_MEM;
    }
    memset(printbuf, 0, totallen);
    yunos_sched_disable();

    end   = task->task_stack_base + task->stack_size;

    rst =  yunos_task_stack_cur_free(task, &offset);
    if (rst == YUNOS_SUCCESS) {
        cur = task->task_stack_base + task->stack_size - offset;
    } else {
        k_err_proc(YUNOS_SYS_SP_ERR);
        yos_free(printbuf);
        yunos_sched_enable();
        return 1;
    }
    p = (int*)cur;
    while(p < (int*)end) {
        if(i%4==0) {
            sprintf(tmp, "\r\n%08x:",(uint32_t)p);
            safesprintf(printbuf, totallen, bufoffset, tmp);
        }
        sprintf(tmp, "%08x ", *p);
        safesprintf(printbuf, totallen, bufoffset, tmp);
        i++;
        p++;
    }
    safesprintf(printbuf, totallen, bufoffset,
    "\r\n-----------------end----------------\r\n\r\n");
    yunos_sched_enable();

    printf("%s",printbuf);
    yos_free(printbuf);
    return 0;

}
int dump_task_stack_byname(char * taskname)
{

    klist_t *taskhead = &g_kobj_list.task_head;
    klist_t *taskend  = taskhead;
    klist_t *tmp;
    ktask_t *task;
    int      printall = 0;

    if(strcmp(taskname,"all") == 0) {
        printall = 1;
    }
    for (tmp = taskhead->next; tmp != taskend; tmp = tmp->next) {
        task = yunos_list_entry(tmp, ktask_t, task_stats_item);
        if(printall == 1 || strcmp(taskname, task->task_name) == 0){
            printf("------task %s stack -------",task->task_name);
            dump_task_stack(task);
        }
    }

    return 0;
}



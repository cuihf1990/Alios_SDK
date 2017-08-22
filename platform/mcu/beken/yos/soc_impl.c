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

#if (YUNOS_CONFIG_HW_COUNT > 0)
void soc_hw_timer_init(void)
{
}

hr_timer_t soc_hr_hw_cnt_get(void)
{
    return *(volatile uint64_t *)0xc0000120;
}

lr_timer_t soc_lr_hw_cnt_get(void)
{
    return 0;
}
#endif /* YUNOS_CONFIG_HW_COUNT */

#if (YUNOS_CONFIG_INTRPT_GUARD > 0)
void soc_intrpt_guard(void)
{
}
#endif

#if (YUNOS_CONFIG_INTRPT_STACK_REMAIN_GET > 0)
size_t soc_intrpt_stack_remain_get(void)
{
    return 0;
}
#endif

#if (YUNOS_CONFIG_INTRPT_STACK_OVF_CHECK > 0)
void soc_intrpt_stack_ovf_check(void)
{
}
#endif

#if (YUNOS_CONFIG_DYNTICKLESS > 0)
void soc_tick_interrupt_set(tick_t next_ticks,tick_t elapsed_ticks)
{
}

tick_t soc_elapsed_ticks_get(void)
{
    return 0;
}
#endif

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
extern void         *heap_start;
extern void         *heap_end;
extern void         *heap_len;

k_mm_region_t g_mm_region[] = {(uint8_t*)&heap_start,(size_t)&heap_len};

#endif

#if (YUNOS_CONFIG_MM_LEAKCHECK > 0 )

extern int _bss_start, _bss_end, _data_ram_begin, _data_ram_end;

void yos_mm_leak_region_init(void)
{
    yunos_mm_leak_region_init(&_bss_start, &_bss_end);
    yunos_mm_leak_region_init(&_data_ram_begin, &_data_ram_end);
}

size_t soc_get_cur_sp()
{
    size_t sp = 0;
    asm volatile(
        "mov %0,sp\n"
        :"=r"(sp));
    return sp;
}

#endif
static void soc_print_stack()
{

    uint32_t offset = 0;
    kstat_t  rst    = YUNOS_SUCCESS;
    void    *cur, *end;
    int      i=0;
    int     *p;

    end   = yunos_cur_task_get()->task_stack_base + yunos_cur_task_get()->stack_size;
    cur = soc_get_cur_sp();
    p = (int*)cur;
    while(p < (int*)end) {
        if(i%4==0) {
            printf("\r\n%08x:",(uint32_t)p);
        }
        printf("%08x ", *p);
        i++;
        p++;
    }
    printf("\r\n");
    return;
}
void soc_err_proc(kstat_t err)
{
    (void)err;
    soc_print_stack();
    assert(0);
}

yunos_err_proc_t g_err_proc = soc_err_proc;


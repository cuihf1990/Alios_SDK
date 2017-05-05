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

#define YOC_MM_ALLOC_DEPTH 0

#if (YUNOS_CONFIG_HW_COUNT > 0)
void soc_hw_timer_init(void)
{
}

hr_timer_t soc_hr_hw_cnt_get(void)
{
    return 0;
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
k_mm_region_t       g_mm_region;
k_mm_region_head_t  g_mm_region_head;

extern char _estack;
extern char _Min_Stack_Size;
extern char _end;

void soc_sys_mem_init(void)
{
    g_mm_region.start = &_end;
    g_mm_region.len   = (uint32_t)&_estack - (uint32_t)&_end - (uint32_t)&_Min_Stack_Size;

    yunos_mm_region_init(&g_mm_region_head, &g_mm_region, 1);
}

void *soc_mm_alloc(size_t size)
{
    void   *tmp      = NULL;
    kstat_t err      = YUNOS_SUCCESS;
    size_t  alloctor = (size_t)__builtin_return_address(YOC_MM_ALLOC_DEPTH);

    err = yunos_mm_bf_alloc(&g_mm_region_head, &tmp, size, alloctor);
    if (err != YUNOS_SUCCESS) {
        return NULL;
    }

    return tmp;
}


void soc_mm_free(void *mem)
{
    yunos_mm_bf_free(&g_mm_region_head, mem);
}
#endif

void soc_err_proc(kstat_t err)
{
}

#ifdef CONFIG_YOC_YSH
#include <stm32f4xx_hal_dma.h>
#include <stm32f4xx_hal_uart.h>

extern UART_HandleTypeDef UartHandle;

static inline char get_term_char(void)
{
    char c;

    HAL_UART_Receive(&UartHandle, &c, 1, 0xFFFF);

    return c;
}

int32_t soc_term_init(void)
{
    return 0;
}

int32_t soc_term_puts(ysh_ctrl_tbl_t *ctrl_tbl,  char *buf, uint32_t len)
{
    printf("%s", buf);
    return len;
}

int32_t soc_term_gets(ysh_ctrl_tbl_t *ctrl_tbl, char *buf, uint32_t len)
{
    char         in = 0;
    uint8_t      plen = 0;
    ysh_stdio_t *pstate = (ysh_stdio_t *)ctrl_tbl;
    char        *pin = pstate->cmd;
    int          escape = 0;

    while ((in = get_term_char()) != '\n') {
        if (in == '\r') {
            printf("\r\n");
			break;
        } else if (escape) {
            if (in != 0x5b || escape == 2) {
                escape = 0;
                continue;
            } else {
                escape = 2;
                continue;
            }
        } else if (in == 0x08) {    /* 0x08:Backspace */
            if (plen > 0) {
                plen--;
                pin--;
               *pin = 0;
                printf("\b\e[K");
            }

            continue;
        } else if (in == 0x1b) {
            escape = 1;
            continue;
        }

        printf("%c", in);
       *pin = in;
        pin++;
        plen++;
    }

    return plen;
}

void soc_term_exit(ysh_ctrl_tbl_t *ctrl_tbl, int32_t status)
{
}
#endif

yunos_err_proc_t g_err_proc = soc_err_proc;

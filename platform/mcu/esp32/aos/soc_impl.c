/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#if (RHINO_CONFIG_HW_COUNT > 0)
hr_timer_t soc_hr_hw_cnt_get(void)
{
    return 0;
}
#endif

#define SYS_DYN_POOL_SIZE_1 0x18000

k_mm_region_t g_mm_region[] = {{(uint8_t*)0x3ffe8000, SYS_DYN_POOL_SIZE_1}};
int           g_region_num  = sizeof(g_mm_region)/sizeof(k_mm_region_t);

void soc_err_proc(kstat_t err)
{
    printf("kernel panic,err %d!\n",err);
    assert(0);
}

krhino_err_proc_t g_err_proc = soc_err_proc;


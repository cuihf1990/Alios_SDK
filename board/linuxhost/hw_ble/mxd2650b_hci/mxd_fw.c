/****************************************************************************
 * Copyright (C) 2015 The YunOS Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mxd_type.h"
#include "ble_hci_adaptor.h"


extern uint8_t g_ble_iram[];
extern uint8_t g_rf_iram[];
extern uint32_t g_ble_iram_size;
extern uint32_t g_rf_iram_size;
extern void bt_sleep(int ms);

#define mxd_printf printf

static void mxd_write_phy_reg(u32 startaddr, u8 len, u32 *pbuf)
{
    int i = 0;
    u8 data[276];
    u8 *pp = data + 1;
    data[0] = 0x55;
    if (len == 0 || len > 64)
    {
        mxd_printf("len(%d) invalid,it shall  between 0 and 64\n", len);
        return;
    }
    UINT8_TO_STREAM(pp, len);
    UINT16_TO_STREAM(pp, 0);
    UINT32_TO_STREAM(pp, startaddr);
    for (i = 0; i < len; i++)
    {
        UINT32_TO_STREAM(pp, *pbuf);
        pbuf++;
    }

    hci_data_send(data, len * 4 + 8);
}

static int mxd_write_ram_code(int if_rf)
{
    //int i, tmp = 0;
    //int ch = 0;
    u32 startaddr = 0x00200500, fsize = 0, len = 0;
    u32 pos = 0;
    //u8 data[256];
    //u8 *pbb = data;
    u8 *pfmem = 0;

    if (0 == if_rf)
    {
        fsize = g_ble_iram_size;
        pfmem = g_ble_iram;
        mxd_printf("ble iram, size=%d\n", fsize);
    }
    else
    {
        fsize = g_rf_iram_size;
        pfmem = g_rf_iram;
        mxd_printf("rf iram, size=%d\n", fsize);
    }

    pos = 0;
    while (fsize > 0)
    {

        if (fsize > 60 * 4)
        {
            len = 60 * 4;
            fsize -= 60 * 4;
        }
        else
        {
            len = fsize;
            fsize = 0;
        }

        mxd_write_phy_reg(startaddr, len / 4, (u32 *)(pfmem + pos));
        mxd_printf("[write [start addr:0x%x--len-0x%x]]:\n", startaddr, len / 4);

        startaddr += len;
        pos += len;
    }
    return 0;
}

int mxd_fw_init(void)
{
    mxd_printf("download rf ram code\n");
    {
        u32 startaddr = 0x201f80, pos = 0x200500;
        if (1 == mxd_write_ram_code(1))
        {
            return -1;
        }

        mxd_write_phy_reg(startaddr, 1, &pos);
        mxd_printf("please wait .......\n");
        bt_sleep(1520);

        mxd_printf("download ram code successfully\n");
    }

    mxd_printf("download ble ram code\n");
    {
        u32 startaddr = 0x201f80, pos = 0;
        if (1 == mxd_write_ram_code(0))
        {
            return -1;
        }

        mxd_write_phy_reg(startaddr, 1, &pos);
        mxd_printf("please wait .......\n");
        bt_sleep(1520);
        mxd_printf("download ram code successfully\n");
    }
    return 0;
}

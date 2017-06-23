#include "include.h"
#include "arm_arch.h"

#if CFG_SUPPORT_MANUAL_CALI
#include "target_util_pub.h"
#include "mem_pub.h"

#include "drv_model_pub.h"
#include "sys_ctrl_pub.h"
#include "phy.h"

#include "bk7011_cal_pub.h"
#include "bk7011_cal.h"

#include <string.h>
#include "flash_pub.h"

/* yhb changed the ATE flash address */
#define TXPWR_TAB_FLASH_ADDR              (0xE000)
#define TXPWR_TAB_TAB                     (0x12345678)

#define WLAN_2_4_G_CHANNEL_NUM            (14)
#define MAX_RATE_FOR_B                    (11)    // 11M
#define MAX_RATE_FOR_G                    (54)   // 55M

#define TXPWR_ELEM_INUSED                 (1)
#define TXPWR_ELEM_UNUSED                 (0)

#define MCAL_DEBUG          1
#include "uart_pub.h"
#if MCAL_DEBUG
#define MCAL_PRT      os_printf
#define MCAL_WARN     os_printf
#define MCAL_FATAL    fatal_prf
#else
#define MCAL_PRT      null_prf
#define MCAL_WARN     null_prf
#define MCAL_FATAL    null_prf
#endif

typedef struct txpwr_st
{
    UINT8 mod;
    UINT8 pa;
    UINT16 valid;
} TXPWR_ST, *TXPWR_PTR;

typedef struct txpwr_elem_st
{
    UINT32 type;
    UINT32 len;
} TXPWR_ELEM_ST, *TXPWR_ELEM_PTR;

typedef enum
{
    TXPWR_ENABLE_ID     = 0,
    TXPWR_TAB_B_ID,
    TXPWR_TAB_G_ID
} TXPWR_ELEM_ID;

typedef enum
{
    TXPWR_NONE_RD     = 0u,
    TXPWR_TAB_B_RD    = 0x1u,
    TXPWR_TAB_G_RD    = 0x2u,
    TXPWR_TAB_N_RD    = 0x4u
} TXPWR_IS_RD;

typedef struct tag_txpwr_st
{
    TXPWR_ELEM_ST head;
} TAG_TXPWR_ST, *TAG_TXPWR_PTR;

typedef struct tag_enable_st
{
    TXPWR_ELEM_ST head;
    UINT32 flag;
} TAG_ENABLE_ST, *TAG_ENABLE_PTR;

typedef struct tag_txpwr_tab_st
{
    TXPWR_ELEM_ST head;
    TXPWR_ST tab[WLAN_2_4_G_CHANNEL_NUM];
} TAG_TXPWR_TAB_ST, *TAG_TXPWR_TAB_PTR;

TXPWR_PTR gtxpwr_tab_b = NULL;
TXPWR_PTR gtxpwr_tab_g = NULL;

static void manual_cal_start_fitting_txpwr(UINT32 rate)
{
    if((rate <= MAX_RATE_FOR_B) && (rate > 0))
    {
        if(!gtxpwr_tab_b)
        {
            gtxpwr_tab_b = (TXPWR_PTR)os_malloc(sizeof(TXPWR_ST) * WLAN_2_4_G_CHANNEL_NUM);
            if(!gtxpwr_tab_b)
            {
                MCAL_WARN("start fitting, no memory for txpwr tab b\r\n");
                return;
            }
            os_memset(gtxpwr_tab_b, 0, sizeof(TXPWR_ST)*WLAN_2_4_G_CHANNEL_NUM);
        }
    }
    else if(rate <= MAX_RATE_FOR_G)
    {
        if(!gtxpwr_tab_g)
        {
            gtxpwr_tab_g = (TXPWR_PTR)os_malloc(sizeof(TXPWR_ST) * WLAN_2_4_G_CHANNEL_NUM);
            if(!gtxpwr_tab_g)
            {
                MCAL_WARN("start fitting, no memory for txpwr tab g\r\n");
                return;
            }
            os_memset(gtxpwr_tab_g, 0, sizeof(TXPWR_ST)*WLAN_2_4_G_CHANNEL_NUM);
        }
    }
}

void manual_cal_save_txpwr(UINT32 rate, UINT32 channel, UINT32 mod, UINT32 pa)
{
    TXPWR_PTR txpwr_tab_ptr = NULL;

    if((channel == 0) || (channel > WLAN_2_4_G_CHANNEL_NUM))
    {
        MCAL_WARN("Manual cal wrong channel:%d\r\n", channel);
        return;
    }

    channel -= 1;
    manual_cal_start_fitting_txpwr(rate);

    if(rate <= MAX_RATE_FOR_B)
    {
        if(!gtxpwr_tab_b)
        {
            MCAL_WARN("txpwr tab b is null\r\n");
            return;
        }
        txpwr_tab_ptr = &gtxpwr_tab_b[channel];
    }
    else if(rate <= MAX_RATE_FOR_G)
    {
        if(!gtxpwr_tab_g)
        {
            MCAL_WARN("txpwr tab g is null\r\n");
            return;
        }
        txpwr_tab_ptr = &gtxpwr_tab_g[channel];
    }
    else
    {
        MCAL_FATAL("Manual cal wrong rate:%d\r\n", rate);
        return;
    }

    txpwr_tab_ptr->mod = mod;
    txpwr_tab_ptr->pa = pa;
    txpwr_tab_ptr->valid = TXPWR_ELEM_INUSED;
}

int manual_cal_get_txpwr(UINT32 rate, UINT32 channel, UINT32 *mod, UINT32 *pa)
{
    TXPWR_PTR txpwr_tab_ptr = NULL;

    if((channel == 0) || (channel > WLAN_2_4_G_CHANNEL_NUM))
    {
        MCAL_WARN("Manual cal wrong channel:%d\r\n", channel);
        return 0;
    }

    if((!gtxpwr_tab_b) && (!gtxpwr_tab_g))
    {
        //MCAL_WARN("txpwr tab is null\r\n");
        return 0;
    }

    channel -= 1;
    if(rate <= MAX_RATE_FOR_B)
    {
        if(!gtxpwr_tab_b)
            return 0;
        txpwr_tab_ptr = &gtxpwr_tab_b[channel];
    }
    else if(rate <= MAX_RATE_FOR_G)
    {
        if(!gtxpwr_tab_g)
            return 0;
        txpwr_tab_ptr = &gtxpwr_tab_g[channel];
    }
    else
    {
        MCAL_FATAL("Manual cal wrong rate:%d\r\n", rate);
        return 0;
    }

    if(txpwr_tab_ptr->valid == TXPWR_ELEM_INUSED)
    {
        *mod = txpwr_tab_ptr->mod;
        *pa = txpwr_tab_ptr->pa;
        return 1;
    }
    else
        return 0;


}


void manual_cal_show_txpwr_tab(void)
{
    TXPWR_PTR txpwr_tab_ptr = NULL;
    UINT32 i;

    if((!gtxpwr_tab_b) && (!gtxpwr_tab_g))
    {
        MCAL_WARN("txpwr tab is null\r\n");
        return;
    }

    MCAL_PRT("txpwr table for b:\r\n");
    if(gtxpwr_tab_b)
    {
        for(i = 0; i < WLAN_2_4_G_CHANNEL_NUM; i++)
        {
            txpwr_tab_ptr = &gtxpwr_tab_b[i];
            MCAL_PRT("ch:%2d: mod:%2d, pa:%2d, vaild:%2d\r\n",
                     i + 1, txpwr_tab_ptr->mod, txpwr_tab_ptr->pa, txpwr_tab_ptr->valid);
        }
    }

    MCAL_PRT("\r\ntxpwr table for g:\r\n");
    if(gtxpwr_tab_g)
    {
        for(i = 0; i < WLAN_2_4_G_CHANNEL_NUM; i++)
        {
            txpwr_tab_ptr = &gtxpwr_tab_g[i];
            MCAL_PRT("ch:%2d: mod:%2d, pa:%2d, vaild:%2d\r\n",
                     i + 1, txpwr_tab_ptr->mod, txpwr_tab_ptr->pa, txpwr_tab_ptr->valid);
        }
    }
}

TXPWR_IS_RD manual_cal_txpwr_tab_is_fitted(void)
{
    TXPWR_IS_RD is_ready = TXPWR_NONE_RD;
    TXPWR_PTR txpwr_tab_ptr = NULL;
    UINT32 i;

    if(gtxpwr_tab_b)
    {
        for(i = 0; i < WLAN_2_4_G_CHANNEL_NUM; i++)
        {
            txpwr_tab_ptr = &gtxpwr_tab_b[i];
            if(txpwr_tab_ptr->valid == TXPWR_ELEM_UNUSED)
            {
                MCAL_WARN("txpwr table b[%d] is unused\r\n", i);
                break;
            }
        }
        if(i == WLAN_2_4_G_CHANNEL_NUM)
            is_ready |= TXPWR_TAB_B_RD;
    }


    if(gtxpwr_tab_g)
    {
        for(i = 0; i < WLAN_2_4_G_CHANNEL_NUM; i++)
        {
            txpwr_tab_ptr = &gtxpwr_tab_g[i];
            if(txpwr_tab_ptr->valid == TXPWR_ELEM_UNUSED)
            {
                MCAL_WARN("txpwr table g[%d] is unused\r\n", i);
                break;
            }
        }
        if(i == WLAN_2_4_G_CHANNEL_NUM)
            is_ready |= TXPWR_TAB_G_RD;
    }
    return is_ready;
}

static void manual_cal_do_fitting(TXPWR_PTR dst, TXPWR_PTR srclow, TXPWR_PTR srchigh)
{
    UINT8 mod = 0;
    UINT8 pa = 0;

    if((srclow->valid == TXPWR_ELEM_UNUSED) || 
       (srchigh->valid == TXPWR_ELEM_UNUSED))
        return;

    mod = (srclow->mod + srchigh->mod) / 2;
    pa = (srclow->pa + srchigh->pa) / 2;
    dst->mod = mod;
    dst->pa = pa;
    dst->valid = TXPWR_ELEM_INUSED;
}

UINT32 manual_cal_fitting_txpwr_tab(void)
{
    TXPWR_PTR tab_ptr = NULL;

    if((!gtxpwr_tab_b) && (!gtxpwr_tab_g))
    {
        MCAL_WARN("txpwr tab is null\r\n");
        return 0;
    }

    // for b, check ch1, ch7, ch13 is in used
    if(gtxpwr_tab_b)
    {
        tab_ptr = gtxpwr_tab_b;
        if((!tab_ptr[0].valid) || (!tab_ptr[6].valid) || (!tab_ptr[12].valid))
        {
            MCAL_WARN("txpwr table for b fitting failed!, ch1 ch7 ch13 unused\r\n");
            return 0;
        }

        // fitting ch4, use ch1, ch7
        manual_cal_do_fitting(&tab_ptr[3], &tab_ptr[0], &tab_ptr[6]);
        // fitting ch2, use ch1, ch4
        manual_cal_do_fitting(&tab_ptr[1], &tab_ptr[0], &tab_ptr[3]);
        // fitting ch3, use ch2, ch4
        manual_cal_do_fitting(&tab_ptr[2], &tab_ptr[1], &tab_ptr[3]);
        // fitting ch5, use ch4, ch7
        manual_cal_do_fitting(&tab_ptr[4], &tab_ptr[3], &tab_ptr[6]);
        // fitting ch6, use ch5, ch7
        manual_cal_do_fitting(&tab_ptr[5], &tab_ptr[4], &tab_ptr[6]);

        // fitting ch10, use ch7, ch13
        manual_cal_do_fitting(&tab_ptr[9], &tab_ptr[6], &tab_ptr[12]);
        // fitting ch8, use ch7, ch10
        manual_cal_do_fitting(&tab_ptr[7], &tab_ptr[6], &tab_ptr[9]);
        // fitting ch9, use ch8, ch10
        manual_cal_do_fitting(&tab_ptr[8], &tab_ptr[7], &tab_ptr[9]);
        // fitting ch11, use ch10, ch13
        manual_cal_do_fitting(&tab_ptr[10], &tab_ptr[9], &tab_ptr[12]);
        // fitting ch12, use ch11, ch13
        manual_cal_do_fitting(&tab_ptr[11], &tab_ptr[10], &tab_ptr[12]);

        // fitting ch14, the same as ch13
        manual_cal_do_fitting(&tab_ptr[13], &tab_ptr[12], &tab_ptr[12]);
    }

    // for g
    if(gtxpwr_tab_g)
    {
        tab_ptr = gtxpwr_tab_g;
        if((!tab_ptr[0].valid) || (!tab_ptr[6].valid) || (!tab_ptr[12].valid))
        {
            MCAL_WARN("txpwr table for g fitting failed!, ch1 ch7 ch13 unused\r\n");
            return 0;
        }

        // fitting ch4, use ch1, ch7
        manual_cal_do_fitting(&tab_ptr[3], &tab_ptr[0], &tab_ptr[6]);
        // fitting ch2, use ch1, ch4
        manual_cal_do_fitting(&tab_ptr[1], &tab_ptr[0], &tab_ptr[3]);
        // fitting ch3, use ch2, ch4
        manual_cal_do_fitting(&tab_ptr[2], &tab_ptr[1], &tab_ptr[3]);
        // fitting ch5, use ch4, ch7
        manual_cal_do_fitting(&tab_ptr[4], &tab_ptr[3], &tab_ptr[6]);
        // fitting ch6, use ch5, ch7
        manual_cal_do_fitting(&tab_ptr[5], &tab_ptr[4], &tab_ptr[6]);

        // fitting ch10, use ch7, ch13
        manual_cal_do_fitting(&tab_ptr[9], &tab_ptr[6], &tab_ptr[12]);
        // fitting ch8, use ch7, ch10
        manual_cal_do_fitting(&tab_ptr[7], &tab_ptr[6], &tab_ptr[9]);
        // fitting ch9, use ch8, ch10
        manual_cal_do_fitting(&tab_ptr[8], &tab_ptr[7], &tab_ptr[9]);
        // fitting ch11, use ch10, ch13
        manual_cal_do_fitting(&tab_ptr[10], &tab_ptr[9], &tab_ptr[12]);
        // fitting ch12, use ch11, ch13
        manual_cal_do_fitting(&tab_ptr[11], &tab_ptr[10], &tab_ptr[12]);

        // fitting ch14, the same as ch13
        manual_cal_do_fitting(&tab_ptr[13], &tab_ptr[12], &tab_ptr[12]);
    }

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
static UINT32 manual_cal_search_txpwr_tab(UINT32 type, UINT32 start_addr)
{
    UINT32 status, addr, end_addr;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    ddev_read(flash_handle, (char *)&head, sizeof(TXPWR_ELEM_ST), start_addr);
    addr = start_addr + sizeof(TXPWR_ELEM_ST);
    end_addr = addr + head.len;
    while(addr < end_addr)
    {
        ddev_read(flash_handle, (char *)&head, sizeof(TXPWR_ELEM_ST), addr);
        if(type == head.type)
        {
            break;
        }
        else
        {
            addr += sizeof(TXPWR_ELEM_ST);
            addr += head.len;
        }
    }

    if(addr >= end_addr)
    {
        addr = 0;
    }
    ddev_close(flash_handle);

    return addr;
}

static void manual_cal_update_flash_area(char *buf, UINT32 len)
{
    DD_HANDLE flash_handle;
    UINT32 param, param1, status;

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);

    param1 = 2 | (0 << 8);
    ddev_control(flash_handle, CMD_FLASH_WRITE_SR, &param1);

    param1 = TXPWR_TAB_FLASH_ADDR;
    ddev_control(flash_handle, CMD_FLASH_ERASE_SECTOR, &param1);

    status = ddev_write(flash_handle, (char *)buf, len, TXPWR_TAB_FLASH_ADDR);

    if(status != FLASH_SUCCESS)
    {
        MCAL_FATAL("save txpwr tab to flash failed\r\n");
    }

    param = 2 | (0x400C << 8);
    ddev_control(flash_handle, CMD_FLASH_WRITE_SR, &param);

    MCAL_PRT("manual_cal_write_flash ok\r\n");
}

int manual_cal_load_txpwr_tab_flash(void)
{
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;
    TXPWR_IS_RD is_ready_flash = 0;

    addr_start = TXPWR_TAB_FLASH_ADDR;
    addr = manual_cal_search_txpwr_tab(TXPWR_ENABLE_ID, TXPWR_TAB_FLASH_ADDR);
    if(!addr)
    {
        MCAL_WARN("NO TXPWR_ENABLE_ID found in flash\r\n");
        return -1;
    }

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    ddev_read(flash_handle, (char *)&head, sizeof(TXPWR_ELEM_ST), addr);
    ddev_read(flash_handle, (char *)&status, head.len, addr + sizeof(TXPWR_ELEM_ST));
    is_ready_flash = status;

    // If txpwr tab in flash is unused, we should use auto calibration result
    if(is_ready_flash == TXPWR_NONE_RD)
    {
        MCAL_WARN("txpwr tabe in flash is unused\r\n");
        return -1;
    }

    // for txpwr b
    if(is_ready_flash & TXPWR_TAB_B_RD)
    {
        addr = manual_cal_search_txpwr_tab(TXPWR_TAB_B_ID, addr_start);
        if(addr)
        {
            gtxpwr_tab_b = (TXPWR_PTR)os_malloc(sizeof(TXPWR_ST) * WLAN_2_4_G_CHANNEL_NUM);
            if(!gtxpwr_tab_b)
            {
                MCAL_FATAL("no memory for txpwr tab b\r\n");
                return -1;
            }
            ddev_read(flash_handle, (char *)&head, sizeof(TXPWR_ELEM_ST), addr);
            ddev_read(flash_handle, (char *)gtxpwr_tab_b, head.len, addr + sizeof(TXPWR_ELEM_ST));
        }
        else
        {
            MCAL_WARN("txpwr tabe b in flash no found\r\n");
        }
    }

    // for txpwr g
    if(is_ready_flash & TXPWR_TAB_G_RD)
    {
        addr = manual_cal_search_txpwr_tab(TXPWR_TAB_G_ID, addr_start);
        if(addr)
        {
            gtxpwr_tab_g = (TXPWR_PTR)os_malloc(sizeof(TXPWR_ST) * WLAN_2_4_G_CHANNEL_NUM);
            if(!gtxpwr_tab_g)
            {
                MCAL_FATAL("no memory for txpwr tab g\r\n");
                return -1;
            }
            ddev_read(flash_handle, (char *)&head, sizeof(TXPWR_ELEM_ST), addr);
            ddev_read(flash_handle, (char *)gtxpwr_tab_g, head.len, addr + sizeof(TXPWR_ELEM_ST));
        }
        else
        {
            MCAL_WARN("txpwr tabe g in flash no found\r\n");
        }
    }

    MCAL_PRT("read txpwr tab from flash success\r\n");

    return 0;
}

int manual_cal_save_txpwr_tab_flash(void)
{
    TXPWR_IS_RD is_ready, is_ready_flash;
    UINT32 len = 0;
    UINT8 *buf = NULL;
    TAG_TXPWR_PTR tag_txpwr = NULL;
    TAG_ENABLE_PTR tag_enable = NULL;
    TAG_TXPWR_TAB_PTR tag_tab = NULL;

    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;

    is_ready = manual_cal_txpwr_tab_is_fitted();
    MCAL_PRT("current txpwr table:0x%x\r\n");
    if(is_ready == TXPWR_NONE_RD)
    {
        MCAL_WARN("TXPWR_NONE_RD, Cann't save txpwr tabe in flash\r\n");
        return 0;
    }

    // alloc all memery at onece, so we no need to change the size of buf in combin function
    len = sizeof(TAG_TXPWR_ST) + sizeof(TAG_ENABLE_ST) + 2 * sizeof(TAG_TXPWR_TAB_ST);
    // for flash bug, don't know how
    len += 4;

    buf = (UINT8 *)os_malloc(len);
    if(!buf)
    {
        MCAL_FATAL("no memory for txpwr tab save to flash\r\n");
        return 0;
    }

    // read flash, then combin the table in flash
    is_ready_flash = TXPWR_NONE_RD;
    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    status = status;

    addr_start = TXPWR_TAB_FLASH_ADDR;
    addr = manual_cal_search_txpwr_tab(TXPWR_ENABLE_ID, TXPWR_TAB_FLASH_ADDR);
    if(addr)
    {
        ddev_read(flash_handle, (char *)&head, sizeof(TXPWR_ELEM_ST), addr);
        ddev_read(flash_handle, (char *)&is_ready_flash, head.len, addr + sizeof(TXPWR_ELEM_ST));
        MCAL_PRT("flash txpwr table:0x%x\r\n", is_ready_flash);
    }

    // for tag TXPW
    tag_txpwr = (TAG_TXPWR_PTR)buf;
    tag_txpwr->head.type = TXPWR_TAB_TAB;
    tag_txpwr->head.len = len - sizeof(TAG_TXPWR_ST);

    // for tag TXPWR_ENABLE_ID
    tag_enable = (TAG_ENABLE_PTR)(buf + sizeof(TAG_TXPWR_ST));
    tag_enable->head.type = TXPWR_ENABLE_ID;
    tag_enable->head.len = sizeof(UINT32);
    tag_enable->flag = (UINT32)is_ready;

    // for tag TXPWR_TAB_B_ID
    tag_tab = (TAG_TXPWR_TAB_PTR)(buf + sizeof(TAG_TXPWR_ST) + sizeof(TAG_ENABLE_ST));
    tag_tab->head.type = TXPWR_TAB_B_ID;
    tag_tab->head.len = sizeof(TXPWR_ST) * WLAN_2_4_G_CHANNEL_NUM;
    if(is_ready & TXPWR_TAB_B_RD)
    {
        os_memcpy(&tag_tab->tab[0], gtxpwr_tab_b, tag_tab->head.len);
    }
    else if(is_ready_flash & TXPWR_TAB_B_RD)
    {
        addr = manual_cal_search_txpwr_tab(TXPWR_TAB_B_ID, addr_start);
        if(addr)
        {
            ddev_read(flash_handle, (char *)&head, sizeof(TXPWR_ELEM_ST), addr);
            ddev_read(flash_handle, (char *)&tag_tab->tab[0], head.len, addr + sizeof(TXPWR_ELEM_ST));
        }
        else
        {
            MCAL_PRT("txpwr tabe b in flash no found\r\n");
        }
    }

    // for tag TXPWR_TAB_G_ID
    tag_tab = (TAG_TXPWR_TAB_PTR)(buf + sizeof(TAG_TXPWR_ST) + sizeof(TAG_ENABLE_ST)
                                  + sizeof(TAG_TXPWR_TAB_ST));
    tag_tab->head.type = TXPWR_TAB_G_ID;
    tag_tab->head.len = sizeof(TXPWR_ST) * WLAN_2_4_G_CHANNEL_NUM;
    if(is_ready & TXPWR_TAB_G_RD)
    {
        os_memcpy(&tag_tab->tab[0], gtxpwr_tab_g, tag_tab->head.len);
    }
    else if(is_ready_flash & TXPWR_TAB_G_RD)
    {
        addr = manual_cal_search_txpwr_tab(TXPWR_TAB_G_ID, addr_start);
        if(addr)
        {
            ddev_read(flash_handle, (char *)&head, sizeof(TXPWR_ELEM_ST), addr);
            ddev_read(flash_handle, (char *)&tag_tab->tab[0], head.len, addr + sizeof(TXPWR_ELEM_ST));
        }
        else
        {
            MCAL_PRT("txpwr tabe g in flash no found\r\n");
        }
    }
    ddev_close(flash_handle);

    // show
    {
        UINT32 i = 0;
        MCAL_PRT("\r\nshow txpwr tags before write flash:\r\n");
        for(i = 0; i < len; i++)
        {
            MCAL_PRT("%02x,", buf[i]);
            if((i + 1) % 16 == 0)
                MCAL_PRT("\r\n");
        }
        MCAL_PRT("\r\n");
    }

    manual_cal_update_flash_area((char *)buf, len);
    os_free(buf);

    return 1;
}

#endif



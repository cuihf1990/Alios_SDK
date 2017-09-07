/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <errno.h>
#include <execinfo.h>
#include "ysh.h"

#ifdef CONFIG_YSH_SIMULATE
#define YSH_QUEUE_BUF_SIZE 80
#define YSH_QUEUE_MSG_MAX  79
uint8_t     g_ysh_queue_init   = 0;
kbuf_queue_t g_ysh_simulate_queue;
char       *g_ysh_queue_buf[YSH_QUEUE_BUF_SIZE];
#endif

uint32_t    g_ysh_mode = 0;

void yoc_set_simulate_mode(void)
{
    g_ysh_mode = 1;
}

int32_t yoc_ram_datasize_get(void)
{
    return 0;
}

int32_t yoc_ram_bsssize_get(void)
{
    return 0;
}

int32_t yoc_ram_totalsize_get(void)
{
    return 0;
}

int32_t soc_term_init(void)
{
    return 0;
}

int32_t soc_term_puts(ysh_ctrl_tbl_t *ctrl_tbl,  char *buf, uint32_t len)
{
    printf("%s", buf);
    fflush(stdout);
    return len;
}

#ifdef CONFIG_YSH_SIMULATE
int32_t soc_term_gets_simulate(ysh_ctrl_tbl_t *ctrl_tbl, char *buf, uint32_t len)
{
    kstat_t        ret;
    size_t         size   = 0;
    tick_t         yield_ticks = RHINO_CONFIG_TIME_SLICE_DEFAULT;

    if (g_ysh_queue_init == 0) {
        ret = krhino_buf_queue_create(&g_ysh_simulate_queue, "test_ysh_simulate_queue",
                                     (void *)&g_ysh_queue_buf,
                                      YSH_QUEUE_BUF_SIZE, YSH_QUEUE_MSG_MAX);

        if (ret != RHINO_SUCCESS) {
            return -1;
        }
        g_ysh_queue_init = 1;
    }

    do {
       ret = krhino_buf_queue_recv(&g_ysh_simulate_queue, yield_ticks, buf, &size);
       if (ret != RHINO_SUCCESS) {
           continue;
       } else {
           return size;
       }
    } while(1);
}
#endif
int32_t soc_term_gets_impl(ysh_ctrl_tbl_t *ctrl_tbl, char *buf, uint32_t len)
{
    int            in = 0;
    uint32_t       plen = 0;
    char          *pin = buf;
    fd_set         rfds;
    struct timeval tv;
    int            retval;
    tick_t         yield_ticks = RHINO_CONFIG_TIME_SLICE_DEFAULT;

    /* watch stdin (fd 0) to see when it has input. */
    do {
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);

        /* no wait */
        tv.tv_sec  = 0;
        tv.tv_usec = 0;

        retval = select(1, &rfds, NULL, NULL, &tv);

        if (retval < 0) {
            if(errno == EINTR){
                continue;
            }
            return -1;
        }

        if (retval == 0) {krhino_task_sleep(yield_ticks); continue;}

        /* not block for select tells */
        while ((in = getchar()) != '\n') {
            if (in < 0 && errno != EINTR) {
                return -1;
            }
            *pin = in;
            pin++;
            plen++;
        }

        return plen;

    } while (1);
}

int32_t soc_term_gets(ysh_ctrl_tbl_t *ctrl_tbl, char *buf, uint32_t len)
{
    uint32_t plen = 0;

    if (0 == g_ysh_mode) {
        plen = soc_term_gets_impl(ctrl_tbl, buf, len);
        return plen;
    }
#ifdef CONFIG_YSH_SIMULATE
    if (1 == g_ysh_mode) {
        plen = soc_term_gets_simulate(ctrl_tbl, buf, len);
        return plen;
    }
#endif
    return 0;
}

void soc_term_release(ysh_ctrl_tbl_t *ctrl_tbl)
{
    return;
}

void soc_term_exit(ysh_ctrl_tbl_t *ctrl_tbl, int32_t status)
{
    soc_term_release(ctrl_tbl);
}

int32_t yoc_backtrace(void **buf, int32_t size)
{
    return backtrace(buf, size);
}

char  **yoc_backtrace_symbols(void *const *buf, int32_t size)
{
    return backtrace_symbols(buf, size);
}


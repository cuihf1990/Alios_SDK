/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <aos/aos.h>
#include <lwip/sockets.h>
#include <lwip/apps/tftp.h>

static void* tftp_server_open(const char* fname, const char* mode, u8_t write)
{
    FILE *fp = NULL;

    if (strncmp(mode, "netascii", 8) == 0) {
        fp = fopen(fname, write == 0 ? "r" : "w");
    } else if (strncmp(mode, "octet", 5) == 0) {
        fp = fopen(fname, write == 0 ? "rb" : "wb");
    }
    return (void*)fp;
}

static void tftp_server_close(void* handle)
{
    fclose((FILE*)handle);
}

static int tftp_server_read(void* handle, void* buf, int bytes)
{
    size_t readbytes;
    readbytes = fread(buf, 1, (size_t)bytes, (FILE*)handle);
    return (int)readbytes;
}

static int tftp_server_write(void* handle, struct pbuf* p)
{
    char buff[512];
    size_t writebytes = -1;
    pbuf_copy_partial(p, buff, p->tot_len, 0);
    writebytes = fwrite(buff, 1, p->tot_len, (FILE *)handle);
    return (int)writebytes;
}

const tftp_context_t server_ctx = {
    .open = tftp_server_open,
    .close = tftp_server_close,
    .read = tftp_server_read,
    .write = tftp_server_write
};

static void* tftp_client_open(const char* fname, const char* mode, u8_t write)
{
    FILE *fp = NULL;

    if (strncmp(mode, "netascii", 8) == 0) {
        fp = fopen(fname, write == 0 ? "r" : "w");
    } else if (strncmp(mode, "octet", 5) == 0) {
        fp = fopen(fname, write == 0 ? "rb" : "wb");
    }
    return (void*)fp;
}

static void tftp_client_close(void* handle)
{
    fclose((FILE*)handle);
}

static int tftp_client_read(void* handle, void* buf, int bytes)
{
    size_t readbytes;
    readbytes = fread(buf, 1, (size_t)bytes, (FILE*)handle);
    return (int)readbytes;
}

static int tftp_client_write(void* handle, struct pbuf* p)
{
    char buff[512];
    size_t writebytes = -1;
    pbuf_copy_partial(p, buff, p->tot_len, 0);
    writebytes = fwrite(buff, 1, p->tot_len, (FILE *)handle);
    return (int)writebytes;
}

const tftp_context_t client_ctx = {
    .open = tftp_client_open,
    .close = tftp_client_close,
    .read = tftp_client_read,
    .write = tftp_client_write
};

void gateway_tftp_server_start(void)
{
    tftp_server_init(&server_ctx);
}

void gateway_tftp_server_stop(void)
{
    tftp_server_deinit();
}


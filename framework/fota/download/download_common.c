/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <aos/aos.h>
#include <string.h>

#include "md5.h"
#include "ota_download.h"
#include "ota_util.h"
#include "ota_log.h"
#include "ota_update_manifest.h"


#define KEY_OTA_BREAKPOINT  "key_ota_breakpoint"
#define KEY_OTA_MD5         "key_ota_md5"
#define KEY_OTA_MD5_CTX     "key_ota_md5_ctx"

#define OTA_BUFFER_MAX_SIZE  1536

void save_state(uint32_t breakpoint, MD5_CTX *pMD5)
{
    ota_set_update_breakpoint(breakpoint);
    ota_set_cur_MD5_context(pMD5);
}

uint32_t ota_get_update_breakpoint()
{
    uint32_t offset = 0;
    int len = 4;

    if (aos_kv_get(KEY_OTA_BREAKPOINT, &offset, &len)) {
        offset = 0;
    }
    //OTA_LOG_I("ota_get_update_breakpoint=%d",offset);
    return offset;
}

int ota_set_update_breakpoint(uint32_t offset)
{
    //OTA_LOG_I("ota_set_update_breakpoint=%d",offset);
    return  aos_kv_set(KEY_OTA_BREAKPOINT, &offset, 4, 1);
}

int ota_get_last_MD5(char *value)
{
    int len = 33;
    int ret = aos_kv_get(KEY_OTA_MD5, value, &len);
    return ret;
}

int ota_set_cur_MD5(char *value)
{
    return  aos_kv_set(KEY_OTA_MD5, value, 33, 1);
}

int ota_get_last_MD5_context(MD5_CTX *md5ctx)
{
    int len = sizeof(MD5_CTX);
    int ret = aos_kv_get(KEY_OTA_MD5_CTX, md5ctx, &len);
    return ret;
}

int ota_set_cur_MD5_context(MD5_CTX *md5ctx)
{
    return  aos_kv_set(KEY_OTA_MD5_CTX, md5ctx, sizeof(MD5_CTX), 1);
}

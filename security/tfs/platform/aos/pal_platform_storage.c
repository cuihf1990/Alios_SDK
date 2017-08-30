/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include "log.h"
#include "yos/framework.h"

#define TAG_PAL_STORAGE "PAL_STORAGE"

#define MAX_BUF_LEN 64

int pal_save_info(const char *key, char *value)
{
    LOGD(TAG_PAL_STORAGE, "[%s]: enter.\n", __func__);
    if (key == NULL || value == NULL) {
        LOGE(TAG_PAL_STORAGE, "[%s]: para error.\n", __func__);
        return -1;
    }

    return yos_kv_set(key, value, strlen(value), 1);
}

int pal_get_info(const char *key, char *value)
{
    int ret;
    char buf[MAX_BUF_LEN + 1] = {0};
    int buf_len = MAX_BUF_LEN + 1;

    LOGD(TAG_PAL_STORAGE, "[%s]: enter.\n", __func__);

    if (key == NULL || value == NULL) {
        LOGE(TAG_PAL_STORAGE, "[%s]: para error.\n", __func__);
        return -1;
    }

    ret = yos_kv_get(key, buf, &buf_len);

    if (!ret) {
        strcpy(value, (const char *) buf);
    }

    LOGD(TAG_PAL_STORAGE, "[%s]: exit.\n", __func__);
    return ret;
}

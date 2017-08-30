#include <stdio.h>
#include "hal.h"
#include "log.h"

#define CMD         0X01
#define RESPONSE    0X07

#define TAG_CMD "TFS_HAL_CMD"

//static pthread_mutex_t g_tfs_hal_lock = PTHREAD_MUTEX_INITIALIZER;

static int _verify_package(uint32_t cmd, uint8_t *package, uint32_t len);

int fill_package(uint8_t *package, uint32_t cmd, uint8_t *arg, uint32_t arg_len)
{
    int i = 0;
    int sum = 0;
    int cmd_len = 0;
    int pack_len = 0;

    LOGD(TAG_CMD, "[%s]: enter.\n", __func__);
    if (package == NULL) {
        LOGE(TAG_CMD, "[%s]: fill package error!\n", __func__);
        return -1;
    }

    cmd_len = 1 + arg_len;
    pack_len = 2 + 1 + 2 + cmd_len + 2;

    /* HEAD */
    *package++ = 0XEF;
    *package++ = 0X01;
    /* FLAG */
    *package++ = CMD;
    /* LENGTH */
    *package++ = (cmd_len >> 8) & 0XFF;
    *package++ = cmd_len & 0XFF;
    /* CMD */
    *package = cmd & 0XFF;
    sum += *package++;

    /* ARG */
    for (i = 0; i < arg_len; i++) {
        sum += *(arg + i);
        *package++ = *(arg + i);
    }

    /* SUM */
    sum %= 0XFFFF;
    *package++ = (sum >> 8) & 0XFF;
    *package = sum & 0XFF;

    return pack_len;
}

int hal_cmd(uint32_t cmd, void *in, uint32_t in_len, void *out,
            uint32_t *out_len)
{
    int ret = 0;
    void *handle = NULL;

    //ret = pthread_mutex_lock(&g_tfs_hal_lock);
    LOGD(TAG_CMD, "[%s]: enter.\n", __func__);
    if (ret != 0) {
        LOGE(TAG_CMD, "[%s]: Failed to acquire tfs lock!\n", __func__);
        return -1;
    }

    ret = open_session(&handle);
    if (ret != 0) {
        LOGE(TAG_CMD, "[%s]: open_session error!\n", __func__);
        //pthread_mutex_unlock(&g_tfs_hal_lock);
        return -1;
    }

    ret = invoke_command(handle, cmd, in, in_len, out, out_len);
    if (ret != 0) {
        LOGE(TAG_CMD, "[%s]: invoke_command error!\n", __func__);
        close_session(handle);
        //pthread_mutex_unlock(&g_tfs_hal_lock);
        return -1;
    }

    close_session(handle);

    //ret = pthread_mutex_unlock(&g_tfs_hal_lock);
    if (ret != 0) {
        LOGE(TAG_CMD, "[%s]: Failed to release tfs lock!\n", __func__);
    }

    return _verify_package(cmd, out, *out_len);
}

static int _verify_package(uint32_t cmd, uint8_t *package, uint32_t len)
{
    int i;
    int sum = 0;

    LOGD(TAG_CMD, "[%s]: enter.\n", __func__);
    if ((*package != 0XEF) || (*(package + 1) != 0X01) ||
        (*(package + 2) != 0X07) || (*(package + 5) != cmd)) {
        LOGE(TAG_CMD, "[%s]: verify package error!\n", __func__);
        return -1;
    }
    for (i = 5; i < len - 2; i++) {
        sum += *(package + i);
    }

    if ((((sum >> 8) & 0XFF) == *(package + i)) &&
        ((sum & 0XFF) == *(package + i + 1))) {
        return 0;
    } else {
        LOGE(TAG_CMD, "[%s]: sum verify error!\n", __func__);
        return -1;
    }
}

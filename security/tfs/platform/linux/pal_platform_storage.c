/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "log.h"

#define TAG_PAL_STORAGE "PAL_STORAGE"

int pal_save_info(const char *key, char *value) {
    FILE *fp = NULL;
    int fd = -1;
    int writed = -1;

    LOGD(TAG_PAL_STORAGE, "[%s]: enter.\n", __func__);
    if (key == NULL || value == NULL) {
        LOGE(TAG_PAL_STORAGE, "[%s]: para error\n", __func__);
        return -1;
    }

    // create activate file
    mode_t mask = umask(0);
    fd = open(key, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    umask(mask);
    if (fd == -1) {
        LOGE(TAG_PAL_STORAGE, "[%s]: open file %s failed: %s\n", __func__, key, strerror(errno));
        return -1;
    }

    fp = fdopen(fd, "wb");
    if (fp == NULL) {
        LOGE(TAG_PAL_STORAGE, "[%s]: can not find %s\n", __func__, key);
        return -1;
    }

    writed = fwrite(value, 1, strlen(value) + 1, fp);
    fflush(fp);
    fclose(fp);
    close(fd);
    sync();
    if (writed != (int)(strlen(value) + 1)) {
        LOGE(TAG_PAL_STORAGE, "[%s]: write file %s failed: %s\n", __func__, key, strerror(errno));
        return -1;
    }

    return 0;
}


int pal_get_info(const char *key, char *value) {
    FILE *fp = NULL;
    int file_len = -1;
    int readed = -1;

    LOGD(TAG_PAL_STORAGE, "[%s]: enter.\n", __func__);
    if (key == NULL || value == NULL) {
        LOGE(TAG_PAL_STORAGE, "[%s]: para error\n", __func__);
        return -1;
    }

    fp = fopen(key, "rb");
    if (fp == NULL) {
        LOGE(TAG_PAL_STORAGE, "[%s]: can not find %s\n", __func__, key);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    file_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (file_len < 0) {
        fclose(fp);
        LOGE(TAG_PAL_STORAGE, "[%s]: %s is too short\n", __func__, key);
        return -1;
    }

    readed = fread(value, 1, file_len, fp);
    fclose(fp);

    if (readed < file_len) {
        LOGE(TAG_PAL_STORAGE, "[%s]: read data error\n", __func__);
        return -1;
    }

    return 0;
}

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
#include "yos/conf.h"

#define TAG_PAL_STORAGE "PAL_STORAGE"

#define MAX_BUF_LEN 64

int pal_save_info(const char *key, char *value) {

    return hal_flash_conf_write(NULL, key, (unsigned char *)value, strlen(value));
}

int pal_get_info(const char *key, char *value) {
    int ret;
    char buf[MAX_BUF_LEN+1] = {0};

    ret = hal_flash_conf_read(NULL, key, buf, MAX_BUF_LEN);
    if(!ret)
        strcpy(value,(const char *) buf);

    return ret;
}

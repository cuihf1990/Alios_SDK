/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 *
 * Alibaba Group retains all right, title and interest (including all
 * intellectual property rights) in and to this computer program, which is
 * protected by applicable intellectual property laws.  Unless you have
 * obtained a separate written license from Alibaba Group., you are not
 * authorized to utilize all or a part of this computer program for any
 * purpose (including reproduction, distribution, modification, and
 * compilation into object code), and you must immediately destroy or
 * return to Alibaba Group all copies of this computer program.  If you
 * are licensed by Alibaba Group, your rights to utilize this computer
 * program are limited by the terms of that license.  To obtain a license,
 * please contact Alibaba Group.
 *
 * This computer program contains trade secrets owned by Alibaba Group.
 * and, unless unauthorized by Alibaba Group in writing, you agree to
 * maintain the confidentiality of this computer program and related
 * information and to not disclose this computer program and related
 * information to any other person or entity.
 *
 * THIS COMPUTER PROGRAM IS PROVIDED AS IS WITHOUT ANY WARRANTIES, AND
 * Alibaba Group EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * INCLUDING THE WARRANTIES OF MERCHANTIBILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, TITLE, AND NONINFRINGEMENT.
 */

#ifndef ATTRS_PROFILE_H
#define ATTRS_PROFILE_H

#include "service.h"
#include "os.h"

#define MAX_MODEL   128
#define MD5_LEN  33
#define MAX_URL_LEN 384
#define PROFILE_DIR_NAME    "profile_lua"


typedef struct profile_info {
    char model[OS_PRODUCT_MODEL_LEN];
    uint32_t model_id;
    char filename[STR_LONG_LEN];
    char md5[MD5_LEN];
}profile_info_t;

typedef struct pfnode {
    list_head_t url_node; /* url_list */
    list_head_t list_node; /* g_list */
    struct profile_info info;
    char *url;
} pfnode_t;

int get_device_profile_file(uint8_t dev_type, uint32_t model_id,
        char *file_name, int max_name_length);
int get_global_profile(char *file_name, int max_name_length);

int attrs_profile_init(void);
int attrs_profile_exit(void);

#endif

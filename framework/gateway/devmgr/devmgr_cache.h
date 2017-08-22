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

#ifndef __DEVMGR_CACHE__H__
#define __DEVMGR_CACHE__H__

#include "devmgr.h"
#include <unistd.h>
#ifdef __cplusplus
extern "C" {
#endif

void __dump_attr_cache(dlist_t *attr_head);

int devmgr_get_attr_cache(const char *devid_or_uuid, const char *attr_name,
                          char *attr_value_buff, int buff_size);
int devmgr_read_attr_cache(const char *devid_or_uuid, const char *attr_name,
                           char **attr_value);
int devmgr_update_attr_cache(const char *devid_or_uuid, const char *attr_name,
                             const char *attr_value);
void devmgr_free_device_cache(dev_info_t *devinfo);
int devmgr_cache_init();
void devmgr_cache_exit();

#ifdef __cplusplus
}
#endif
#endif


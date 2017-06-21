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

#ifndef __STDD_PROFILE_H__
#define __STDD_PROFILE_H__
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include "list.h"
#include "json_parser.h"

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

typedef struct name_mapping_s{
    struct list_head list_node;
    char user_name[32];
    char std_name[32];
}name_mapping_t;


typedef struct endpoint_profile_s{
    struct list_head list_node;
    uint8_t endpoint_id;
    dlist_t attr_head;
    dlist_t cmd_head;
}endpoint_profile_t;


typedef struct dev_profile_s{
    struct list_head list_node;
    uint32_t model_id;
    char *profile_name;//profile文件名称
    char *attr_set; //全部属性名称，格式: ["OnOff","MaxPower"]
    char *cmd_set;  //全部命令名称，格式: ["moveHue"]
    struct list_head endpoint_head;
}dev_profile_t;


typedef struct data_list_s{
    struct list_head list_node;
    void *data;
}data_list_t;

int stdd_parse_attr_profile(char *profile_str, dlist_t *profile_head);

int stdd_parse_cmd_profile(char *profile_str, dlist_t *profile_head);

int stdd_parse_str_set(char *str_set, dlist_t *strset_head);

int stdd_parse_endpoint_profile(char *profile_str, dlist_t *profile_head);

void stdd_free_device_profile(dev_profile_t *profile);

void stdd_free_attr_profile(dlist_t *data_list_head);

void stdd_free_cmd_profile(dlist_t *data_list_head);

void stdd_free_device_profile_list(dlist_t *head);

void __dump_attr_profile_list(dlist_t *head);

void __dump_cmd_profile_list(dlist_t *head);

void __dump_endpoint_profile_list(dlist_t *head);

void __dump_device_profile(dev_profile_t *profile);

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif

#endif


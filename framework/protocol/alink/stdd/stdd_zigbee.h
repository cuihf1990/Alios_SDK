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

#ifndef STDD_ZBNET_H
#define STDD_ZBNET_H

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

int stdd_zbnet_report_attrs(unsigned char ieee_addr[8], unsigned char endpoint_id,
    const char *attr_name[], const char *attr_value[]);

int stdd_zbnet_get_attr(const char *devid_or_uuid, const char *attr_name);

int stdd_zbnet_set_attr(const char *devid_or_uuid, const char *attr_name, const char *attr_value);

int stdd_zbnet_exec_rpc(const char *devid_or_uuid, const char *rpc_name, const char *rpc_args);

int stdd_zbnet_report_event(unsigned char ieee_addr[8], unsigned char endpoint_id,
        const char *event_name, const char *event_args);

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif

#endif


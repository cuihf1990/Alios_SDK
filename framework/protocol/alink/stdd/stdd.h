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

#ifndef STDD_PROFILE_H
#define STDD_PROFILE_H
#include <stdint.h>
#include <unistd.h>
#include "yos/list.h"
#include "service.h"
#include "stdd_datatype.h"
#include "devmgr.h"

//#define __STDD_UT__   1
#ifdef __STDD_UT__
#include "stdd_ut.h"
#endif

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

#define MAX_PROFILE_NAME_LEN            128
/*
* 仅用于线下环境子设备入网时从配置文件中获取子设备secret，
* 根据设备上报的rand计算出签名值，并注册到云端
*/
#define DEVICE_KEY_CONFIG_FILE_NAME "proxy_config.lua"
#define DEVICE_KEY_CONFIG_NAME      "key_config"
#define DEVICE_SECRET_NAME          "secret"
const char *stdd_get_subdev_secret(uint32_t short_model, char *secret_buff,
                                   uint32_t buff_size);

//load新的profile
int32_t stdd_update_global_profile(const char *profile_path);
int32_t stdd_update_device_profile(uint8_t dev_type, uint32_t model_id,
                                   const char *profile_path);
int32_t stdd_add_device_profile(uint8_t dev_type, uint32_t model_id,
                                const char *profile_path);
int32_t stdd_check_device_profile(uint8_t dev_type, uint32_t model_id);
int32_t stdd_convert_attrname_cloud2terminal(uint32_t model_id,
                                             const char *src_attr,
                                             uint8_t *endpoint_id, char *dest_attr, uint32_t max_attr_len);
int32_t stdd_convert_attrname_terminal2cloud(uint32_t model_id,
                                             uint16_t endpoint_id,
                                             const char *src_attr, char *dest_attr, uint32_t max_attr_len);
int32_t stdd_convert_cmdname_terminal2cloud(uint32_t model_id,
                                            uint16_t endpoint_id,
                                            const char *cmd_name, char *dest_attr, uint32_t max_attr_len);
int32_t stdd_convert_rpcname_cloud2terminal(int32_t model_id,
                                            const char *user_rpc,
                                            uint8_t *endpoint_id, char *cmd_name, unsigned max_cmd_len);
int32_t stdd_get_attr_mapping_set(uint32_t short_model, const char *alink_attr,
                                  uint8_t *endpoint_id,
                                  char *name_array[], int *array_size);
int32_t stdd_get_cmd_mapping_name(uint32_t short_model, const char *cmd_name,
                                  uint8_t *endpoint_id,
                                  char *target_cmd_name, uint32_t max_cmd_len);
int32_t stdd_get_alink_attr_set(uint32_t short_model, uint16_t endpoint_id,
                                const char *cmdname_or_attrname,
                                char *name_array[], int *array_size);

int stdd_get_device_attr(const char *uuid, const char *attr_name,
                         char *attr_value_buff, int buff_size);

int32_t stdd_get_custom_attr(const char *dev_id, uint32_t model_id,
                             const char *attr_name);
int32_t stdd_set_custom_attr(const char *dev_id, uint32_t model_id,
                             const char *attr_name, const char *attr_value);
int32_t stdd_exec_custom_cmd(const char *dev_id, uint32_t model_id,
                             const char *cmd_name, const char *cmd_args);
int32_t stdd_report_custom_event(const char *dev_id, uint32_t model_id,
                                 unsigned char endpoint_id,
                                 const char *event_name, const char *event_args);
int32_t stdd_report_custom_attr(const char *dev_id, uint32_t model_id,
                                unsigned char endpoint_id,
                                const char *attr_name, const char *attr_value);
int stdd_forward_device_status(dev_info_t *devinfo,
                               const char *attr_name, const char *attr_value);
int stdd_forward_device_event(dev_info_t *devinfo,
                              const char *event_name, const char *event_value);


/*json格式，属性名称数组*/
int stdd_get_device_attrset(const char *devid_or_uuid, char *attrset_buff,
                            int buff_size);

/*json format, get sub-device RPCs*/
int stdd_get_device_service(dev_info_t *devinfo, char *service_buff,
                            int buff_size);

/*获取所有设备信息回掉函数,Topic:  /getDeviceDataArray   APP  ---> GW Client */
int stdd_get_all_device_state(char *out_buffer, int *buffer_len);

//lmns依赖接口
//app -> gw,payload:{"uuid":"","attrSet":["attrName",...]}
int stdd_get_device_data(const char *devid, const char *payload);

//建议屏蔽该方法,app -> gw,payload:[{"uuid":"","attrSet":["attrName",...]}]
int stdd_get_device_data_array(const char *payload);

//app -> gw, payload:{"uuid":"","attrSet":["attrName",...],"attrName":{"value":"","when":""},...}
int stdd_set_device_status(const char *devid, const char *payload);

int stdd_get_device_profile_name(uint8_t dev_type, uint32_t model_id,
                                 char *file_name, int max_name_length);


void *stdd_new_buff(unsigned int buff_size);

void *stdd_dup_string(const char *src);

void stdd_free_buff(void *buff);

int stdd_init();

void stdd_exit();

void stdd_dump_attr_profile();

void stdd_dump_cmd_profile();

void stdd_dump_device_profile();

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif

#endif


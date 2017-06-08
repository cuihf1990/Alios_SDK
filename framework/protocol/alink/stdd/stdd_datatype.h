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

#ifndef STDD_H
#define STDD_H
#include <stdint.h>
#include <unistd.h>
#include "alink_export_zigbee.h"

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif


/*数据类型分类定义*/
#define ALINK_DATATYPE_CLASS_NODATA                 0x01
#define ALINK_DATATYPE_CLASS_DATA                   0x02
#define ALINK_DATATYPE_CLASS_LOGIC                  0x03
#define ALINK_DATATYPE_CLASS_BITMAP                 0x04
#define ALINK_DATATYPE_CLASS_UINT                   0x05
#define ALINK_DATATYPE_CLASS_INT                    0x06
#define ALINK_DATATYPE_CLASS_ENUM                   0x07
#define ALINK_DATATYPE_CLASS_FLOAT                  0x08
#define ALINK_DATATYPE_CLASS_STRING                 0x09
#define ALINK_DATATYPE_CLASS_STRUCTURE              0x0a
#define ALINK_DATATYPE_CLASS_COLLECTION             0x0b
#define ALINK_DATATYPE_CLASS_OTHER                  0xff


/*** Data Types ***/
#define ZCL_DATATYPE_NO_DATA                            0x00
#define ZCL_DATATYPE_DATA8                              0x08    //DATA类型字符串用小写hex string表示
#define ZCL_DATATYPE_DATA16                             0x09
#define ZCL_DATATYPE_DATA24                             0x0a
#define ZCL_DATATYPE_DATA32                             0x0b
#define ZCL_DATATYPE_DATA40                             0x0c
#define ZCL_DATATYPE_DATA48                             0x0d
#define ZCL_DATATYPE_DATA56                             0x0e
#define ZCL_DATATYPE_DATA64                             0x0f
#define ZCL_DATATYPE_BOOLEAN                            0x10    //BOOLEAN类型字符串用10进制字符串表示
#define ZCL_DATATYPE_BITMAP8                            0x18    //BITMAP类型字符串用10进制字符串表示
#define ZCL_DATATYPE_BITMAP16                           0x19
#define ZCL_DATATYPE_BITMAP24                           0x1a
#define ZCL_DATATYPE_BITMAP32                           0x1b
#define ZCL_DATATYPE_BITMAP40                           0x1c
#define ZCL_DATATYPE_BITMAP48                           0x1d
#define ZCL_DATATYPE_BITMAP56                           0x1e
#define ZCL_DATATYPE_BITMAP64                           0x1f
#define ZCL_DATATYPE_UINT8                              0x20    //UINT类型字符串用10进制数值表示
#define ZCL_DATATYPE_UINT16                             0x21
#define ZCL_DATATYPE_UINT24                             0x22
#define ZCL_DATATYPE_UINT32                             0x23
#define ZCL_DATATYPE_UINT40                             0x24
#define ZCL_DATATYPE_UINT48                             0x25
#define ZCL_DATATYPE_UINT56                             0x26
#define ZCL_DATATYPE_UINT64                             0x27
#define ZCL_DATATYPE_INT8                               0x28    //INT类型字符串用10进制数值表示
#define ZCL_DATATYPE_INT16                              0x29
#define ZCL_DATATYPE_INT24                              0x2a
#define ZCL_DATATYPE_INT32                              0x2b
#define ZCL_DATATYPE_INT40                              0x2c
#define ZCL_DATATYPE_INT48                              0x2d
#define ZCL_DATATYPE_INT56                              0x2e
#define ZCL_DATATYPE_INT64                              0x2f
#define ZCL_DATATYPE_ENUM8                              0x30    //ENUM类型字符串用10进制数值表示
#define ZCL_DATATYPE_ENUM16                             0x31    //PREC类型字符串用10进制数值表示
#define ZCL_DATATYPE_SEMI_PREC                          0x38
#define ZCL_DATATYPE_SINGLE_PREC                        0x39
#define ZCL_DATATYPE_DOUBLE_PREC                        0x3a
#define ZCL_DATATYPE_OCTET_STR                          0x41
#define ZCL_DATATYPE_CHAR_STR                           0x42
#define ZCL_DATATYPE_LONG_OCTET_STR                     0x43
#define ZCL_DATATYPE_LONG_CHAR_STR                      0x44
#define ZCL_DATATYPE_ARRAY                              0x48
#define ZCL_DATATYPE_STRUCT                             0x4c
#define ZCL_DATATYPE_SET                                0x50
#define ZCL_DATATYPE_BAG                                0x51
#define ZCL_DATATYPE_TOD                                0xe0
#define ZCL_DATATYPE_DATE                               0xe1    //DATA类型字符串用10进制数值表示
#define ZCL_DATATYPE_UTC                                0xe2    //UTC类型字符串用10进制数值表示
#define ZCL_DATATYPE_CLUSTER_ID                         0xe8    //CLUSTER_ID类型字符串用hex string表示
#define ZCL_DATATYPE_ATTR_ID                            0xe9    //ATTR_ID类型字符串用hex string表示
#define ZCL_DATATYPE_BAC_OID                            0xea
#define ZCL_DATATYPE_IEEE_ADDR                          0xf0    //IEEE_ADDR类型字符串用小写hex string表示
#define ZCL_DATATYPE_128_BIT_SEC_KEY                    0xf1    //IEEE_ADDR类型字符串用小写hex string表示
#define ZCL_DATATYPE_UNKNOWN                            0xff


/***
 * @desc:    更新网关模组端属性profile
 * @para:    profile: 设备属性profile结构指针数组，以NULL结束
 *           同一cluster下存在同名属性profile则覆盖原有的
 * @retc:    ZBNET_SUCCESS/ZBNET_FAILURE
 */
typedef int32_t (*zigbee_update_attr_profile_cb_t)(attr_profile_t *profile[]);


/***
 * @desc:    更新网关模组端命令profile
 * @para:    profile: 设备命令profile结构指针数组，以NULL结束
 *           同一cluster下存在同名命令profile则覆盖原有的
 * @retc:    ZBNET_SUCCESS/ZBNET_FAILURE
 */
typedef int32_t (*zigbee_update_cmd_profile_cb_t)(cmd_profile_t *profile[]);



/***
 * @desc:    获取设备属性接口，设备属性通过设备状态
 *           上报接口异步上报
 * @para:    ieee_addr: 设备8Byte ieee地址
 *           endpoint_id: 1Byte endpoint id
 *           attr_set: 要获取的属性名称指针数组，数组最后元素为NULL
 * @retc:    ZBNET_SUCCESS/ZBNET_FAILURE
 */
typedef int32_t (*zigbee_get_attr_cb_t)(uint8_t ieee_addr[IEEE_ADDR_BYTES], \
                                    uint8_t endpoint_id, const char *attr_set);


/***
 * @desc:    设置设备单个属性接口
 * @para:    ieee_addr: 设备8Byte ieee地址
 *           endpoint_id: 1Byte endpoint id
 *           attr_name: 属性名称
 *           attr_value: 字符串属性值
 * @retc:    ZBNET_SUCCESS/ZBNET_FAILURE
 */
typedef int32_t (*zigbee_set_attr_cb_t)(uint8_t ieee_addr[IEEE_ADDR_BYTES], uint8_t endpoint_id, \
                                    const char *attr_name, const char *attr_value);


/***
 * @desc:    执行设备命令接口
 * @para:    ieee_addr: 设备8Byte ieee地址
 *           endpoint_id: 1Byte endpoint id
 *           cmd_name: 命令名称
 *           cmd_args: json字符串格式命令参数
 * @retc:    ZBNET_SUCCESS/ZBNET_FAILURE
 */
typedef int32_t (*zigbee_exec_cmd_cb_t)(uint8_t ieee_addr[IEEE_ADDR_BYTES], uint8_t endpoint_id, \
                                    const char *cmd_name, const char *cmd_args);


/***
 * @desc:    移除设备接口
 * @para:    ieee_addr: 设备8Byte ieee地址
 * @retc:    ZBNET_SUCCESS/ZBNET_FAILURE
 */
typedef int32_t (*zigbee_remove_device_cb_t)(uint8_t ieee_addr[IEEE_ADDR_BYTES]);


/***
 * @desc:    开始permitjoin接口
 * @para:    duration: permitjoin持续时间，单位:秒，取值范围:1-255,255表示永久
 * @retc:    ZBNET_SUCCESS/ZBNET_FAILURE
 */
typedef int32_t (*zigbee_permit_join_cb_t)(uint8_t duration);


/*
*属性数据类型结构体
*/
typedef struct data_type_s{
    uint8_t type_class;
    int8_t length;//0xff表示为变长
    char *name;
    uint8_t id;
}data_type_t;


data_type_t *stdd_get_datatype_by_name(const char *name, int name_len);
data_type_t *stdd_get_datatype_by_id(uint8_t type_id);

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif

#endif


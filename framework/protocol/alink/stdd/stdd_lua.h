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

#ifndef __STDD_LUA_H__
#define __STDD_LUA_H__
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include "list.h"
#include "json_parser.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

/*设备profile中require C libs定义*/
#define ZIGBEE_LIB_NAME             "zigbee_lib"
#define COMMON_LIB_NAME             "common_lib"
#define API_LIB_NAME                "api_lib"


/*设备profile中lua全局变量名称定义*/
#define LUA_GLOBAL_ATTR_PROFILE     "global_cluster_attr_profile"
#define LUA_GLOBAL_CMD_PROFILE      "global_cluster_cmd_profile"
#define LUA_PRIVATE_ATTR_PROFILE    "private_cluster_attr_profile"
#define LUA_PRIVATE_CMD_PROFILE     "private_cluster_cmd_profile"
#define LUA_ENDPOINT_PROFILE        "device_endpoint_profile"
#define LUA_DEVICE_ATTR_SET         "device_attr_set"
#define LUA_DEVICE_CMD_SET          "device_cmd_set"


/*lua函数调用alink api接口名称定义*/
/*
 * @param (uint8_t *devid, uint32_t model_id, uint8_t *secret)
 * @retval 0:success, other:failure
 */
#define ALINK_ZIGBEE_REGISTER_DEVICE           "c_alink_zigbee_register_device"

/*
 * @param(uint8_t *devid)
 * @retval 0:success, other:failure
 */
#define ALINK_ZIGBEE_UNREGISTER_DEVICE         "c_alink_zigbee_unregister_device"

/*
 * @param (uint8_t *devid, uint8_t endpoint_id, uint8_t *attr_name,
            uint8_t *attr_value)
 * @retval 0:success, other:failure
 */
#define ALINK_ZIGBEE_REPORT_STATUS             "c_alink_zigbee_report_status"

/*
 * @param (uint8_t *devid, uint8_t endpoint_id, uint8_t *event_name,
            uint8_t *event_args)
 * @retval 0:success, other:failure
 */
#define ALINK_ZIGBEE_REPORT_EVENT              "c_alink_zigbee_report_event"

/*
 * @param (uint8_t *devid, uint8_t online_or_not)
 * @retval 0:success,other:failure
 */
#define ALINK_ZIGBEE_UPDATE_ONLINE_STATUS      "c_alink_zigbee_update_online_status"



/*lua函数调用通用设备的c接口名称定义*/
/*
 * @param (uint8_t *devid, uint8_t *attr_name)
 * @retval nil:failure, other:attr_value
 */
#define C_FUNC_GET_DEVICE_ATTR_CACHE    "c_get_attr_cache"

/*
 * args: (uint8_t *devid, uint8_t *attr_name)
 * @retval nil:failure, other:attr_value
 */
#define C_FUNC_GET_GLOBAL_DEVICE_ATTR   "c_get_global_dev_attr"

/*
 * args: (uint8_t *devid, uint8_t *attr_name, uint8_t *attr_value)
 * @retval 0:success, other:failure
 */
#define C_FUNC_SET_GLOBAL_DEVICE_ATTR   "c_set_global_dev_attr"

/*
 * args: (uint32_t short_model, uint8_t *attr_name)
 * @retval nil:failure, other:zigbee attribute name json array
 */
#define C_FUNC_GET_ATTR_MAPPING_SET     "c_get_attr_mapping_set"

/*
 * args: (uint32_t short_model, uint8_t *rpc_name)
 * @retval nil:failure, other:zigbee command name
 */
#define C_FUNC_GET_CMD_MAPPING_NAME     "c_get_cmd_mapping_name"

/*
 * args: (uint32_t short_model, uint8_t endpoint_id, uint8_t *attr_name)
 * @retval nil:failure, other:alink attribute name json array
 */
#define C_FUNC_GET_ALINK_ATTR_SET       "c_get_alink_attr_set"


/*lua脚本调用zigbee设备操作c接口定义*/
/*
 * args: (uint8_t *devid, uint8_t endpoint_id, uint8_t *attr_name)
 * @retval 0:success, other:failure
 */
#define C_FUNC_GET_DEVICE_ATTR          "c_get_device_attr"

/*
 * args: (uint8_t *devid, uint8_t endpoint_id, uint8_t *attr_name,
            uint8_t *attr_value)
 * @retval 0:success, other:failure
 */
#define C_FUNC_SET_DEVICE_ATTR          "c_set_device_attr"

/*
 * args: (uint8_t *devid, uint8_t endpoint_id, uint8_t *cmd_name,
            uint8_t *cmd_args)
 * @retval 0:success, other:failure
 */
#define C_FUNC_EXEC_DEVICE_CMD          "c_exec_device_cmd"

/*
 * args: (uint8_t *devid, uint8_t *attr_name, uint8_t *attr_value)
 * @retval 0:success, other:failure
 */
#define C_FUNC_REPORT_DEVICE_ATTR       "c_report_device_attr"

/*
 * args: (uint8_t *devid, uint8_t *event_name, uint8_t *event_args)
 * @retval 0:success, other:failure
 */
#define C_FUNC_REPORT_DEVICE_EVENT      "c_report_device_event"



/*c函数中调用的lua函数名称定义*/
/*
 * args: (uint8_t *devid, uint8_t *attr_name, uint8_t *short_model)
 * @retval "0":success, "-1":failure
 */
#define LUA_FUNC_GET_CUSTOM_ATTR        "lua_get_custom_attr"


/*
 * args: (uint8_t *devid, uint8_t *attr_name, uint8_t *attr_value,
            uint8_t *short_model)
 * @retval "0":success, "-1":failure
 */
#define LUA_FUNC_SET_CUSTOM_ATTR        "lua_set_custom_attr"


/*
 * args: (uint8_t *devid, uint8_t *cmd_name, uint8_t *cmd_args,
            uint8_t *short_model)
 * @retval "0":success, "-1":failure
 */
#define LUA_FUNC_EXEC_CUSTOM_CMD        "lua_exec_custom_cmd"


/*
 * args: (uint8_t *devid, uint8_t *endpoint_id, uint8_t *event_name,
            uint8_t *event_args, uint8_t *short_model)
 * @retval "0":success, "-1":failure
 */
#define LUA_FUNC_REPORT_CUSTOM_EVENT    "lua_report_custom_event"


/*
 * args: (uint8_t *devid, uint8_t *endpoint_id, uint8_t *attr_name,
            uint8_t *attr_value, uint8_t *short_model)
 * @retval "0":success, "-1":failure
 */
#define LUA_FUNC_REPORT_CUSTOM_ATTR     "lua_report_custom_attr"

int stdd_lua_get_global_variable(void *lua_fd, const char *global_name,
                                 char **result);
int stdd_lua_get_global_function(void *lua_fd, const char *function_name);


/*
 *arg类型全部为char *
*/
int stdd_lua_call_function(void *lua_fd, const char *function_name,
                           char *result[], int result_num, int arg_num, ...);
int stdd_lua_load_file(void *lua_fd, const char *file_path);
void *stdd_lua_open();
void stdd_lua_close(void *lua_fd);
int stdd_lua_load_profile_context(void **lua_fd, uint32_t model_id);

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif

#endif


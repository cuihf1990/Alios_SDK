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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "list.h"
#include "mpool.h"
#include "stdd.h"
#include "stdd_lua.h"
#include "alink_export_internal.h"
#include "devmgr.h"

static int c_get_device_attr(lua_State* L);
static int c_set_device_attr(lua_State* L);
static int c_report_device_attr(lua_State* L);
static int c_exec_device_cmd(lua_State* L);
static int c_report_device_attr(lua_State* L);
static int c_report_device_event(lua_State* L);

/*zigbee协议栈lib*/
luaL_Reg zigbee_lib[] = {
    {C_FUNC_GET_DEVICE_ATTR, c_get_device_attr},
    {C_FUNC_SET_DEVICE_ATTR, c_set_device_attr},
    {C_FUNC_EXEC_DEVICE_CMD, c_exec_device_cmd},
    {C_FUNC_REPORT_DEVICE_ATTR, c_report_device_attr},
    {C_FUNC_REPORT_DEVICE_EVENT, c_report_device_event},
    {NULL, NULL}
};

int stdd_load_zigbee_lib(lua_State *lua_fd)
{
  luaL_newlib(lua_fd, zigbee_lib);
  return 1;
}


static int __get_device_attr(unsigned char ieee_addr[8], unsigned char endpoint_id,
        const char *attr_name)
{
    void *func = alink_cb_func[_ALINK_ZIGBEE_GET_DEVICE_STATUS];
    OS_ASSERT(func, "get_device_status callback is NULL");

    return ((zigbee_get_attr_cb_t)func)(ieee_addr, endpoint_id, attr_name);
}


static int __set_device_attr(unsigned char ieee_addr[8], unsigned char endpoint_id,
        const char *attr_name, const char *attr_value)
{
    void *func = alink_cb_func[_ALINK_ZIGBEE_SET_DEVICE_STATUS];
    OS_ASSERT(func, "set_device_status callback is NULL");

    return ((zigbee_set_attr_cb_t)func)(ieee_addr, endpoint_id, attr_name, attr_value);
}


static int __exec_device_cmd(unsigned char ieee_addr[8],
        unsigned char endpoint_id, const char *cmd_name, const char *cmd_args)
{
    void *func = alink_cb_func[_ALINK_ZIGBEE_EXECUTE_DEVICE_CMD];
    OS_ASSERT(func, "exec_device_cmd callback is NULL");

    return ((zigbee_exec_cmd_cb_t)func)(ieee_addr, endpoint_id, cmd_name, cmd_args);
}


/*args: char *dev_id,int endpoint_id,char *attr_name*/
static int c_get_device_attr(lua_State* L)
{
    int ret = SERVICE_RESULT_ERR;
    dev_info_t *devinfo = NULL;

    const char *dev_id = luaL_checkstring(L,1);
    int endpoint_id = luaL_checkinteger(L,2);
    const char *attr_name = luaL_checkstring(L,3);

    //1.获取ieee_address,model_id
    devinfo = devmgr_get_devinfo(dev_id);
    PTR_GOTO(devinfo, end, "get device info fail, devid:%s", dev_id);

    ret = __get_device_attr(devinfo->dev_base.u.ieee_addr, (unsigned char)endpoint_id, attr_name);
    RET_GOTO(ret, end, "get device attr fail, attr_name:%s", attr_name);

end:
    if(devinfo)
        devmgr_put_devinfo_ref(devinfo);

    lua_pushinteger(L, ret);
    return 1;
}


/*args: char *dev_id,int endpoint_id,char *attr_name,char *attr_value*/
static int c_set_device_attr(lua_State* L)
{
    int ret = SERVICE_RESULT_ERR;
    dev_info_t *devinfo = NULL;

    const char *dev_id = luaL_checkstring(L,1);
    int endpoint_id = luaL_checkinteger(L,2);
    const char *attr_name = luaL_checkstring(L,3);
    const char *attr_value = luaL_checkstring(L,4);

    log_trace("set attr: dev_id:%s, endpoint_id:%d, attr_name:%s, attr_value:%s", dev_id, endpoint_id,
            attr_name, attr_value);

    //1.获取ieee_address,model_id
    devinfo = devmgr_get_devinfo(dev_id);
    PTR_GOTO(devinfo, end, "get device info fail, devid:%s", dev_id);

    ret = __set_device_attr(devinfo->dev_base.u.ieee_addr, (unsigned char)endpoint_id, attr_name, attr_value);
    RET_GOTO(ret, end, "set device attr fail, attr_name:%s, attr_value:%s", attr_name, attr_value);

end:
    if(devinfo)
        devmgr_put_devinfo_ref(devinfo);

    lua_pushinteger(L, ret);
    return 1;
}


/*args: char *dev_id,char *attr_name,char *attr_value*/
static int c_report_device_attr(lua_State* L)
{
    int ret = SERVICE_RESULT_ERR;
    dev_info_t *devinfo = NULL;

    const char *dev_id = luaL_checkstring(L,1);
    const char *attr_name = luaL_checkstring(L,2);
    const char *attr_value = luaL_checkstring(L,3);

    log_trace("report attr: dev_id:%s, attr_name:%s, attr_value:%s", dev_id,
            attr_name, attr_value);

    //1.获取ieee_address,model_id
    devinfo = devmgr_get_devinfo(dev_id);
    PTR_GOTO(devinfo, end, "get device info fail, devid:%s", dev_id);

    ret = stdd_forward_device_status(devinfo, attr_name, attr_value);
    RET_LOG(ret, "forward device attr fail, attr_name:%s, attr_value", attr_name, attr_value);

end:
    if(devinfo)
        devmgr_put_devinfo_ref(devinfo);

    lua_pushinteger(L, ret);
    return 1;
}

/*args: char *dev_id,char *event_name,char *event_value*/
static int c_report_device_event(lua_State* L)
{
    int ret = SERVICE_RESULT_ERR;
    dev_info_t *devinfo = NULL;

    const char *dev_id = luaL_checkstring(L,1);
    const char *event_name = luaL_checkstring(L,2);
    const char *event_value = luaL_checkstring(L,3);

    log_trace("report event: dev_id:%s, event_name:%s, event_value:%s", dev_id,
            event_name, event_value);

    //1.获取ieee_address,model_id
    devinfo = devmgr_get_devinfo(dev_id);
    PTR_GOTO(devinfo, end, "get device info fail, devid:%s", dev_id);

    ret = stdd_forward_device_event(devinfo, event_name, event_value);
    RET_LOG(ret, "forward device event fail, attr_name:%s, attr_value", event_name, event_value);

end:
    if(devinfo)
        devmgr_put_devinfo_ref(devinfo);

    lua_pushinteger(L, ret);
    return 1;
}


/*args: char *dev_id,int endpoint_id,char *cmd_name,char *cmd_args*/
static int c_exec_device_cmd(lua_State* L)
{
    int ret = SERVICE_RESULT_ERR;
    dev_info_t *devinfo = NULL;

    const char *dev_id = luaL_checkstring(L,1);
    int endpoint_id = luaL_checkinteger(L,2);
    const char *cmd_name = luaL_checkstring(L,3);
    const char *cmd_args = luaL_checkstring(L,4);

    log_trace("exec cmd: dev_id:%s, endpoint_id:%d, cmd_name:%s, cmd_args:%s", dev_id, endpoint_id,
            cmd_name, cmd_args);

    //1.获取ieee_address,model_id
    devinfo = devmgr_get_devinfo(dev_id);
    PTR_GOTO(devinfo, end, "get device info fail, devid:%s", dev_id);

    ret = __exec_device_cmd(devinfo->dev_base.u.ieee_addr, (unsigned char)endpoint_id, cmd_name, cmd_args);
    RET_GOTO(ret, end, "excute device command fail, cmd_name:%s, cmd_args:%s", cmd_name, cmd_args);

end:
    if(devinfo)
        devmgr_put_devinfo_ref(devinfo);

    lua_pushinteger(L, ret);
    return 1;
}


int stdd_zbnet_report_attrs(unsigned char ieee_addr[8], unsigned char endpoint_id,
    const char *attr_name[], const char *attr_value[])
{
    int32_t ret = SERVICE_RESULT_ERR;
    dev_info_t *devinfo = NULL;
    char *result = NULL;
    const char *dest_value = NULL;
    int32_t i = 0;
    char *attr_set[32] = {NULL};
    int32_t array_size = 0;

    char addr[20] = {0};
    get_ieeeaddr_string_by_extaddr(ieee_addr, addr, sizeof(addr));
    log_trace("report attr, ieee addr:%s, endpoint id:0x%08x", addr, endpoint_id);
    int y = 0;
    while(attr_name[y]){
        log_trace("\tattr name:%s, value:%s", attr_name[y], attr_value[y]);
        y++;
    }

    //1.获取ieee_address,model_id
    devinfo = devmgr_get_devinfo_by_ieeeaddr(ieee_addr);
    PTR_GOTO(devinfo, out, CALL_FUCTION_FAILED, "devmgr_get_devinfo_by_ieeeaddr");
    __dump_devinfo(devinfo);

    /*
     *只要有数据上报就link状态切换为LINKED,并login到云端
    */
    if(devinfo->link_state == LINK_STATE_OFFLINE || devinfo->cloud_state != DEVICE_STATE_LOGINED)
        devmgr_relogin_device(devinfo->dev_base.dev_id);

    if(devinfo->cloud_state!=DEVICE_STATE_LOGINED)
        log_warn("The device %s link state %d, cloud state %d",
			devinfo->dev_base.uuid, devinfo->link_state, devinfo->cloud_state);

    //2.check & load device profile
    stdd_check_device_profile(DEV_TYPE_ZIGBEE, devinfo->dev_base.model_id);

    while(attr_name[i++]){
        //3'.report customized attribute
        ret = stdd_report_custom_attr(devinfo->dev_base.dev_id, devinfo->dev_base.model_id, endpoint_id,
                attr_name[i - 1], attr_value[i - 1]);
        //返回SERVICE_RESULT_OK表示属性上报成功，遍历下一个属性
        if(SERVICE_RESULT_OK == ret){
            log_trace("report custom attribute success, attr_name:%s, attr_value:%s",
                attr_name[i - 1], attr_value[i - 1]);
            continue;
        }

        //3.终端设备属性名称转化为云端属性名称
        array_size = sizeof(attr_set)/sizeof(attr_set[0]);
        ret = stdd_get_alink_attr_set(devinfo->dev_base.model_id, endpoint_id, attr_name[i - 1], attr_set, &array_size);
        if(ret != SERVICE_RESULT_OK || array_size == 0){
            log_warn("stdd_get_alink_attr_set fail, attr_name:%s", attr_name[i - 1]);
            array_size = 0;
            //continue;
        }

        //异常情况:映射的属性数大于1的在lua脚本中处理
        if(array_size > 1){
            log_warn("size of attribute mapping set:%d, attr_name:%s", array_size, attr_name);
            log_warn("mapping set:");
            int32_t attr_count = array_size;
            while(attr_count-- > 0){
                os_printf("%s,", attr_set[attr_count]);
            }
        }

        //4.转发设备状态到cloud,lmns,ifttt和devmgr模块
        ret = stdd_forward_device_status(devinfo, array_size > 0?attr_set[0]:attr_name[i - 1], attr_value[i - 1]);
        RET_LOG(ret, "forward device attr fail, attr_name:%s, attr_value",
            array_size > 0?attr_set[0]:attr_name[i - 1], attr_value[i - 1]);

        //释放buff，初始化attr_set
        while(array_size-- > 0){
            if(attr_set[array_size] != NULL){
                stdd_free_buff(attr_set[array_size]);
                attr_set[array_size] = NULL;
            }
        }
    }
out:
    if(devinfo)
        devmgr_put_devinfo_ref(devinfo);

    return ret;
}


int stdd_zbnet_get_attr(const char *devid_or_uuid, const char *attr_name)
{
    uint8_t endpoint_id = 1;
    int32_t ret = SERVICE_RESULT_ERR;
    dev_info_t *devinfo = NULL;
    char *attr_set[32] = {NULL};
    int32_t array_size = sizeof(attr_set)/sizeof(attr_set[0]);

    //1.获取ieee_address,model_id
    devinfo = devmgr_get_devinfo(devid_or_uuid);
    PTR_GOTO(devinfo, out, CALL_FUCTION_FAILED, "devmgr_get_devinfo");

    //2.check & load device profile
    stdd_check_device_profile(DEV_TYPE_ZIGBEE, devinfo->dev_base.model_id);

    //3'.get customized attribute
    ret = stdd_get_custom_attr(devid_or_uuid, devinfo->dev_base.model_id, attr_name);
    if(SERVICE_RESULT_OK == ret){
        log_trace("get custom attribute success, attr_name:%s", attr_name);
        goto out;
    }

    //3.云端属性名称转化为终端设备属性名称
    ret = stdd_get_attr_mapping_set(devinfo->dev_base.model_id, attr_name, &endpoint_id, attr_set, &array_size);
    if(ret != SERVICE_RESULT_OK || array_size == 0){
        log_warn("stdd_get_attr_mapping_set fail, attr_name:%s", attr_name);
        array_size = 0;
    }

    //异常情况:映射的属性数大于1的在lua脚本中处理
    if(array_size > 1){
        log_warn("size of attribute mapping set:%d, attr_name:%s", array_size, attr_name);
        log_warn("mapping set:");
        int32_t attr_count = array_size;
        while(attr_count-- > 0){
            os_printf("%s,", attr_set[attr_count]);
        }
    }

    //4.调用协议栈接口获取第一个映射的属性
    ret = __get_device_attr(devinfo->dev_base.u.ieee_addr, endpoint_id, array_size > 0?attr_set[0]:attr_name);
    RET_GOTO(ret, out, "get device attr fail, attr_name:%s", array_size > 0?attr_set[0]:attr_name);

out:
    if(devinfo)
        devmgr_put_devinfo_ref(devinfo);
    while(array_size-- > 0){
        if(attr_set[array_size])
            stdd_free_buff(attr_set[array_size]);
    }

    return ret;
}


int stdd_zbnet_set_attr(const char *devid_or_uuid, const char *attr_name, const char *attr_value)
{
    uint8_t endpoint_id = 1;
    int32_t ret = SERVICE_RESULT_ERR;
    dev_info_t *devinfo = NULL;
    char *attr_set[32] = {NULL};
    int32_t array_size = sizeof(attr_set)/sizeof(attr_set[0]);

    //1.获取ieee_address,model_id
    devinfo = devmgr_get_devinfo(devid_or_uuid);
    PTR_GOTO(devinfo, out, CALL_FUCTION_FAILED, "devmgr_get_devinfo");

    //2.check & load device profile
    stdd_check_device_profile(DEV_TYPE_ZIGBEE, devinfo->dev_base.model_id);

    //3'.set customized attribute
    ret = stdd_set_custom_attr(devid_or_uuid, devinfo->dev_base.model_id, attr_name, attr_value);
    if(SERVICE_RESULT_OK == ret){
        log_trace("set custom attribute success, attr_name:%s, attr_value:%s", attr_name, attr_value);
        goto out;
    }

    //3.云端属性名称转化为终端设备属性名称
    ret = stdd_get_attr_mapping_set(devinfo->dev_base.model_id, attr_name, &endpoint_id, attr_set, &array_size);
    if(ret != SERVICE_RESULT_OK || array_size == 0){
        log_warn("stdd_get_attr_mapping_set fail, attr_name:%s", attr_name);
        array_size = 0;
    }

    //异常情况:映射的属性数大于1的在lua脚本中处理
    if(array_size > 1){
        log_warn("size of attribute mapping set:%d, attr_name:%s, mapping set:", array_size, attr_name);

        log_warn("mapping set:");
        int32_t attr_count = array_size;
        while(attr_count-- > 0){
            os_printf("%s,", attr_set[attr_count]);
        }
    }

    //4.调用协议栈接口设置第一个映射的属性，
    ret = __set_device_attr(devinfo->dev_base.u.ieee_addr, endpoint_id,
        array_size > 0?attr_set[0]:attr_name, attr_value);
    RET_GOTO(ret, out, "set device attr fail, attr_name:%s, attr_value:%s",
        array_size > 0?attr_set[0]:attr_name, attr_value);

out:
    if(devinfo)
        devmgr_put_devinfo_ref(devinfo);
    while(array_size-- > 0){
        if(attr_set[array_size])
            stdd_free_buff(attr_set[array_size]);
    }

    return ret;
}


int stdd_zbnet_exec_rpc(const char *devid_or_uuid, const char *rpc_name, const char *rpc_args)
{
    uint8_t endpoint_id = 1;
    char cmd_name[32] = {0};
    int32_t ret = SERVICE_RESULT_ERR;
    dev_info_t *devinfo = NULL;

    //1.获取ieee_address,model_id
    devinfo = devmgr_get_devinfo(devid_or_uuid);
    PTR_GOTO(devinfo, out, CALL_FUCTION_FAILED, "devmgr_get_devinfo");

    //2.check & load device profile
    stdd_check_device_profile(DEV_TYPE_ZIGBEE, devinfo->dev_base.model_id);

    //3'execute customized command
    ret = stdd_exec_custom_cmd(devid_or_uuid, devinfo->dev_base.model_id, rpc_name, rpc_args);
    if(SERVICE_RESULT_OK == ret){
        log_trace("exec custom command success, cmd_name:%s, cmd_args", rpc_name, rpc_args);
        goto out;
    }

    //3.用户rpc名称转zigbee命令名称
    ret = stdd_get_cmd_mapping_name(devinfo->dev_base.model_id, rpc_name, &endpoint_id,
        cmd_name, sizeof(cmd_name));
    if(ret != SERVICE_RESULT_OK)
        log_warn("stdd_get_cmd_mapping_name fail, rpc_name:%s, rpc_args:%s", rpc_name, rpc_args);

    //4.调用协议栈接口发送消息
    ret = __exec_device_cmd(devinfo->dev_base.u.ieee_addr, endpoint_id,
        strlen(cmd_name)!=0?cmd_name:rpc_name, rpc_args);
    RET_GOTO(ret, out, "excute device command fail, cmd_name:%s, cmd_args:%s",
        strlen(cmd_name)!=0?cmd_name:rpc_name, rpc_args);
out:
    if(devinfo)
        devmgr_put_devinfo_ref(devinfo);

    return ret;
}


/*
 * device -> coo evemt
*/
int stdd_zbnet_report_event(unsigned char ieee_addr[8], unsigned char endpoint_id,
        const char *event_name, const char *event_args)
{
    char cloud_attr[32] = {0};
    int32_t ret = SERVICE_RESULT_ERR;
    dev_info_t *devinfo = NULL;
    char *result = NULL;

    char addr[20] = {0};
    get_ieeeaddr_string_by_extaddr(ieee_addr, addr, sizeof(addr));
    log_trace("report attr, ieee addr:%s, endpoint id:0x%08x", addr, endpoint_id);

    //1.获取ieee_address,model_id
    devinfo = devmgr_get_devinfo_by_ieeeaddr(ieee_addr);
    PTR_GOTO(devinfo, out, CALL_FUCTION_FAILED, "devmgr_get_devinfo_by_ieeeaddr");
    __dump_devinfo(devinfo);

    //check logined
    if(devinfo->cloud_state != DEVICE_STATE_LOGINED)
        devmgr_relogin_device(devinfo->dev_base.dev_id);

    //2.check & load device profile
    stdd_check_device_profile(DEV_TYPE_ZIGBEE, devinfo->dev_base.model_id);

    //3.report customized event
    ret = stdd_report_custom_event(devinfo->dev_base.dev_id, devinfo->dev_base.model_id, endpoint_id,
            event_name, event_args);
    RET_GOTO(ret, out, "report custom event fail, cmd_name:%s, cmd_args:%s", event_name, event_args);

    log_trace("report custom event, cmd_name:%s, cmd_args:%s", event_name, event_args);

out:
    if(devinfo)
        devmgr_put_devinfo_ref(devinfo);

    return ret;
}



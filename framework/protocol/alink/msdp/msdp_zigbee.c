#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "list.h"
#include "msdp.h"
#include "mpool.h"
#include "stdd.h"
#include "json_parser.h"
#include "msdp_common.h"
#include "stdd_zigbee.h"
#include "service.h"
#include "devmgr_cache.h"

static int __get_attribute(const char *uuid, const char *attr_name, char **attr_value)
{
    int ret = SERVICE_RESULT_ERR;
    char *attr_cache = NULL;

    log_trace("uuid:%s, attr_name:%s", uuid, attr_name);
    ret = devmgr_read_attr_cache(uuid, attr_name, &attr_cache);
    if(SERVICE_RESULT_OK != ret){
        log_warn("read attribute cache fail, attr_name:%s", attr_name);
    }
    if(SERVICE_RESULT_OK == ret && NULL != attr_cache){
        *attr_value = msdp_dup_string(attr_cache);
        os_free(attr_cache);
        return SERVICE_RESULT_OK;
    }

    ret = stdd_zbnet_get_attr(uuid, attr_name);
    RET_LOG(ret, "get attribute fail, attrname:%s", attr_name);

    return ret;
}


static int __set_attribute(const char *uuid, const char *attr_name, char *attr_value)
{
    int ret = SERVICE_RESULT_ERR;

    ret = stdd_zbnet_set_attr(uuid, attr_name, attr_value);
    RET_RETURN(ret, CALL_FUCTION_FAILED, "stdd_zbnet_set_attr");

    return ret;
}



/*获取指定设备所有属性*/
static int __get_all_attrname(const char *uuid, char *attr_set, int buff_size)
{
    int count, ret = SERVICE_RESULT_ERR;
    ret = stdd_get_device_attrset(uuid, attr_set, buff_size);
    RET_RETURN(ret, "get device all attribute name fail, uuid:%s", uuid);

    return ret;
}


/*
--> {"uuid":"42E6E69DAEF6483FBBA412A28AF7CD76","attrSet":["wlanSwitchState", "lightSwitchState"]}
<-- {"uuid":"42E6E69DAEF6483FBBA412A28AF7CD76","attrSet":["wlanSwitchState"],"wlanSwitchState":{"value":"1","when":"1404443369"}}
*/
static int msdp_get_status(char *params)
{
    int ret = SERVICE_RESULT_ERR;
    char *str_pos, *params_ptr = params;
    int str_len = 0;
    char params_buff[512] = {0};

    int attrset_size = 0;
    str_pos = json_get_value_by_name(params, strlen(params), JSON_KEY_ATTRSET, &str_len, NULL);
    if(str_pos)
    {
        log_trace("attrset:%s", str_pos);
        attrset_size = json_get_array_size(str_pos, str_len);
    }

    if(NULL == str_pos || attrset_size == 0){
        //uuid
        char uuid[MAX_UUID_LEN] = {0};
        str_pos = json_get_value_by_name(params, strlen(params), JSON_KEY_UUID, &str_len, NULL);
        PTR_RETURN(str_pos, SERVICE_RESULT_ERR, "get uuid fail, params:%s", params);
        strncpy(uuid, str_pos, str_len);

        //else,todo:get devid

        char all_attr[256] = {0};
        ret = __get_all_attrname(uuid, all_attr, sizeof(all_attr));
        RET_RETURN(ret, "__get_all_attrname(%s)", uuid);
        log_trace("attrset:%s", all_attr);

        snprintf(params_buff, sizeof(params_buff) - 1, GET_DEVICE_ATTR_STRING_FMT, uuid, all_attr);
        params_ptr = params_buff;
    }

    log_trace("params_ptr:%s", params_ptr);
    char *json_out = NULL;
    ret = msdp_get_device_status_handler(params_ptr, msdp_get_attr_each_cb, __get_attribute, &json_out);
    RET_RETURN(ret, CALL_FUCTION_FAILED, "msdp_status_handler");
    if(json_out)
    {
        log_trace("post device data: params:%s", json_out);
        if (SERVICE_RESULT_OK != msdp_add_asyncpost_task(json_out)){
            log_error("add async post task fail, params:%s", json_out);
            msdp_free_buff(json_out);
        }
        json_out = NULL;
    }

    return ret;
}


/*
--> {"uuid":"42E6E69DAEF6483FBBA412A28AF7CD76","attrSet":["wlanSwitchState"],"wlanSwitchState":{"value":"1","when":"1404443369"}}
*/
static int msdp_set_status(char *params)
{
    int ret = SERVICE_RESULT_ERR;
    //log_trace("params:%s\n", params);

    //遍历attrset，设置属性
    ret = msdp_set_device_status_handler(params, msdp_set_attr_each_cb, __set_attribute);
    RET_RETURN(ret, CALL_FUCTION_FAILED, "msdp_set_attr_each_cb");

    return ret;
}



/*
--> {"uuid":"42E6E69DAEF6483FBBA412A28AF7CD76","service\":"startBwCheck","args":{}}
<-- {"uuid": "42E6E69DAEF6483FBBA412A28AF7CD76", "service": "stopBwCheck", "code": "1", "result": [ ] }
*/
static int msdp_rpc(char *params)
{
    int ret = SERVICE_RESULT_ERR;
    char rpc_name[MAX_SERVICE_NAME_LEN] = {0};
    char uuid[MAX_UUID_LEN] = {0};
    char rpc_args[256] = {0};
    char *str_pos = NULL;
    int str_len = 0;

    log_trace("params:%s", params);

    str_pos = json_get_value_by_name(params, strlen(params), JSON_KEY_SERVICE, &str_len, NULL);
    PTR_RETURN(str_pos, ret, "get service name fail, params:%s", params);
    strncpy(rpc_name, str_pos, str_len);

    str_pos = json_get_value_by_name(params, strlen(params), JSON_KEY_ARGS, &str_len, NULL);
    PTR_RETURN(str_pos, ret, "get service args fail, params:%s", params);
    strncpy(rpc_args, str_pos, str_len);

    str_pos = json_get_value_by_name(params, strlen(params), JSON_KEY_UUID, &str_len, NULL);
    PTR_RETURN(str_pos, ret, "get uuid fail, params:%s", params);
    strncpy(uuid, str_pos, str_len);

    ret = stdd_zbnet_exec_rpc(uuid, rpc_name, rpc_args);
    RET_RETURN(ret, "exeute rpc fail, uuid:%s, rpc_name:%s, args:%s", uuid, rpc_name, rpc_args);

    return ret;
}


int msdp_zbnet_init()
{
    dev_option_cb callback[MAX_CALLBACK_NUM] = {NULL};

    /*默认cmd callback*/
    callback[ACB_GET_DEVICE_STATUS] = msdp_get_status;
    callback[ACB_SET_DEVICE_STATUS] = msdp_set_status;
    callback[ACB_REQUEST_REMOTE_SERVICE] = msdp_rpc;

    /*注册zigbee设备事件处理接口*/
    return msdp_register_device_helper(DEV_TYPE_ZIGBEE, callback);
}


void mdsp_zbnet_exit()
{
    msdp_unregister_device_helper(DEV_TYPE_ZIGBEE);
}



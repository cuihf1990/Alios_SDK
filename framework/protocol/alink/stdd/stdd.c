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
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "mpool.h"
#include "log.h"
#include "alink_export_internal.h"
#include "alink_protocol.h"
#include "stdd_parser.h"
#include "stdd_lua.h"
#include "stdd.h"
#include "stdd_lua.h"
#include "stdd_datatype.h"
#include "devmgr_cache.h"
#include "msdp.h"
#include "attrs_profile_mgr.h"
#include "ifttt_trigger_manager.h"


#define MAX_PROFILE_SIZE_ONCE_UPDATE    10
#define MAX_MODEL_NUM       128
#define NUMBER_STR_LEN      16

static mpool_t *stdd_mpool = NULL;
static void *g_L = NULL;

static LIST_HEAD(attr_profile_head);
static LIST_HEAD(cmd_profile_head);
static LIST_HEAD(device_profile_head);


static dev_profile_t *__get_device_profile(uint32_t model_id)
{
    dev_profile_t *profile = NULL;

    list_for_each_entry_t(profile, &device_profile_head, list_node, dev_profile_t) {
        if (profile->model_id == model_id) {
            return profile;
        }
    }

    return NULL;
}


static int32_t __update_attr_profile(char *profile_str)
{
    int ret = SERVICE_RESULT_ERR;
    attr_profile_t *profile[MAX_PROFILE_SIZE_ONCE_UPDATE + 1] = {NULL};
    attr_profile_t *attr, *n_attr;
    LIST_HEAD(new_list);
    LIST_HEAD(delete_list);
    data_list_t *pos, *next, *n_pos, *n_next;

    ret = stdd_parse_attr_profile(profile_str, &new_list);
    RET_RETURN(ret, "parser attr profile fail, profile:%s", profile_str);

    if (!alink_cb_func[_ALINK_ZIGBEE_UPDATE_ATTR_PROFILE]) {
        log_warn("unregistered update_attr_profile callback");

        /*更新到新的全局链表*/
        list_for_each_entry_safe_t(n_pos, n_next, &new_list, list_node, data_list_t) {
            list_move(&n_pos->list_node, &attr_profile_head);
        }
        return SERVICE_RESULT_ERR;
    }

    int array_size, i = 0;
    array_size = sizeof(profile) / sizeof(profile[0]);

    list_for_each_entry_safe_t(n_pos, n_next, &new_list, list_node, data_list_t) {
        n_attr = (attr_profile_t *)n_pos->data;
        list_for_each_entry_safe_t(pos, next, &attr_profile_head, list_node,
                                   data_list_t) {
            attr = (attr_profile_t *)pos->data;
            if (attr->cluster_id == n_attr->cluster_id &&
                attr->attribute_id == n_attr->attribute_id) {
                log_warn("update attr profile, clusterid:0x%04x, attrid:0x%04x, old_attrname:%s, new_attrname:%s",
                         attr->cluster_id, attr->attribute_id, attr->attr_name, n_attr->attr_name);
                list_move(&pos->list_node, &delete_list);
            }
        }
        //纪录attr_profile指针
        profile[i++] = n_attr;
        if (i == array_size - 1) {
            log_info("update attr profile to zigbee stack, size:%d, name:%s", i,
                     n_attr->attr_name);
            //更新profile到zigbee模组端
            ret = ((zigbee_update_attr_profile_cb_t)
                   alink_cb_func[_ALINK_ZIGBEE_UPDATE_ATTR_PROFILE])(profile);
            RET_GOTO(ret, end, "update attr profile fail");
            memset(profile, 0, sizeof(profile));
            i = 0;
        }
    }

    if (profile[0]) {
        log_info("update attr profile to zigbee stack, size:%d", i);
        ret = ((zigbee_update_attr_profile_cb_t)
               alink_cb_func[_ALINK_ZIGBEE_UPDATE_ATTR_PROFILE])(profile);
        RET_GOTO(ret, end, "update attr profile fail");
    }

end:
    /*更新到新的全局链表*/
    list_for_each_entry_safe_t(n_pos, n_next, &new_list, list_node, data_list_t) {
        list_move(&n_pos->list_node, &attr_profile_head);
    }

    /*释放待删除链表节点*/
    stdd_free_attr_profile(&delete_list);

    return ret;
}


static int32_t __update_cmd_profile(char *profile_str)
{
    int ret = SERVICE_RESULT_ERR;
    cmd_profile_t *profile[MAX_PROFILE_SIZE_ONCE_UPDATE + 1] = {NULL};
    cmd_profile_t *cmd, *n_cmd;
    LIST_HEAD(new_list);
    LIST_HEAD(delete_list);
    data_list_t *pos, *next, *n_pos, *n_next;

    ret = stdd_parse_cmd_profile(profile_str, &new_list);
    RET_RETURN(ret, "parser cmd profile fail, profile:%s", profile_str);

    if (!alink_cb_func[_ALINK_ZIGBEE_UPDATE_CMD_PROFILE]) {
        log_warn("unregistered update_cmd_profile callback");
        /*更新到新的全局链表*/
        list_for_each_entry_safe_t(n_pos, n_next, &new_list, list_node, data_list_t) {
            list_move(&n_pos->list_node, &cmd_profile_head);
        }
        return SERVICE_RESULT_ERR;
    }

    int array_size, i = 0;
    array_size = sizeof(profile) / sizeof(profile[0]);

    list_for_each_entry_safe_t(n_pos, n_next, &new_list, list_node, data_list_t) {
        n_cmd = (cmd_profile_t *)n_pos->data;
        list_for_each_entry_safe_t(pos, next, &cmd_profile_head, list_node,
                                   data_list_t) {
            cmd = (cmd_profile_t *)pos->data;
            if (cmd->cluster_id == n_cmd->cluster_id &&
                cmd->cmd_id == n_cmd->cmd_id) {
                log_warn("update cmd profile, clusterid:0x%04x, cmdid:0x%02x, old_cmdname:%s, new_cmdname:%s",
                         cmd->cluster_id, cmd->cmd_id, cmd->cmd_name, n_cmd->cmd_name);
                list_move(&pos->list_node, &delete_list);
            }
        }

        //纪录cmd_profile指针
        profile[i++] = n_cmd;
        if (i == array_size - 1) {
            log_info("update cmd profile to zigbee stack, size:%d", i);
            //更新profile到zigbee模组端
            ret = ((zigbee_update_cmd_profile_cb_t)
                   alink_cb_func[_ALINK_ZIGBEE_UPDATE_CMD_PROFILE])(profile);
            RET_GOTO(ret, end, "update cmd profile fail");
            memset(profile, 0, sizeof(profile));
            i = 0;
        }
    }

    if (profile[0]) {
        log_info("update cmd profile to zigbee stack, size:%d", i);
        //更新profile到zigbee模组端
        ret = ((zigbee_update_cmd_profile_cb_t)
               alink_cb_func[_ALINK_ZIGBEE_UPDATE_CMD_PROFILE])(profile);
        RET_GOTO(ret, end, "update cmd profile fail");
    }

end:
    /*更新到新的全局链表*/
    list_for_each_entry_safe_t(n_pos, n_next, &new_list, list_node, data_list_t) {
        list_move(&n_pos->list_node, &cmd_profile_head);
    }

    /*释放待删除链表节点*/
    stdd_free_cmd_profile(&delete_list);

    return ret;
}



static dev_profile_t *__stdd_load_device_profile(const char *profile_path,
                                                 uint32_t model_id)
{
    int ret = SERVICE_RESULT_ERR;
    char *result = NULL;
    LIST_HEAD(profile_head);
    dev_profile_t *dev_profile = NULL;

    if (NULL == g_L) {
        g_L = stdd_lua_open();
        PTR_RETURN(g_L, NULL, "lua open fail");
    }

    ret = stdd_lua_load_file(g_L, profile_path);
    RET_GOTO(ret, err, "lua load file fail, file:%s", profile_path);

    dev_profile = stdd_new_buff(sizeof(dev_profile_t));
    PTR_RETURN(dev_profile, NULL, "stdd_new_buff fail");
    memset(dev_profile, 0, sizeof(dev_profile_t));
    INIT_LIST_HEAD(&dev_profile->endpoint_head);
    dev_profile->model_id = model_id;

    /*custom attributes and commands*/
#if 0
    //private attr profile
    ret = stdd_lua_get_global_variable(g_L, LUA_PRIVATE_ATTR_PROFILE, &result);
    if (ret == SERVICE_RESULT_OK &&
        result != NULL && strlen(result) > 0) {
        ret = __update_attr_profile(result);
        RET_LOG(ret, "update attr profile fail, profile:%s", result);
    }

    result = NULL;
    //private cmd profile
    ret = stdd_lua_get_global_variable(g_L, LUA_PRIVATE_CMD_PROFILE, &result);
    if (ret == SERVICE_RESULT_OK &&
        result != NULL && strlen(result) > 0) {
        ret = __update_cmd_profile(result);
        RET_LOG(ret, "update cmd profile fail, profile:%s", result);
    }
#endif
    //private attr&cmd name profile
    result = NULL;
    ret = stdd_lua_get_global_variable(g_L, LUA_ENDPOINT_PROFILE, &result);
    if (ret == SERVICE_RESULT_OK &&
        result != NULL && strlen(result) > 0) {
        ret = stdd_parse_endpoint_profile(result, &dev_profile->endpoint_head);
        RET_LOG(ret, "parse endpoint profile fail, profile:%s", result);
    }

    //attr set
    result = NULL;
    ret = stdd_lua_get_global_variable(g_L, LUA_DEVICE_ATTR_SET, &result);
    RET_LOG(ret, "get lua global variable fail, name:%s", LUA_DEVICE_ATTR_SET);
    if (ret == SERVICE_RESULT_OK &&
        result != NULL && strlen(result)) {
        dev_profile->attr_set = stdd_dup_string(result ? result : "[]");
    }

    //cmd set
    result = NULL;
    ret = stdd_lua_get_global_variable(g_L, LUA_DEVICE_CMD_SET, &result);
    RET_LOG(ret, "get lua global variable fail, name:%s", LUA_DEVICE_CMD_SET);
    dev_profile->cmd_set = stdd_dup_string(result ? result : "[]");

    //profile file
    dev_profile->profile_name = stdd_dup_string(profile_path);

    list_add_tail(&dev_profile->list_node, &device_profile_head);

    return dev_profile;
err:
    if (dev_profile) {
        stdd_free_device_profile(dev_profile);
    }

    return NULL;
}

void *stdd_dup_string(const char *src)
{
    return pstrdup(stdd_mpool, src);
}

void *stdd_new_buff(unsigned int buff_size)
{
    void *buff = pmalloc(stdd_mpool, buff_size);
    if (NULL != buff) {
        memset(buff, 0, buff_size);
    }

    return buff;
}

void stdd_free_buff(void *buff)
{
    if (buff) {
        pfree(stdd_mpool, buff);
    }
}


int32_t stdd_update_global_profile(const char *profile_path)
{
    int ret = SERVICE_RESULT_ERR;
    char *result = NULL;

    log_trace("update global profile:%s", profile_path);

    if (NULL == g_L) {
        g_L = stdd_lua_open();
        PTR_RETURN(g_L, ret, "lua open fail");
    }
    ret = stdd_lua_load_file(g_L, profile_path);
    RET_GOTO(ret, end, "lua load file fail, file:%s", profile_path);

    //global attr profile
    ret = stdd_lua_get_global_variable(g_L, LUA_GLOBAL_ATTR_PROFILE, &result);
    RET_GOTO(ret, end, "get lua global variable fail, name:%s, profile path:%s",
             LUA_GLOBAL_ATTR_PROFILE, profile_path);
    ret = __update_attr_profile(result);
    if (ret != SERVICE_RESULT_OK) {
        log_error("update global attr profile fail, profile path:%s", profile_path);
    }

    result = NULL;
    //global cmd profile
    ret = stdd_lua_get_global_variable(g_L, LUA_GLOBAL_CMD_PROFILE, &result);
    RET_GOTO(ret, end, "get lua global variable fail, name:%s, profile path:%s",
             LUA_GLOBAL_CMD_PROFILE, profile_path);
    ret = __update_cmd_profile(result);
    RET_GOTO(ret, end, "update global cmd profile fail, profile path:%s",
             profile_path);

end:
    //global profile比较大,用完关闭lua句柄
    stdd_lua_close(g_L);
    g_L = NULL;
    return ret;
}


int32_t stdd_add_device_profile(uint8_t dev_type, uint32_t model_id,
                                const char *profile_path)
{
    char *result = NULL;
    dev_profile_t *dev_profile = __get_device_profile(model_id);
    if (dev_profile) {
        log_info("device profile exist, model id:%0x%08x", (unsigned int)model_id);
        return SERVICE_RESULT_OK;
    }

    dev_profile = __stdd_load_device_profile(profile_path, model_id);
    PTR_RETURN(dev_profile, SERVICE_RESULT_ERR,
               "load device profile fail, profile path:%s, model id:%d", profile_path,
               model_id);

    return SERVICE_RESULT_OK;
}

int32_t stdd_update_device_profile(uint8_t dev_type, uint32_t model_id,
                                   const char *profile_path)
{
    char *result = NULL;

    log_trace("update device profile:%s", profile_path);
    dev_profile_t *dev_profile = __get_device_profile(model_id);
    if (dev_profile) {
        list_del(&dev_profile->list_node);
        stdd_free_device_profile(dev_profile);
    }

    dev_profile = __stdd_load_device_profile(profile_path, model_id);
    PTR_RETURN(dev_profile, SERVICE_RESULT_ERR,
               "load device profile fail, profile path:%s, model id:0x%08x", profile_path,
               model_id);
    //stdd_dump_device_profile();

    return SERVICE_RESULT_OK;
}


int stdd_get_device_profile_name(uint8_t dev_type, uint32_t model_id,
                                 char *file_name, int max_name_length)
{
    int ret = SERVICE_RESULT_ERR;

    /*优先从device profile链表中查询*/
    dev_profile_t *dev_profile = __get_device_profile(model_id);
    if (dev_profile && access(dev_profile->profile_name, F_OK) == 0) {
        if (strlen(file_name) > max_name_length - 1) {
            log_error("the buffer is too small of profile name");
            return ret;
        }
        strncpy(file_name, dev_profile->profile_name, max_name_length - 1);
        ret = SERVICE_RESULT_OK;
    } else {
        ret = get_device_profile_file(dev_type, model_id, file_name, max_name_length);
        if (SERVICE_RESULT_OK == ret && strlen(file_name) > 0) {
            stdd_update_device_profile(dev_type, model_id, file_name);
        } else {
            ret = SERVICE_RESULT_ERR;
        }
    }

    return ret;
}

int32_t stdd_check_device_profile(uint8_t dev_type, uint32_t model_id)
{
    int ret = SERVICE_RESULT_ERR;
    char profile_path[MAX_PROFILE_NAME_LEN] = {0};

    if (__get_device_profile(model_id)) {
        return SERVICE_RESULT_OK;
    }

    //load device profile
    ret = get_device_profile_file(dev_type, model_id, profile_path,
                                  sizeof(profile_path));
    if (SERVICE_RESULT_OK == ret && strlen(profile_path) > 0) {
        ret = stdd_add_device_profile(dev_type, model_id, profile_path);
        return ret;
    }

    return  SERVICE_RESULT_ERR;
}


/*alink attribute -> zigbee attribute/command*/
int32_t stdd_get_attr_mapping_set(uint32_t short_model, const char *alink_attr,
                                  uint8_t *endpoint_id,
                                  char *name_array[], int *array_size)
{
    int32_t tmp_id, attr_count = 0;
    if (*array_size < 1) {
        return SERVICE_RESULT_ERR;
    }

    memset(name_array, 0, sizeof(name_array[0]) * (*array_size));
    dev_profile_t *dev_profile = __get_device_profile(short_model);
    if (NULL == dev_profile) {
        log_error("get device profile fail,short model:0x%08x", short_model);
        return SERVICE_RESULT_ERR;
    }

    endpoint_profile_t *endpoint;
    list_for_each_entry_t(endpoint, &dev_profile->endpoint_head, list_node,
                          endpoint_profile_t) {
        name_mapping_t *mapping;
        tmp_id = endpoint->endpoint_id;
        list_for_each_entry_t(mapping, &endpoint->attr_head, list_node,
                              name_mapping_t) {
            if (strcmp(alink_attr, mapping->user_name) == 0 && attr_count < *array_size) {
                name_array[attr_count++] = stdd_dup_string(mapping->std_name);
            }
        }
        if (attr_count > 0) {
            *endpoint_id = tmp_id;
            break;
        }
    }

    *array_size = attr_count;
    log_trace("mapping attr count: %d", attr_count);

    if (*array_size < 1) {
        return SERVICE_RESULT_ERR;
    }
}

/*alink rpc -> zigbee command*/
int32_t stdd_get_cmd_mapping_name(uint32_t short_model, const char *cmd_name,
                                  uint8_t *endpoint_id,
                                  char *target_cmd_name, uint32_t max_cmd_len)
{
    dev_profile_t *dev_profile = __get_device_profile(short_model);
    if (NULL == dev_profile) {
        return SERVICE_RESULT_ERR;
    }

    endpoint_profile_t *endpoint;
    list_for_each_entry_t(endpoint, &dev_profile->endpoint_head, list_node,
                          endpoint_profile_t) {
        name_mapping_t *mapping;
        list_for_each_entry_t(mapping, &endpoint->cmd_head, list_node, name_mapping_t) {
            if (strcmp(cmd_name, mapping->user_name) == 0) {
                strncpy(target_cmd_name, mapping->std_name, max_cmd_len - 1);
                *endpoint_id = endpoint->endpoint_id;
                return SERVICE_RESULT_OK;
            }
        }
    }

    return SERVICE_RESULT_ERR;
}

/*zigbee attribute/command -> alink attribute*/
int32_t stdd_get_alink_attr_set(uint32_t short_model, uint16_t endpoint_id,
                                const char *cmdname_or_attrname,
                                char *name_array[], int *array_size)
{
    int32_t attr_count = 0;
    if (*array_size < 1) {
        return SERVICE_RESULT_ERR;
    }

    dev_profile_t *dev_profile = __get_device_profile(short_model);
    if (NULL == dev_profile) {
        log_error("get device profile fail, short model:0x%08x", short_model);
        return SERVICE_RESULT_ERR;
    }

    endpoint_profile_t *endpoint;
    list_for_each_entry_t(endpoint, &dev_profile->endpoint_head, list_node,
                          endpoint_profile_t) {
        if (endpoint->endpoint_id != endpoint_id) {
            continue;
        }
        name_mapping_t *mapping;
        list_for_each_entry_t(mapping, &endpoint->attr_head, list_node,
                              name_mapping_t) {
            if (strcmp(cmdname_or_attrname, mapping->std_name) == 0 &&
                attr_count < *array_size) {
                name_array[attr_count++] = stdd_dup_string(mapping->user_name);
            }
        }
    }

    *array_size = attr_count;
    log_trace("alink attr count: %d", attr_count);

    if (*array_size < 1) {
        return SERVICE_RESULT_ERR;
    }
}


int32_t stdd_get_custom_attr(const char *dev_id, uint32_t model_id,
                             const char *attr_name)
{
    int ret = SERVICE_RESULT_ERR;
    char *result[2] = {NULL};
    char shortmodel_str[NUMBER_STR_LEN] = {0};
    void *lua_fd = NULL;

    ret = stdd_lua_load_profile_context(&lua_fd, model_id);
    RET_GOTO(ret, end, "init lua context fail, model id:0x%08x", model_id);

    snprintf(shortmodel_str, sizeof(shortmodel_str) - 1, "%u", model_id);
    ret = stdd_lua_call_function(lua_fd, LUA_FUNC_GET_CUSTOM_ATTR, result,
                                 1, 3, dev_id, attr_name, shortmodel_str);
    RET_GOTO(ret, end, "call lua fuction \"%s\" fail", LUA_FUNC_GET_CUSTOM_ATTR);

    //result copy
    if (result[0] && strlen(result[0]) > 0) {
        ret = atoi(result[0]);
    } else {
        log_error("call lua function result is NULL, function:%s",
                  LUA_FUNC_GET_CUSTOM_ATTR);
        ret = SERVICE_RESULT_ERR;
    }

end:
    if (lua_fd) {
        stdd_lua_close(lua_fd);
    }
    return ret;
}


int32_t stdd_set_custom_attr(const char *dev_id, uint32_t model_id,
                             const char *attr_name, const char *attr_value)
{
    int ret = SERVICE_RESULT_ERR;
    char *result[2] = {NULL};
    char shortmodel_str[NUMBER_STR_LEN] = {0};
    void *lua_fd = NULL;

    ret = stdd_lua_load_profile_context(&lua_fd, model_id);
    RET_GOTO(ret, end, "init lua context fail, model id:0x%08x", model_id);

    snprintf(shortmodel_str, sizeof(shortmodel_str) - 1, "%u", model_id);
    ret = stdd_lua_call_function(lua_fd, LUA_FUNC_SET_CUSTOM_ATTR, result,
                                 1, 4, dev_id, attr_name, attr_value, shortmodel_str);
    RET_GOTO(ret, end, "call lua fuction \"%s\" fail", LUA_FUNC_SET_CUSTOM_ATTR);

    //result
    if (result[0] && strlen(result[0]) > 0) {
        ret = atoi(result[0]);
    } else {
        log_error("call lua function result is NULL, function:%s",
                  LUA_FUNC_SET_CUSTOM_ATTR);
        ret = SERVICE_RESULT_ERR;
    }

end:
    if (lua_fd) {
        stdd_lua_close(lua_fd);
    }

    return ret;
}


int32_t stdd_exec_custom_cmd(const char *dev_id, uint32_t model_id,
                             const char *cmd_name, const char *cmd_args)
{
    int ret = SERVICE_RESULT_ERR;
    char *result[2] = {NULL};
    char shortmodel_str[NUMBER_STR_LEN] = {0};
    void *lua_fd = NULL;

    ret = stdd_lua_load_profile_context(&lua_fd, model_id);
    RET_GOTO(ret, end, "init lua context fail, model id:0x%08x", model_id);

    snprintf(shortmodel_str, sizeof(shortmodel_str) - 1, "%u", model_id);
    ret = stdd_lua_call_function(lua_fd, LUA_FUNC_EXEC_CUSTOM_CMD, result,
                                 1, 4, dev_id, cmd_name, cmd_args, shortmodel_str);
    RET_GOTO(ret, end, "call lua fuction \"%s\" fail", LUA_FUNC_EXEC_CUSTOM_CMD);

    //result
    if (result[0] && strlen(result[0]) > 0) {
        ret = atoi(result[0]);
    } else {
        log_error("call lua function result is NULL, function:%s",
                  LUA_FUNC_EXEC_CUSTOM_CMD);
        ret = SERVICE_RESULT_ERR;
    }

end:
    if (lua_fd) {
        stdd_lua_close(lua_fd);
    }

    return ret;
}


int32_t stdd_report_custom_attr(const char *dev_id, uint32_t model_id,
                                unsigned char endpoint_id,
                                const char *attr_name, const char *attr_value)
{
    int ret = SERVICE_RESULT_ERR;
    char *result[2] = {NULL};
    char endpoint_str[NUMBER_STR_LEN] = {0};
    char shortmodel_str[NUMBER_STR_LEN] = {0};
    void *lua_fd = NULL;

    ret = stdd_lua_load_profile_context(&lua_fd, model_id);
    RET_GOTO(ret, end, "init lua context fail, model id:0x%08x", model_id);

    snprintf(endpoint_str, sizeof(endpoint_str) - 1, "%d", endpoint_id);
    snprintf(shortmodel_str, sizeof(shortmodel_str) - 1, "%u", model_id);
    ret = stdd_lua_call_function(lua_fd, LUA_FUNC_REPORT_CUSTOM_ATTR, result,
                                 1, 5, dev_id, endpoint_str, attr_name, attr_value, shortmodel_str);
    RET_GOTO(ret, end, "call lua fuction \"%s\" fail", LUA_FUNC_REPORT_CUSTOM_ATTR);

    //result
    if (result[0] && strlen(result[0]) > 0) {
        ret = atoi(result[0]);
    } else {
        log_error("call lua function result is NULL, function:%s",
                  LUA_FUNC_REPORT_CUSTOM_ATTR);
        ret = SERVICE_RESULT_ERR;
    }

end:
    if (lua_fd) {
        stdd_lua_close(lua_fd);
    }

    return ret;
}


int32_t stdd_report_custom_event(const char *dev_id, uint32_t model_id,
                                 unsigned char endpoint_id,
                                 const char *event_name, const char *event_args)
{
    int ret = SERVICE_RESULT_ERR;
    char *result[2] = {NULL};
    char endpoint_str[NUMBER_STR_LEN] = {0};
    char shortmodel_str[NUMBER_STR_LEN] = {0};
    void *lua_fd = NULL;

    ret = stdd_lua_load_profile_context(&lua_fd, model_id);
    RET_GOTO(ret, end, "init lua context fail, model id:0x%08x", model_id);

    snprintf(endpoint_str, sizeof(endpoint_str) - 1, "%d", endpoint_id);
    snprintf(shortmodel_str, sizeof(shortmodel_str) - 1, "%u", model_id);
    ret = stdd_lua_call_function(lua_fd, LUA_FUNC_REPORT_CUSTOM_EVENT, result,
                                 1, 5, dev_id, endpoint_str, event_name, event_args, shortmodel_str);
    RET_GOTO(ret, end, "call lua fuction \"%s\" fail",
             LUA_FUNC_REPORT_CUSTOM_EVENT);

    //result
    if (result[0] && strlen(result[0]) > 0) {
        ret = atoi(result[0]);
    } else {
        log_error("call lua function result is NULL, function:%s",
                  LUA_FUNC_REPORT_CUSTOM_EVENT);
        ret = SERVICE_RESULT_ERR;
    }

end:
    if (lua_fd) {
        stdd_lua_close(lua_fd);
    }

    return ret;
}


/*读取设备属性*/
int stdd_get_device_attr(const char *uuid, const char *attr_name,
                         char *attr_value_buff, int buff_size)
{
    int ret = SERVICE_RESULT_ERR;

    if (NULL == uuid) {
        log_error("Invalid args: uuid");
        return ret;
    }

    if (strlen(uuid) == 0) {
        uuid = config_get_main_uuid();
    }

    if (buff_size == 0) {
        return ret;
    }
    attr_value_buff[0] = '\0';

    /*网关本身*/
    if (strcmp(uuid, config_get_main_uuid()) == 0) {
        ret = msdp_get_gw_attr(uuid, attr_name, attr_value_buff, buff_size);
    } else {
        ret = devmgr_get_attr_cache(uuid, attr_name, attr_value_buff, buff_size);
    }
    RET_RETURN(ret, "get device attr fail, uuid:%s, attr_name:%s", uuid, attr_name);
    log_trace("get device attr, uuid:%s, attr_name:%s, attr_value:%s", uuid,
              attr_name, attr_value_buff);

    return ret;
}

int __push_device_status(dev_info_t *devinfo,
                         const char *attr_name, const char *attr_value)
{
    int32_t ret = SERVICE_RESULT_ERR;

    //1.post设备属性
    if (strlen(devinfo->dev_base.uuid) > 0 &&
        devinfo->dev_base.dev_type != DEV_TYPE_WIFI) {
        ret = msdp_report_device_status(devinfo->dev_base.uuid, attr_name, attr_value);
        RET_LOG(ret, CALL_FUCTION_FAILED, "msdp_report_device_status");

        //如果设备未login则先login再report
        if (ALINK_CODE_ERROR_SUBDEV_NOT_LOGIN == ret)
            if (SERVICE_RESULT_OK == devmgr_relogin_device(devinfo->dev_base.uuid)) {
                msdp_report_device_status(devinfo->dev_base.uuid, attr_name, attr_value);
            }
    }

    //2.通知本地链接模块
    //ret = lmns_report_device_status(devinfo->dev_base.devid, attr_name, attr_value);
    //RET_LOG(ret, CALL_FUCTION_FAILED, "lmns_report_device_status");


    //4.更新属性缓存
    ret = devmgr_update_attr_cache(devinfo->dev_base.dev_id, attr_name, attr_value);
    RET_LOG(ret, CALL_FUCTION_FAILED, "devmgr_update_attr_cache");

    return ret;
}


int stdd_forward_device_status(dev_info_t *devinfo,
                               const char *attr_name, const char *attr_value)
{
    int32_t ret = SERVICE_RESULT_ERR;

    ret = __push_device_status(devinfo, attr_name, attr_value);
    RET_LOG(ret, CALL_FUCTION_FAILED, "__push_device_status");

    //3.设备状态同步给ifttt模块
    if (strlen(devinfo->dev_base.uuid) > 0) {
        ifttt_push_dev_data(devinfo->dev_base.uuid, attr_name, attr_value);
    }

    return ret;
}


int stdd_forward_device_event(dev_info_t *devinfo,
                              const char *event_name, const char *event_value)
{
    int32_t ret = SERVICE_RESULT_ERR;

    ret = __push_device_status(devinfo, event_name, event_value);
    RET_LOG(ret, CALL_FUCTION_FAILED, "__push_device_status");

    //3.设备事件通知ifttt模块
    //if(strlen(devinfo->dev_base.uuid) > 0){
    //    ifttt_push_dev_event(devinfo->dev_base.uuid, attr_name, attr_value);
    //}

    return ret;
}


int stdd_get_device_attrset(const char *devid_or_uuid, char *attrset_buff,
                            int buff_size)
{
    int ret = SERVICE_RESULT_ERR;
    dev_info_t *devinfo = NULL;

    devinfo = devmgr_get_devinfo(devid_or_uuid);
    PTR_RETURN(devinfo, ret, "get device info fail, devid:%s", devid_or_uuid);

    dev_profile_t *profile = __get_device_profile(devinfo->dev_base.model_id);
    PTR_GOTO(profile, out, "get device profile fail, model_id:0x%08x",
             (unsigned int)devinfo->dev_base.model_id);
    if (buff_size <= strlen(profile->attr_set)) {
        log_error("buffer size is too small");
        goto out;
    }

    strncpy(attrset_buff, profile->attr_set, buff_size);
    ret = SERVICE_RESULT_OK;

out:
    if (devinfo) {
        devmgr_put_devinfo_ref(devinfo);
    }
    return ret;
}

int stdd_get_device_service(dev_info_t *devinfo, char *service_buff,
                            int buff_size)
{
    int ret = SERVICE_RESULT_ERR;

    dev_profile_t *profile = __get_device_profile(devinfo->dev_base.model_id);
    PTR_GOTO(profile, out, "get device profile fail, model_id:0x%08x",
             (unsigned int)devinfo->dev_base.model_id);
    if (buff_size <= strlen(profile->cmd_set)) {
        log_error("buffer size is too small");
        goto out;
    }

    if (NULL != profile->cmd_set) {
        strncpy(service_buff, profile->cmd_set, buff_size - 1);
    }
    ret = SERVICE_RESULT_OK;

out:
    return ret;
}


const char *stdd_get_subdev_secret(uint32_t short_model, char *secret_buff,
                                   uint32_t buff_size)
{
    int ret = SERVICE_RESULT_ERR;
    char *result = NULL;
    char file_path[STR_LONG_LEN] = {0};
    char shortmodel_str[16] = {0};
    char *model_config = NULL;
    int len = 0;
    char *subdev_secret = NULL;

    void *lua_fd = stdd_lua_open();
    PTR_RETURN(lua_fd, NULL, "lua open fail");

    snprintf(file_path, sizeof(file_path) - 1, "%s%s/%s",
             os_get_storage_directory(), PROFILE_DIR_NAME, DEVICE_KEY_CONFIG_FILE_NAME);
    ret = stdd_lua_load_file(lua_fd, file_path);
    RET_GOTO(ret, out, "lua load file fail, file:%s", file_path);

    //device key config
    ret = stdd_lua_get_global_variable(lua_fd, DEVICE_KEY_CONFIG_NAME, &result);
    RET_GOTO(ret, out, "get %s failed from config file:%s", DEVICE_KEY_CONFIG_NAME,
             file_path);

    snprintf(shortmodel_str, sizeof(shortmodel_str), "%08x", short_model);
    model_config = json_get_value_by_name(result, strlen(result), shortmodel_str,
                                          &len, NULL);
    PTR_GOTO(model_config, out, "get devconfig fail, shortmodel:%s, keyconfig:",
             shortmodel_str, result);

    int secret_len = 0;
    char *secret_ptr = json_get_value_by_name(model_config, len, DEVICE_SECRET_NAME,
                                              &secret_len, NULL);
    PTR_GOTO(model_config, out, "get secret fail, modelconfig:%s", model_config);


    if (secret_len >= buff_size) {
        log_error("buffer size is too small");
        goto out;
    }
    strncpy(secret_buff, secret_ptr, secret_len);
    subdev_secret = secret_buff;

out:
    stdd_lua_close(lua_fd);
    return subdev_secret;
}


static int stdd_load_profile(void)
{
    int ret = SERVICE_RESULT_ERR;
    char profile_path[MAX_PROFILE_NAME_LEN] = {0};
    uint32_t model_id[MAX_MODEL_NUM] = {0};
    int model_num = MAX_MODEL_NUM;

    //获取并load全局属性和命令profile
    ret = get_global_profile(profile_path, sizeof(profile_path));
    RET_RETURN(ret, CALL_FUCTION_FAILED, "fmgr_get_global_profile");
    if (ret == SERVICE_RESULT_OK) {
        log_trace("global profile path:%s", profile_path);
        ret = stdd_update_global_profile(profile_path);
        RET_LOG(ret, "update global profile fail, profile:%s", profile_path);
    }

    //遍历子设备列表，依次load device profile
    ret = devmgr_get_all_device_modelid(model_id, &model_num);
    RET_RETURN(ret, CALL_FUCTION_FAILED, "devmgr_get_all_device_modelid");
    if (ret == SERVICE_RESULT_OK) {
        log_trace("all device count:%d", model_num);
        int i = 0;
        for (i  = 0; i < model_num; i++) {
            profile_path[0] = '\0';
            ret = get_device_profile_file(DEV_TYPE_ZIGBEE, model_id[i], profile_path,
                                          sizeof(profile_path));
            RET_LOG(ret, "get device profile fail, devie model id:0x%08x",
                    (unsigned int)model_id[i]);
            if (ret != SERVICE_RESULT_OK) {
                continue;
            }
            log_trace("model id:%08x, profile path:%s", (unsigned int)model_id[i],
                      profile_path);
            ret = stdd_update_device_profile(DEV_TYPE_ZIGBEE, model_id[i], profile_path);
            RET_LOG(ret, "update device profile fail, model id:0x%08x, profile:%s",
                    (unsigned int)model_id[i], profile_path);
        }
    }

    return SERVICE_RESULT_OK;
}


static void stdd_free_profile(void)
{
    stdd_free_attr_profile(&attr_profile_head);
    stdd_free_cmd_profile(&cmd_profile_head);
    stdd_free_device_profile_list(&device_profile_head);
}



void stdd_dump_attr_profile(void)
{
    __dump_attr_profile_list(&attr_profile_head);
}


void stdd_dump_cmd_profile(void)
{
    __dump_cmd_profile_list(&cmd_profile_head);
}


void stdd_dump_device_profile(void)
{
    dev_profile_t *pos;
    list_for_each_entry_t(pos, &device_profile_head, list_node, dev_profile_t) {
        __dump_device_profile(pos);
    }
}


int stdd_init(void)
{
    int ret = SERVICE_RESULT_ERR;

    attrs_profile_init();

    if (NULL == stdd_mpool) {
        stdd_mpool = mpool_create("profile", 4);
        PTR_RETURN(stdd_mpool, ret, "create mpool fail");
    }

    ret = stdd_load_profile();
    RET_LOG(ret, "load profile fail");

    stdd_dump_attr_profile();
    stdd_dump_cmd_profile();
    stdd_dump_device_profile();

    log_trace("stdd init success");
    return SERVICE_RESULT_OK;
}


void stdd_exit(void)
{
    attrs_profile_exit();

    stdd_free_profile();
    mpool_destroy(stdd_mpool);
    stdd_mpool = NULL;

    log_trace();
}


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
#include "log.h"
#include "json_parser.h"
#include "stdd_parser.h"
#include "stdd_datatype.h"

/*
 --alink标准属性值格式
 --"OnOff":"1"
 --"MaxPower":"210"
 --"AlarmState":{"ContactAlarm":"0","MotionAlarm":"0","WaterAlarm":"0","GasAlarm":"0",
                    "SmokeAlarm":"0","KeyFobValue":"1","SirenAlarm":"1"}
 --"NeighborTable":["0001ab2346583266"]
 --"ChildInfo":{"extAddr":"0001ab2346583266","childRssi":"20","parentRssi":"20"}

 --alink标准属性profile
 global_cluster_attr_profile =
 '[
     {"cluster_id":"0006","attr_set":[{"attr_id":"0000", "attr_name":"OnOff", "data_type":"boolean","element":[]}]},
     {"cluster_id":"0B04","attr_set":[{"attr_id":"0803", "attr_name":"MaxPower", "data_type":"int16","element":[]}]},
     {"cluster_id":"0500","attr_set":[{"attr_id":"0002", "attr_name":"AlarmState", "data_type":"bitmap8",
                                        "element":[{"mask_code":"0x01", "attr_name":"ContactAlarm", "data_type":"int8"},
                                                {"mask_code":"0x02", "attr_name":"MotionAlarm", "data_type":"int8"},
                                                {"mask_code":"0x04", "attr_name":"WaterAlarm", "data_type":"int8"},
                                                {"mask_code":"0x08", "attr_name":"GasAlarm", "data_type":"int8"},
                                                {"mask_code":"0x10", "attr_name":"SmokeAlarm", "data_type":"int8"},
                                                {"mask_code":"0x20", "attr_name":"KeyFobValue", "data_type":"int8"},
                                                {"mask_code":"0x40", "attr_name":"SirenAlarm", "data_type":"int8"}]},
    {"cluster_id":"fe01","attr_set":[{"attr_id":"0x0001","attr_name":"NeighborTable","data_type":"array",
                                        "element":{"data_type":"ieee_addr","element_name":""}},
                                     {"attr_id":"0x0003","attr_name":"ChildInfo","data_type":"structure",
                                        "element":[{"element_name":"extAddr","data_type":"ieee_addr"},
                                                {"element_name":"childRssi","data_type":"int8"},
                                                {"element_name":"parentRssi","data_type":"int8"}]}]}
 ]'

 --alink标准命令参数格式
 --"MoveHue":{"Hue":"16", "Direction":"1", "TrasitionTime":"23423","bitmap_arg":{"sub_arg1":"0","sub_arg2":"1"}}

 --alink标准命令profile
 global_cluster_cmd_profile =
 '[
     {"cluster_id":"0300", "cmd_set":[{"cmd_id":"01", "cmd_name":"MoveHue", "direction":"01",
                  "arg_list":[{"arg_name":"Hue","data_type":"uint8"},
                              {"arg_name":"Direction","data_type":"bit8"},
                              {"arg_name":"TransitionTime","data_type":"uint16"},
                              {"arg_name":"bitmap_arg","data_type":"bitmap8",
                                "element":[{"mask_code":"0x01", "arg_name":"sub_arg1", "data_type":"int8"},
                                           {"mask_code":"0x02", "arg_name":"sub_arg2", "data_type":"int8"}]}]}]
     }
 ]'


device_endpoint_mapping =
'[
    {"endpoint_id":"0x01","attr_mapping":[{"alias_name":"OnOff_A", "std_name":"OnOff"},
    									  {"alias_name":"MaxPower_A", "std_name":"MaxPower"}],
    {"endpoint_id":"0x02","attr_mapping":[{"alias_name":"OnOff_B", "std_name":"OnOff"},
    									  {"alias_name":"MaxPower_B", "std_name":"MaxPower"}],
]'


--设备属性集合，包括标准属性和私有属性
device_attr_set = '["OnOff","MaxPower","LigthOn"]'


--设备属性集合，包括标准命令和私有命令
device_cmd_set = '["MoveHue"]'
*/

//static const char *debug_attr_profile = "[{\"cluster_id\":\"0xfe01\",\"attr_set\":[{\"attr_id\":\"0x0000\",\"attr_name\":\"NeighborTableSize\",\"data_type\":\"uint8\"},{\"attr_id\":\"0x0001\",\"attr_name\":\"NeighborTable\",\"data_type\":\"array\",\"element\":{\"data_type\":\"structure\",\"element\":[{\"element_name\":\"extAddr\",\"data_type\":\"\"},{\"element_name\":\"rssi\",\"data_type\":\"int8\"}]}},{\"attr_id\":\"0x0002\",\"attr_name\":\"ChildTableSize\",\"data_type\":\"uint8\"},{\"attr_id\":\"0x0003\",\"attr_name\":\"ChildTable\",\"data_type\":\"array\",\"element\":{\"data_type\":\"structure\",\"element\":[{\"element_name\":\"extAddr\",\"data_type\":\"\"},{\"element_name\":\"childRssi\",\"data_type\":\"int8\"},{\"element_name\":\"parentRssi\",\"data_type\":\"int8\"}]}}]}]"


#define PROFILE_KEY_CLUSTER_ID      "cluster_id"
#define PROFILE_KEY_ATTR_SET        "attr_set"
#define PROFILE_KEY_ATTR_ID         "attr_id"
#define PROFILE_KEY_ATTR_NAME       "attr_name"
#define PROFILE_KEY_ELEMENT_NAME    "element_name"
#define PROFILE_KEY_ELEMENT         "element"
#define PROFILE_KEY_BITMASK         "bit_mask"
#define PROFILE_KEY_MASK_CODE       "mask_code"
#define PROFILE_KEY_DATA_TYPE       "data_type"
#define PROFILE_KEY_CMD_SET         "cmd_set"
#define PROFILE_KEY_CMD_ID          "cmd_id"
#define PROFILE_KEY_CMD_NAME        "cmd_name"
#define PROFILE_KEY_DIRECTION       "direction"
#define PROFILE_KEY_ARG_LIST        "arg_list"
#define PROFILE_KEY_ARG_NAME        "arg_name"
#define PROFILE_KEY_ENDPOINT_ID     "endpoint_id"
#define PROFILE_KEY_ATTR_MAPPING    "attr_mapping"
#define PROFILE_KEY_CMD_MAPPING     "cmd_mapping"
#define PROFILE_KEY_STD_NAME        "std_name"
#define PROFILE_KEY_ALIAS_NAME      "alias_name"

typedef void (*free_buff_cb_t)(void *buff);
#define __free_data_list(head, member, type, free_cb)\
    do{\
        data_list_t *pos, *next;\
        list_for_each_entry_safe_t(pos, next, head, member, type)\
        {\
            list_del(&(pos->list_node));\
            if((pos->data) && free_cb)\
            {\
                (*(free_buff_cb_t)free_cb)(pos->data);\
            }\
            stdd_free_buff(pos);\
        }\
    }while(0)


static int __get_id_from_hexstr(const char *hexstr, int str_len, void *id, int32_t id_len)
{
    char fmt[16] = {0};

    if(id_len != sizeof(uint8_t) && id_len != sizeof(uint16_t) &&
        id_len != sizeof(uint32_t)){
        log_error("invalid id length: %d", id_len);
        return SERVICE_RESULT_ERR;
    }

    if(strncmp(hexstr, "0x", 2) == 0 || strncmp(hexstr, "0X", 2) == 0)
        strncpy(fmt, hexstr, 2);
    strcat(fmt, "%");
    int len = strlen(fmt);

    uint32_t num_tmp = 0;
    snprintf(fmt + len, sizeof(fmt) - 1 - len, "0%dx", id_len*2);
    if(sscanf(hexstr, fmt, &num_tmp) != 1){
        log_error("invalid cluster id:%s", hexstr);
        return SERVICE_RESULT_ERR;
    }

    uint32_t mask = 0xffffffff>>(32-(id_len * 8));
    if(id_len == 1){
        uint8_t *num_id = id;
        *num_id = num_tmp & mask;
    }
    else if(id_len == 2){
        uint16_t *num_id = id;
        *num_id = num_tmp & mask;
    }
    else{
        uint32_t *num_id = id;
        *num_id = num_tmp & mask;
    }

    return SERVICE_RESULT_OK;
}


static void __free_attr_element(attr_element_t *element)
{
    if(element->next_element)
        __free_attr_element(element->next_element);

    if(element->child_element)
        __free_attr_element(element->child_element);

    stdd_free_buff(element);
}


static void __free_attr_profile(void *profile)
{
    if(((attr_profile_t *)profile)->attr_element)
        __free_attr_element(((attr_profile_t *)profile)->attr_element);

    stdd_free_buff(profile);
}


static void __free_cmd_argument(void *arg)
{
    cmd_argument_t *cmd_arg = (cmd_argument_t *)arg;

    if(cmd_arg->next_arg)
        __free_cmd_argument(cmd_arg->next_arg);

    if(cmd_arg->child_argument)
        __free_cmd_argument(cmd_arg->child_argument);

    stdd_free_buff(cmd_arg);
}


static void __free_cmd_profile(void *profile)
{
    if(((cmd_profile_t *)profile)->args)
        __free_cmd_argument(((cmd_profile_t *)profile)->args);

    stdd_free_buff(profile);
    return;
}


static void __free_name_mapping(list_head_t *mapping_head)
{
    name_mapping_t *pos, *next;
    list_for_each_entry_safe_t(pos, next, mapping_head, list_node, name_mapping_t){
        list_del(&pos->list_node);
        stdd_free_buff(pos);
    }
}


static void __free_endpoint_profile(endpoint_profile_t *profile)
{
    __free_name_mapping(&profile->attr_head);
    __free_name_mapping(&profile->cmd_head);
    stdd_free_buff(profile);
}


static void __free_endpoint_profile_list(list_head_t *endpoint_head)
{
    endpoint_profile_t *pos, *next;
    list_for_each_entry_safe_t(pos, next, endpoint_head, list_node, endpoint_profile_t){
        list_del(&pos->list_node);
        __free_endpoint_profile(pos);
    }
}


void __dump_attr_element(attr_element_t *element, char *prefix)
{
    char next_prefix[32] = {0};
    strcpy(next_prefix, prefix);
    strcat(next_prefix, "\t");
    data_type_t *data_type = stdd_get_datatype_by_id(element->data_type);

    os_printf("%selement_name:%s", prefix, element->element_name);
    os_printf(", data_type:0x%02x(%s)", element->data_type, data_type->name);
    if(element->mask_code != 0)
        os_printf(", mask_code:0x%08x", element->mask_code);
    os_printf("\n");

    if(element->next_element){
        __dump_attr_element(element->next_element, prefix);
    }

    if(element->child_element){
        os_printf("%schild element:\n", prefix);
        __dump_attr_element(element->child_element, next_prefix);
    }
}



void __dump_attr_profile(attr_profile_t *profile)
{
    data_type_t *data_type = stdd_get_datatype_by_id(profile->data_type);

    os_printf("cluster_id:0x%04x", profile->cluster_id);
    os_printf(", attr_id:0x%04x", profile->attribute_id);
    os_printf(", attr_name:%s", profile->attr_name);
    os_printf(", data_type:0x%02x(%s)\n", profile->data_type, data_type->name);
    if(profile->attr_element)
        __dump_attr_element(profile->attr_element, "\t");
}


void __dump_attr_profile_list(list_head_t *head)
{
    data_list_t *pos;
    //attr_profile_t *profile = NULL;
    os_printf("attr profile:\n");
    list_for_each_entry_t(pos, head, list_node, data_list_t){
        __dump_attr_profile((attr_profile_t*)pos->data);
    }
    os_printf("==========================================\n");
}


void __dump_cmd_argument(cmd_argument_t *argument, char *prefix)
{
    char next_prefix[32] = {0};
    strcpy(next_prefix, prefix);
    strcat(next_prefix, "\t");
    data_type_t *data_type = stdd_get_datatype_by_id(argument->data_type);

    os_printf("%sarg_name:%s", prefix, argument->arg_name);
    os_printf(", data_type:0x%02x(%s)\n", argument->data_type, data_type->name);
    if(argument->child_argument){
        os_printf("%schild_argument:\n", prefix);
        __dump_cmd_argument(argument->child_argument, next_prefix);
    }
    if(argument->next_arg){
        __dump_cmd_argument(argument->next_arg, prefix);
    }
}


void __dump_cmd_profile(cmd_profile_t *profile)
{
    os_printf("cluster_id:0x%04x", profile->cluster_id);
    os_printf(", cmd_id:0x%02x", profile->cmd_id);
    os_printf(", cmd_name:%s, direction:0x%02x\n", profile->cmd_name, profile->direction);
    if(profile->args)
        __dump_cmd_argument(profile->args, "\t");
}

void __dump_cmd_profile_list(list_head_t *head)
{
    data_list_t *pos;
    os_printf("cmd profile:\n");
    list_for_each_entry_t(pos, head, list_node, data_list_t){
        __dump_cmd_profile((cmd_profile_t*)pos->data);
    }
    os_printf("==========================================\n");
}


void __dump_endpoint_profile(endpoint_profile_t *profile)
{
    os_printf("\tendpoint 0x%02x config:\n", profile->endpoint_id);
    os_printf("\tattr mapping:\n");
    name_mapping_t *name_mapping;
    list_for_each_entry_t(name_mapping, &profile->attr_head, list_node, name_mapping_t){
        /*只print名称不一相同的纪录*/
        if(strcmp(name_mapping->user_name, name_mapping->std_name) != 0)
            os_printf("\t\taliasname:%s, stdname:%s\n", name_mapping->user_name, name_mapping->std_name);
    }

    os_printf("\tcmd mapping:\n");
    list_for_each_entry_t(name_mapping, &profile->cmd_head, list_node, name_mapping_t){
        if(strcmp(name_mapping->user_name, name_mapping->std_name) != 0)
            os_printf("\t\taliasname:%s, stdname:%s\n", name_mapping->user_name, name_mapping->std_name);
    }
    os_printf("==========================================\n");
}


void __dump_endpoint_profile_list(list_head_t *head)
{
    endpoint_profile_t *profile = NULL;
    list_for_each_entry_t(profile, head, list_node, endpoint_profile_t){
        __dump_endpoint_profile(profile);
    }
}

void __dump_device_profile(dev_profile_t *profile)
{
    os_printf("device profile:\n");
    os_printf("\tshort_model:0x%08x\n", (unsigned int)profile->model_id);
    os_printf("\tprofile_name:%s\n", profile->profile_name?profile->profile_name:"NULL");
    os_printf("\tattr_set:%s\n", profile->attr_set?profile->attr_set:"NULL");
    os_printf("\tcmd_set:%s\n", profile->cmd_set?profile->cmd_set:"NULL");
    __dump_endpoint_profile_list(&profile->endpoint_head);
    os_printf("==========================================\n");
}


static int stdd_parse_element_cb(char *element_str, int str_len, int str_type, list_head_t *head);
static attr_element_t *stdd_parse_complix_type(char *element_str, int str_len, int value_type)
{
    int type, len, ret = SERVICE_RESULT_ERR;
    char *element_name, *data_type = NULL;
    attr_element_t *element = NULL;
    attr_element_t *tail_element = NULL;
    data_list_t *pos, *next;

    LIST_HEAD(sub_element_head);

    char back_chr = 0;
    backup_json_str_last_char(element_str, str_len, back_chr);
    char *index, *entry;
    json_array_for_each_entry(element_str, index, entry, len, type){
        ret = stdd_parse_element_cb(entry, len, type, &sub_element_head);
        RET_LOG(ret, "parse element fail, element_str:%s", element_str);
    }
    restore_json_str_last_char(element_str, str_len, back_chr);

    list_for_each_entry_safe_t(pos, next, &sub_element_head, list_node, data_list_t){
        if(NULL ==  element)
            element = tail_element = (attr_element_t *)pos->data;
        else{
            tail_element->next_element = (attr_element_t *)pos->data;
            tail_element = tail_element->next_element;
        }
        pos->data = NULL;
    }
    __free_data_list(&sub_element_head, list_node, data_list_t, NULL);

end:
    return element;
}


static int stdd_parse_element_cb(char *element_str, int str_len, int str_type, list_head_t *element_head)//list_head_t *head
{
    int len, ret = SERVICE_RESULT_ERR;
    char *element_name, *sub_element, *data_type = NULL;

    /*new element*/
    attr_element_t *element = (attr_element_t *)stdd_new_buff(sizeof(attr_element_t));
    PTR_RETURN(element, ret, "new element fail");
    memset(element, 0, sizeof(attr_element_t));

    /*element name*/
    element_name = json_get_value_by_name(element_str, str_len, PROFILE_KEY_ELEMENT_NAME, &len, NULL);
    if(element_name)
        strncpy(element->element_name, element_name,
            sizeof(element->element_name)>len?len:sizeof(element->element_name) - 1);

    /*data type*/
    data_type = json_get_value_by_name(element_str, str_len, PROFILE_KEY_DATA_TYPE, &len, NULL);
    PTR_GOTO(data_type, err, "get data type fail, element:%s", element_str);
    data_type_t *type = stdd_get_datatype_by_name(data_type, len);
    PTR_GOTO(type, err, "invalid datatype:%s", data_type);
    element->data_type = type->id;

    /*mask code*/
    char *mask_code = json_get_value_by_name(element_str, str_len, PROFILE_KEY_MASK_CODE, &len, NULL);
    if(mask_code){
        ret = __get_id_from_hexstr(mask_code, len, (uint8_t *)&element->mask_code, sizeof(element->mask_code));
        RET_LOG(ret, "invalid mask code:%s", mask_code);
    }

    //object和array类型需要解析child element
    if(type->type_class == ALINK_DATATYPE_CLASS_STRUCTURE ||
        type->type_class == ALINK_DATATYPE_CLASS_COLLECTION){
        /*sub element str*/
        int value_type = 0;
        sub_element = json_get_value_by_name(element_str, str_len, PROFILE_KEY_ELEMENT, &len, &value_type);
        PTR_GOTO(sub_element, err, "get sub element fail, element:%s", element_str);

        element->child_element = stdd_parse_complix_type(sub_element, len, value_type);
        PTR_GOTO(element->child_element, err, "parser sub element fail, elementstr:%s", element_str);
    }

    /*new list node*/
    data_list_t *element_node = (data_list_t *)stdd_new_buff(sizeof(data_list_t));
    PTR_GOTO(element_node, err, "new data list node fail");

    element_node->data = element;
    list_add_tail(&element_node->list_node, element_head);

    return SERVICE_RESULT_OK;

err:
    if(element)
        __free_attr_element(element);

    return SERVICE_RESULT_ERR;
}


static int stdd_parse_attr_cb(char *attr_str, int str_len, int str_type, list_head_t *profile_head, char *cluster_id)//list_head_t *head, cluster_id
{
    int len, ret = SERVICE_RESULT_ERR;
    char *attr_id, *attr_name, *data_type = NULL;

    /*new attri profile*/
    attr_profile_t *profile = (attr_profile_t *)stdd_new_buff(sizeof(attr_profile_t));
    PTR_RETURN(profile, ret, "pmalloc fail");
    memset(profile, 0, sizeof(attr_profile_t));

    /*cluster id*/
    ret = __get_id_from_hexstr(cluster_id, strlen(cluster_id), &profile->cluster_id, sizeof(profile->cluster_id));
    RET_GOTO(ret, err, "invalid cluster id:%s", cluster_id);

    /*attr id*/
    attr_id = json_get_value_by_name(attr_str, str_len, PROFILE_KEY_ATTR_ID, &len, NULL);
    PTR_GOTO(attr_id, err, "get attr id fail, attrstr:%s", attr_str);
    ret = __get_id_from_hexstr(attr_id, len, &profile->attribute_id, sizeof(profile->attribute_id));
    RET_GOTO(ret, err, "invalid attr id:%s", attr_id);

    /*attr name*/
    attr_name = json_get_value_by_name(attr_str, str_len, PROFILE_KEY_ATTR_NAME, &len, NULL);
    PTR_GOTO(attr_name, err, "get attr name fail, attrname:%s", attr_name);
    strncpy(profile->attr_name, attr_name,
        sizeof(profile->attr_name)>len?len:sizeof(profile->attr_name) - 1);

    /*data type*/
    data_type = json_get_value_by_name(attr_str, str_len, PROFILE_KEY_DATA_TYPE, &len, NULL);
    PTR_GOTO(data_type, err, "get data type fail, datatype:%s", data_type);
    data_type_t *type = stdd_get_datatype_by_name(data_type, len);
    PTR_GOTO(type, err, "invalid datatype:%s", data_type);
    profile->data_type = type->id;

    //object,array和bitmap类型需要解析element
    if(type->type_class == ALINK_DATATYPE_CLASS_STRUCTURE ||
        type->type_class == ALINK_DATATYPE_CLASS_COLLECTION ||
        type->type_class == ALINK_DATATYPE_CLASS_BITMAP){

        /*attr element*/
        int str_type = 0;
        char *element_str = json_get_value_by_name(attr_str, str_len, PROFILE_KEY_ELEMENT, &len, &str_type);
        //bitmap类型的属性允许没有element字段
        if(type->type_class == ALINK_DATATYPE_CLASS_BITMAP && NULL == element_str){
            log_warn("get element fail");
        }
        else{
            PTR_GOTO(element_str, err, "get element fail, attrstr:%s", attr_str);
            profile->attr_element = stdd_parse_complix_type(element_str, len, str_type);
            if(NULL == profile->attr_element)
                log_warn("parser element return NULL, cluster_id:0x%04x, attr_id:0x%02x, attr_name:%s",
                        profile->cluster_id, profile->attr_name, profile->attr_name);
        }
    }
    /*new list node*/
    data_list_t *attr_profile_node = (data_list_t *)stdd_new_buff(sizeof(data_list_t));
    PTR_GOTO(attr_profile_node, err, "pmalloc fail");

    attr_profile_node->data = profile;
    list_add_tail(&attr_profile_node->list_node, profile_head);

    return SERVICE_RESULT_OK;

err:
    if(profile)
        __free_attr_profile(profile);

    return SERVICE_RESULT_ERR;
}


static int stdd_parse_cluster_attr_cb(char *cluster_str, int str_len, int str_type, list_head_t *profile_head)//list_head_t *head
{
    int ret = SERVICE_RESULT_OK;
    char *str_pos, *attr_set = NULL;
    char cluster_id[32] = {0};
    int type, len = 0;

    /*cluster id*/
    str_pos = json_get_value_by_name(cluster_str, str_len, PROFILE_KEY_CLUSTER_ID, &len, &type);
    PTR_RETURN(str_pos, SERVICE_RESULT_ERR, "get clusterid fail");
    strncpy(cluster_id, str_pos, len);
    cluster_id[len] = '\0';
    log_trace("cluster_id:%s", cluster_id);

    /*attr set*/
    attr_set = json_get_value_by_name(cluster_str, str_len, PROFILE_KEY_ATTR_SET, &len, &type);
    PTR_RETURN(attr_set, SERVICE_RESULT_ERR, "get attr_set fail, cluster str:%s",  cluster_str);
    if(type != JARRAY){
        log_error("invalid attr set:%s", attr_set);
        return SERVICE_RESULT_ERR;
    }

    char back_chr = 0;
    backup_json_str_last_char(attr_set, len, back_chr);
    char *entry, *pos;
    int entry_len;
    json_array_for_each_entry(attr_set, pos, entry, entry_len, type){
        ret = stdd_parse_attr_cb(entry, entry_len, type, profile_head, cluster_id);
        RET_LOG(ret, "parser attr set profile fail, attrset:%s", attr_set);
    }
    restore_json_str_last_char(attr_set, len, back_chr);

    return ret;
}


static int stdd_parse_cmd_args_cb(char *cmd_arg_str, int str_len, int str_type, list_head_t *arg_head);
static cmd_argument_t *stdd_parser_cmd_args(char *arg_list, int str_len)
{
    int ret = SERVICE_RESULT_ERR;
    LIST_HEAD(args_head);
    data_list_t *pos, *next;
    cmd_argument_t *start, *tail;
    start = tail = NULL;

    char back_chr = 0;
    backup_json_str_last_char(arg_list, str_len, back_chr);
    char *str_pos, *entry;
    int len, type;
    json_array_for_each_entry(arg_list, str_pos, entry, len, type){
        ret = stdd_parse_cmd_args_cb(entry, len, type, &args_head);
        RET_GOTO(ret, err, "parser cmd args fail, arglist:%s", arg_list);
    }
    restore_json_str_last_char(arg_list, str_len, back_chr);

    list_for_each_entry_safe_t(pos, next, &args_head, list_node, data_list_t){
        if(NULL == tail)
            start = tail = (cmd_argument_t *)pos->data;
        else{
            tail->next_arg = (cmd_argument_t *)pos->data;
            tail = tail->next_arg;
        }
        pos->data = NULL;
    }
    __free_data_list(&args_head, list_node, data_list_t, NULL);
    return start;

err:
    __free_data_list(&args_head, list_node, data_list_t, &__free_cmd_argument);
    return NULL;
}

static int stdd_parse_cmd_args_cb(char *cmd_arg_str, int str_len, int str_type, list_head_t *arg_head)//list_head_t *cmd_arg_head
{
    int len, ret = SERVICE_RESULT_ERR;
    char *arg_name, *data_type = NULL;

    /*new cmd argument*/
    cmd_argument_t *cmd_arg = (cmd_argument_t *)stdd_new_buff(sizeof(cmd_argument_t));
    PTR_RETURN(cmd_arg, SERVICE_RESULT_ERR, "new cmd argument fail");
    memset(cmd_arg, 0, sizeof(cmd_argument_t));

    /*cmd name*/
    arg_name = json_get_value_by_name(cmd_arg_str, str_len, PROFILE_KEY_ARG_NAME, &len, NULL);
    PTR_GOTO(arg_name, err, "get cmd argname fail, argname:%s", arg_name);
    strncpy(cmd_arg->arg_name, arg_name,
        sizeof(cmd_arg->arg_name)>len?len:sizeof(cmd_arg->arg_name) - 1);

    /*data type*/
    data_type = json_get_value_by_name(cmd_arg_str, str_len, PROFILE_KEY_DATA_TYPE, &len, NULL);
    PTR_GOTO(data_type, err, "get data type fail, datatype:%s", data_type);
    data_type_t *type = stdd_get_datatype_by_name(data_type, len);
    PTR_GOTO(type, err, "invalid datatype:%s", data_type);
    cmd_arg->data_type = type->id;

    /*mask code*/
    char *mask_code = json_get_value_by_name(cmd_arg_str, str_len, PROFILE_KEY_MASK_CODE, &len, NULL);
    if(mask_code != NULL){
        ret = __get_id_from_hexstr(mask_code, len, (uint8_t *)&cmd_arg->mask_code, sizeof(cmd_arg->mask_code));
        RET_LOG(ret, "invalid mask code:%s", mask_code);
    }

    /*bitmap类型参数，继续解析子参数*/
    if(type->type_class == ALINK_DATATYPE_CLASS_BITMAP){
        char *sub_argument = json_get_value_by_name(cmd_arg_str, str_len, PROFILE_KEY_ELEMENT, &len, NULL);
        if(NULL != sub_argument)
            cmd_arg->child_argument = stdd_parser_cmd_args(sub_argument, len);
        else
            log_warn("get element fail");
    }

    /*add to profile list tail*/
    data_list_t *arg_node = (data_list_t *)stdd_new_buff(sizeof(data_list_t));
    PTR_GOTO(arg_node, err, "new data list node fail");
    arg_node->data = cmd_arg;
    list_add_tail(&arg_node->list_node, arg_head);

    return SERVICE_RESULT_OK;
err:
    if(cmd_arg)
        __free_cmd_argument(cmd_arg);

    return SERVICE_RESULT_ERR;
}



static int stdd_parse_cmd_cb(char *cmd_str, int str_len, int str_type, list_head_t *profile_head, char *cluster_id)//list_head_t *profile_head, cluster_id
{
    int len, ret = SERVICE_RESULT_ERR;
    char *cmd_id, *cmd_name = NULL;
    LIST_HEAD(args_head);

    /*new attri profile*/
    cmd_profile_t *profile = (cmd_profile_t *)stdd_new_buff(sizeof(cmd_profile_t));
    PTR_RETURN(profile, ret, "pmalloc fail");

    /*cluster id*/
    ret = __get_id_from_hexstr(cluster_id, strlen(cluster_id), (uint8_t *)&profile->cluster_id, sizeof(profile->cluster_id));
    RET_GOTO(ret, err, "invalid cluster id:%s", cluster_id);

    /*cmd id*/
    cmd_id = json_get_value_by_name(cmd_str, str_len, PROFILE_KEY_CMD_ID, &len, NULL);
    PTR_GOTO(cmd_id, err, "get cmd id fail, cmdstr:%s", cmd_id);
    ret = __get_id_from_hexstr(cmd_id, len, (uint8_t *)&profile->cmd_id, sizeof(profile->cmd_id));
    RET_GOTO(ret, err, "invalid cmd id:%s", cmd_id);

    /*cmd name*/
    cmd_name = json_get_value_by_name(cmd_str, str_len, PROFILE_KEY_CMD_NAME, &len, NULL);
    PTR_GOTO(cmd_name, err, "get cmd name fail, cmdstr:%s", cmd_str);
    strncpy(profile->cmd_name, cmd_name,
        sizeof(profile->cmd_name)>len?len:sizeof(profile->cmd_name) - 1);

    /*direction*/
    char *direc = json_get_value_by_name(cmd_str, str_len, PROFILE_KEY_DIRECTION, &len, NULL);
    if(direc){
        int direction = 0;
        sscanf(direc, "%d", &direction);
        profile->direction = (unsigned char)direction;
    }

    /*arg list*/
    char *arg_list = json_get_value_by_name(cmd_str, str_len, PROFILE_KEY_ARG_LIST, &len, NULL);
    if(arg_list != NULL)
        profile->args = stdd_parser_cmd_args(arg_list, len);
    else
        profile->args = NULL;

    /*add to profile list tail*/
    data_list_t *profile_node = (data_list_t *)stdd_new_buff(sizeof(data_list_t));
    PTR_GOTO(profile_node, err, "new data list node fail");
    profile_node->data = profile;
    list_add_tail(&profile_node->list_node, profile_head);

    return SERVICE_RESULT_OK;
err:
    if(profile){
        __free_cmd_profile(profile);
    }

    return SERVICE_RESULT_ERR;
}


static int stdd_parse_cluster_cmd_cb(char *cluster_cmd_str, int str_len, int str_type, list_head_t *profile_head)//list_head_t *profile_head
{
    int ret = SERVICE_RESULT_OK;
    char *str_pos, *cmd_set = NULL;
    int type, len = 0;
    char cluster_id[32] = {0};

    /*cluster id*/
    str_pos = json_get_value_by_name(cluster_cmd_str, str_len, PROFILE_KEY_CLUSTER_ID, &len, &type);
    PTR_RETURN(str_pos, SERVICE_RESULT_ERR, "get clusterid fail");
    strncpy(cluster_id, str_pos, sizeof(cluster_id)>len?len:sizeof(cluster_id) - 1);

    /*cmd set*/
    cmd_set = json_get_value_by_name(cluster_cmd_str, str_len, PROFILE_KEY_CMD_SET, &len, &type);
    PTR_RETURN(cmd_set, SERVICE_RESULT_ERR, "get cmd_set fail");
    if(type != JARRAY){
        log_error("invalid cmd set:%s", cmd_set);
        return SERVICE_RESULT_ERR;
    }

    char back_chr = 0;
    backup_json_str_last_char(cmd_set, len, back_chr);
    char *entry;
    int entry_len;
    json_array_for_each_entry(cmd_set, str_pos, entry, entry_len, type){
        ret = stdd_parse_cmd_cb(entry, entry_len, type, profile_head, cluster_id);
    }
    restore_json_str_last_char(cmd_set, len, back_chr);

    return ret;
}


static int stdd_parse_str_cb(char *str, int str_len, int str_type, list_head_t *str_head)
{
    int ret = SERVICE_RESULT_ERR;

    data_list_t *str_node = (data_list_t *)stdd_new_buff(sizeof(data_list_t));
    PTR_RETURN(str_node, ret, "new data list node fail");

    str_node->data = stdd_new_buff(str_len + 1);
    *((char *)(str_node->data) + str_len) = '\0';
    list_add_tail(&str_node->list_node, str_head);

    return SERVICE_RESULT_OK;
}


static int stdd_parse_name_mapping_cb(char *mapping_str, int str_len, int str_type, list_head_t *mapping_head)//list_head
{
    int len, ret = SERVICE_RESULT_ERR;
    char *user_name, *std_name = NULL;
    LIST_HEAD(args_head);

    /*new name profile*/
    name_mapping_t *mapping = (name_mapping_t *)stdd_new_buff(sizeof(name_mapping_t));
    PTR_RETURN(mapping, ret, "pmalloc fail");

    /*std name*/
    std_name = json_get_value_by_name(mapping_str, str_len, PROFILE_KEY_STD_NAME, &len, NULL);
    PTR_GOTO(std_name, err, "get std name fail, mapping_str:%s, len = %d", mapping_str, str_len);
    strncpy(mapping->std_name, std_name, sizeof(mapping->std_name)>len?len:sizeof(mapping->std_name) - 1);

    /*user name*/
    user_name = json_get_value_by_name(mapping_str, str_len, PROFILE_KEY_ALIAS_NAME, &len, NULL);
    PTR_GOTO(user_name, err, "get user name fail, mapping_str:%s", mapping_str);
    strncpy(mapping->user_name, user_name, sizeof(mapping->user_name)>len?len:sizeof(mapping->user_name) - 1);

    /*cluster id, attribute id*/
    /*add to attr mapping list*/
    list_add_tail(&mapping->list_node, mapping_head);
    return SERVICE_RESULT_OK;
err:

    if(mapping)
        stdd_free_buff(mapping);

    return SERVICE_RESULT_ERR;
}


static int stdd_parse_endpoint_cb(char *endpoint_str, int str_len, int str_type, list_head_t *mapping_head)//endpoin_id, list_head_t
{
    int ret = SERVICE_RESULT_ERR;
    char *endpoint_id, *mapping_set = NULL;
    int type, len = 0;
    char back_chr = 0;
    char *str_pos, *entry;
    int entry_len = 0;

    /*new attr profile*/
    endpoint_profile_t *profile = (endpoint_profile_t *)stdd_new_buff(sizeof(endpoint_profile_t));
    PTR_RETURN(profile, ret, "pmalloc fail");
    list_init_head(&profile->attr_head);
    list_init_head(&profile->cmd_head);

    /*endpoint id*/
    endpoint_id = json_get_value_by_name(endpoint_str, str_len, PROFILE_KEY_ENDPOINT_ID, &len, &type);
    PTR_RETURN(endpoint_id, SERVICE_RESULT_ERR, "get endpoint id fail");
    ret = __get_id_from_hexstr(endpoint_id, len, (uint8_t *)&profile->endpoint_id, sizeof(profile->endpoint_id));
    RET_GOTO(ret, err, "invalid endpoint id:%s", endpoint_id);

    /*attr name mapping*/
    mapping_set = json_get_value_by_name(endpoint_str, str_len, PROFILE_KEY_ATTR_MAPPING, &len, &type);
    if(mapping_set != NULL){
        if(type != JARRAY){
            log_error("invalid attr mapping set:%s", mapping_set);
            ret = SERVICE_RESULT_ERR;
            goto err;
        }

        backup_json_str_last_char(mapping_set, len, back_chr);
        json_array_for_each_entry(mapping_set, str_pos, entry, entry_len, type){
            ret = stdd_parse_name_mapping_cb(entry, entry_len, type, &profile->attr_head);
            RET_LOG(ret, "parser name mapping set fail, mapping set:%s", mapping_set);
        }
        restore_json_str_last_char(mapping_set, len, back_chr);
    }

    /*cmd name mapping*/
    mapping_set = json_get_value_by_name(endpoint_str, str_len, PROFILE_KEY_CMD_MAPPING, &len, &type);
    if(mapping_set != NULL){
        if(type != JARRAY){
            log_error("invalid cmd mapping set:%s", mapping_set);
            ret = SERVICE_RESULT_ERR;
            goto err;
        }

        backup_json_str_last_char(mapping_set, len, back_chr);
        json_array_for_each_entry(mapping_set, str_pos, entry, entry_len, type){
            ret = stdd_parse_name_mapping_cb(entry, entry_len, type, &profile->cmd_head);
            RET_LOG(ret, "parser name mapping set fail, mapping set:%s", mapping_set);
        }
        restore_json_str_last_char(mapping_set, len, back_chr);
    }

    list_add_tail(&profile->list_node, mapping_head);

    return ret;

err:
    if(profile)
        __free_endpoint_profile(profile);

    return ret;
}


int stdd_parse_attr_profile(char *profile_str, list_head_t *profile_head)
{
    int ret = SERVICE_RESULT_OK;

    char *str_pos, *entry;
    int str_len, type;
    json_array_for_each_entry(profile_str, str_pos, entry, str_len, type){
        ret = stdd_parse_cluster_attr_cb(entry, str_len, type, profile_head);
        RET_LOG(ret, "parse attribute profile fail, profile:%s", profile_str);
    }

    return ret;
}


int stdd_parse_cmd_profile(char *profile_str, list_head_t *profile_head)
{
    int ret = SERVICE_RESULT_OK;

    char *str_pos, *entry;
    int str_len, type;
    json_array_for_each_entry(profile_str, str_pos, entry, str_len, type){
        ret = stdd_parse_cluster_cmd_cb(entry, str_len, type, profile_head);
        RET_LOG(ret, "parse command profile fail, profile:%s", profile_str);
    }

    return ret;
}


int stdd_parse_str_set(char *str_set, list_head_t *strset_head)
{
    int ret = SERVICE_RESULT_OK;

    char *str_pos, *entry;
    int str_len, type;
    json_array_for_each_entry(str_set, str_pos, entry, str_len, type){
        ret = stdd_parse_str_cb(entry, str_len, type, strset_head);
        RET_LOG(ret, "parse string set fail, profile:%s", str_set);
    }

    return ret;
}


int stdd_parse_endpoint_profile(char *profile_str, list_head_t *profile_head)
{
    int ret = SERVICE_RESULT_OK;

    char *str_pos, *entry;
    int str_len, type;
    json_array_for_each_entry(profile_str, str_pos, entry, str_len, type){
        ret = stdd_parse_endpoint_cb(entry, str_len, type, profile_head);
        RET_LOG(ret, "parse endpoint profile fail, profile:%s", profile_str);
    }

    return ret;
}


void stdd_free_device_profile(dev_profile_t *profile)
{
    __free_endpoint_profile_list(&profile->endpoint_head);
    if(profile->attr_set)
        stdd_free_buff(profile->attr_set);
    if(profile->cmd_set)
        stdd_free_buff(profile->cmd_set);
    if(profile->profile_name)
        stdd_free_buff(profile->profile_name);

    return;
}


void stdd_free_attr_profile(list_head_t *data_list_head)
{
    __free_data_list(data_list_head, list_node, data_list_t, &__free_attr_profile);
}


void stdd_free_cmd_profile(list_head_t *data_list_head)
{
    __free_data_list(data_list_head, list_node, data_list_t, &__free_cmd_profile);
}


void stdd_free_device_profile_list(list_head_t *head)
{
    dev_profile_t *pos, *next;
    list_for_each_entry_safe_t(pos, next, head, list_node, dev_profile_t){
        stdd_free_device_profile(pos);
        list_del(&pos->list_node);
        stdd_free_buff(pos);
    }
}


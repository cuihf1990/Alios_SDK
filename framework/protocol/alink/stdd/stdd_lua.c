#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "stdd_lua.h"
#include "stdd.h"
#include "msdp.h"
#include "digest_algorithm.h"
#include "devmgr_cache.h"
#include "alink_export_zigbee.h"

static int c_get_device_attr_cache(lua_State* lua_fd);
static int c_get_global_dev_attr(lua_State* lua_fd);
static int c_set_global_dev_attr(lua_State* lua_fd);
static int c_get_attr_mapping_set(lua_State* lua_fd);
static int c_get_cmd_mapping_name(lua_State* lua_fd);
static int c_get_alink_attr_set(lua_State* lua_fd);

static int c_alink_zigbee_register_device(lua_State* lua_fd);
static int c_alink_zigbee_unregister_device(lua_State* lua_fd);
static int c_alink_zigbee_report_status(lua_State* lua_fd);
static int c_alink_zigbee_report_event(lua_State* lua_fd);
static int c_alink_zigbee_update_online_status(lua_State* lua_fd);

int stdd_load_zigbee_lib(lua_State *lua_fd);

/*网关通用lib*/
static luaL_Reg common_lib[] = {
    {C_FUNC_GET_DEVICE_ATTR_CACHE, c_get_device_attr_cache},
    {C_FUNC_GET_GLOBAL_DEVICE_ATTR, c_get_global_dev_attr},
    {C_FUNC_SET_GLOBAL_DEVICE_ATTR, c_set_global_dev_attr},
    {C_FUNC_GET_ATTR_MAPPING_SET, c_get_attr_mapping_set},
    {C_FUNC_GET_CMD_MAPPING_NAME, c_get_cmd_mapping_name},
    {C_FUNC_GET_ALINK_ATTR_SET, c_get_alink_attr_set},
    {NULL, NULL}
};

/*alink api lib*/
static luaL_Reg api_lib[] = {
    {ALINK_ZIGBEE_REGISTER_DEVICE, c_alink_zigbee_register_device},
    {ALINK_ZIGBEE_UNREGISTER_DEVICE, c_alink_zigbee_unregister_device},
    {ALINK_ZIGBEE_REPORT_STATUS, c_alink_zigbee_report_status},
    {ALINK_ZIGBEE_REPORT_EVENT, c_alink_zigbee_report_event},
    {ALINK_ZIGBEE_UPDATE_ONLINE_STATUS, c_alink_zigbee_update_online_status},
    {NULL, NULL}
};


static int __lua_load_file(lua_State *lua_fd, const char *lua_file)
{
    int ret = luaL_loadfile(lua_fd, lua_file);
    RET_RETURN(ret, "lua_loadfile %s fail, ret:%d, error:%s\n", lua_file, ret, lua_tostring(lua_fd,-1));

    ret = lua_pcall(lua_fd,0,0,0);
    if(ret != 0)
    {
        log_error("lua_pcall %s return %d, error:%s\n", lua_file, ret, lua_tostring(lua_fd,-1));
        lua_pop(lua_fd, 1);
        return SERVICE_RESULT_ERR;
    }

    return SERVICE_RESULT_OK;
}


static char *__new_json_from_str_array(char *string_array[], int array_size)
{
    int buff_size = array_size * (STR_NAME_LEN + 3) + 3;
    char *json_buff = stdd_new_buff(buff_size);
    if(json_buff == NULL)
        return NULL;

    int len = snprintf(json_buff, buff_size, "[");
    while(array_size-- > 0 && buff_size > len){
        if(array_size > 0)
            len += snprintf(json_buff+len, buff_size, "\"%s\",", string_array[array_size]);
        else
            len += snprintf(json_buff+len, buff_size, "\"%s\"", string_array[array_size]);
    }
    len += snprintf(json_buff+len, buff_size, "]");

    if(buff_size == len){
        stdd_free_buff(json_buff);
        return NULL;
    }

    return json_buff;
}

static int stdd_load_commom_lib(lua_State *lua_fd)
{
    luaL_newlib(lua_fd, common_lib);
    return 1;
}


static int stdd_load_api_lib(lua_State *lua_fd)
{
    luaL_newlib(lua_fd, api_lib);
    return 1;
}

static int c_get_device_attr_cache(lua_State* lua_fd)
{
    int ret = SERVICE_RESULT_ERR;
    char *attr_cache = NULL;
    const char *dev_id = luaL_checkstring(lua_fd,1);
    const char *attr_name = luaL_checkstring(lua_fd,2);

    if(dev_id == NULL || attr_name == NULL){
        log_error("invalid arguments");
        return 0;
    }

    devmgr_read_attr_cache(dev_id, attr_name, &attr_cache);
    lua_pushstring(lua_fd, attr_cache?attr_cache:"");

    if(attr_cache){
        os_free(attr_cache);
        attr_cache = NULL;
    }

    return 1;
}


static int c_get_global_dev_attr(lua_State* lua_fd)
{
    const char *dev_id = luaL_checkstring(lua_fd,1);
    const char *attr_name = luaL_checkstring(lua_fd,2);

    if(dev_id == NULL || attr_name == NULL){
        log_error("invalid arguments");
        return 0;
    }

    int buff_size = 128;
    char *attr_value = os_malloc(buff_size);
    if(NULL == attr_value){
        log_error("os malloc error");
        return 0;
    }

    attr_value[0] = '\0';
    stdd_get_device_attr(dev_id, attr_name, attr_value, buff_size);
    lua_pushstring(lua_fd, attr_value);

    if(attr_value){
        os_free(attr_value);
        attr_value = NULL;
    }

    return 1;
}


static int c_set_global_dev_attr(lua_State* lua_fd)
{
    int ret = SERVICE_RESULT_ERR;
    const char *dev_id = luaL_checkstring(lua_fd,1);
    const char *attr_name = luaL_checkstring(lua_fd,2);
    const char *attr_value = luaL_checkstring(lua_fd,3);

    if(dev_id == NULL || attr_name == NULL || attr_value == NULL){
        log_error("invalid arguments");
        return 0;
    }

    ret = msdp_set_device_status(dev_id, attr_name, attr_value);
    lua_pushinteger(lua_fd, ret);

    return 1;
}


/*
 *描述:alink属性->zigbee属性/命令映射
 *入参:short_model, attr_name
 *出参:endpoint_id, attr_set
*/
static int c_get_attr_mapping_set(lua_State* lua_fd)
{
    int32_t ret = SERVICE_RESULT_ERR;
    uint32_t short_model = luaL_checkinteger(lua_fd,1);
    const char *attr_name = luaL_checkstring(lua_fd,2);
    int32_t lua_result = 0;
    uint8_t endpoint_id = 0;
    char *attr_array[32] = {NULL};

    if(short_model == 0 || attr_name == NULL){
        log_error("invalid arguments");
        return 0;
    }

    int32_t array_size = sizeof(attr_array)/sizeof(attr_array[0]);
    ret = stdd_get_attr_mapping_set(short_model, attr_name, &endpoint_id, attr_array, &array_size);
    RET_GOTO(ret, out, "get alink attr mapping set fail, short_model: 0x%08x, attr_name: %s",short_model, attr_name);

    char *attr_set = __new_json_from_str_array(attr_array, array_size);
    if(endpoint_id > 0 && attr_set != NULL){
        lua_pushinteger(lua_fd, endpoint_id);
        lua_pushstring(lua_fd, attr_set);

        lua_result = 2;
    }

    log_trace("attr_set:%s", attr_set);

    if(attr_set)
        stdd_free_buff(attr_set);

out:
    while(array_size-- > 1){
        if(attr_array[array_size] != NULL)
            stdd_free_buff(attr_array[array_size]);
        attr_array[array_size] = NULL;
    }

    return lua_result;
}


/*
 *描述:alink命令->zigbee命令映射
 *入参:short_model, cmd_name
 *出参:endpoint_id, cmd_name
*/
static int c_get_cmd_mapping_name(lua_State* lua_fd)
{
    int32_t ret = SERVICE_RESULT_ERR;
    uint32_t short_model = luaL_checkinteger(lua_fd,1);
    const char *cmd_name = luaL_checkstring(lua_fd,2);

    if(short_model == 0 || cmd_name == NULL){
        log_error("invalid arguments");
        return 0;
    }

    uint8_t endpoint_id = 0;
    char cmd_mapping_name[STR_NAME_LEN] = {0};

    ret = stdd_get_cmd_mapping_name(short_model, cmd_name, &endpoint_id, cmd_mapping_name, sizeof(cmd_mapping_name));
    if(endpoint_id > 0 && cmd_mapping_name != NULL){
        lua_pushinteger(lua_fd, endpoint_id);
        lua_pushstring(lua_fd, cmd_mapping_name);

        return 2;
    }

    return 0;
}


/*
 *描述:zigbee命令->alink属性映射
 *入参:short_model, endpoint_id, mapping_name
 *出参:attr_set
*/
static int c_get_alink_attr_set(lua_State* lua_fd)
{
    int32_t ret = SERVICE_RESULT_ERR;
    uint32_t short_model = luaL_checkinteger(lua_fd,1);
    uint8_t endpoint_id = luaL_checkinteger(lua_fd,2);
    const char *mapping_name = luaL_checkstring(lua_fd,3);

    if(short_model == 0 || endpoint_id == 0 || mapping_name == NULL){
        log_error("invalid arguments");
        return 0;
    }

    int32_t lua_result = 0;
    char *attr_array[32] = {NULL};
    int32_t array_size = sizeof(attr_array)/sizeof(attr_array[0]);
    ret = stdd_get_alink_attr_set(short_model, endpoint_id, mapping_name, attr_array, &array_size);
    RET_GOTO(ret, out, "get alink attr set fail, short_model: 0x%08x, mapping_name: %s",short_model, mapping_name);


    char *attr_set = __new_json_from_str_array(attr_array, array_size);
    if(attr_set != NULL){
        lua_pushstring(lua_fd, attr_set);
        lua_result = 1;
    }

    log_trace("attr_set:%s", attr_set);
    if(attr_set)
        stdd_free_buff(attr_set);

out:
    while(array_size--){
        if(attr_array[array_size])
            stdd_free_buff(attr_array[array_size]);
    }

    return lua_result;
}



/*
 *描述:注册设备api
 *入参:devid, model_id, secret
 *出参:result
*/
static int c_alink_zigbee_register_device(lua_State* lua_fd)
{
    int32_t ret = SERVICE_RESULT_ERR;
    char rand[SUBDEV_RAND_BYTES] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    const uint8_t *devid = luaL_checkstring(lua_fd,1);
    uint32_t short_model = luaL_checkinteger(lua_fd,2);
    const char *secret = luaL_checkstring(lua_fd,3);
    char buff[128] = {0};
    unsigned char md5_ret[16]= {0};
    char rand_hexstr[SUBDEV_RAND_BYTES*2 + 1] = {0};
    char sign_buff[STR_SIGN_LEN + 1] = {0};

    if (short_model == 0 || devid == NULL || secret == NULL){
        log_error("invalid arguments");
        return 0;
    }

    log_trace("short_model:0x%08x, devid:%s, secret:%s", short_model, devid, secret);

    int i, len = SUBDEV_RAND_BYTES;
    memcpy(buff, rand, len);
    len += snprintf(buff + len, sizeof(buff), "%s", secret);
    digest_md5(buff, len, md5_ret);
    for (i = 0; i < STR_SIGN_LEN/2; i++) {
        sprintf(sign_buff + i * 2, "%02x", md5_ret[i]);
    }

    bytes_2_hexstr(rand, SUBDEV_RAND_BYTES, rand_hexstr, sizeof(rand_hexstr));
    log_trace("rand hexstr:%s, secret:%s, sign:%s\n", rand_hexstr, secret, sign_buff);

    uint8_t ieee_addr[IEEE_ADDR_BYTES] = {0};
    get_ieeeaddr_by_string((char *)devid, ieee_addr);

    ret = alink_zigbee_register_device(ieee_addr, short_model, rand, sign_buff);
    lua_pushinteger(lua_fd, ret);

    return 1;
}

/*
 *描述:注销设备
 *入参:devid
 *出参:result
*/
static int c_alink_zigbee_unregister_device(lua_State* lua_fd)
{
    int32_t ret = SERVICE_RESULT_ERR;
    const uint8_t *devid = luaL_checkstring(lua_fd,1);

    if (devid == NULL ){
        log_error("invalid arguments");
        return 0;
    }

    log_trace("devid:%s", devid);

    uint8_t ieee_addr[IEEE_ADDR_BYTES] = {0};
    get_ieeeaddr_by_string((char *)devid, ieee_addr);

    ret = alink_zigbee_unregister_device(ieee_addr);
    lua_pushinteger(lua_fd, ret);

    return 1;
}


/*
 *描述:上报设备状态
 *入参:devid, endpoint_id, attr_name, attr_value
 *出参:result
*/
static int c_alink_zigbee_report_status(lua_State* lua_fd)
{
    int32_t ret = SERVICE_RESULT_ERR;
    const uint8_t *devid = luaL_checkstring(lua_fd,1);
    uint8_t endpoint_id = luaL_checkinteger(lua_fd,2);
    const uint8_t *attr_name = luaL_checkstring(lua_fd,3);
    const uint8_t *attr_value = luaL_checkstring(lua_fd,4);

    if (devid == NULL || attr_name == NULL || attr_value == NULL){
        log_error("invalid arguments");
        return 0;
    }

    log_trace("endpoint_id:0x%02x, devid:%s, attr_name:%s, attr_value:%s", endpoint_id, devid, attr_name, attr_value);

    uint8_t ieee_addr[IEEE_ADDR_BYTES] = {0};
    get_ieeeaddr_by_string((char *)devid, ieee_addr);

    const char *name_set[2] = {attr_name, NULL};
    const char *value_set[2] = {attr_value, NULL};
    ret = alink_zigbee_report_attrs(ieee_addr, endpoint_id, name_set, value_set);
    lua_pushinteger(lua_fd, ret);

    return 1;
}


/*
 *描述:上报设备事件
 *入参:devid, endpoint_id, event_name, event_args
 *出参:result
*/
static int c_alink_zigbee_report_event(lua_State* lua_fd)
{
    int32_t ret = SERVICE_RESULT_ERR;
    const uint8_t *devid = luaL_checkstring(lua_fd,1);
    uint32_t endpoint_id = luaL_checkinteger(lua_fd,2);
    const uint8_t *event_name = luaL_checkstring(lua_fd,3);
    const uint8_t *event_args = luaL_checkstring(lua_fd,4);

    if (devid == NULL || event_name == NULL || event_args == NULL){
        log_error("invalid arguments");
        return 0;
    }

    log_trace("endpoint_id:0x%02x, devid:%s, event_name:%s, event_args:%s", endpoint_id, event_name, event_args);

    uint8_t ieee_addr[IEEE_ADDR_BYTES] = {0};
    get_ieeeaddr_by_string((char *)devid, ieee_addr);

    ret = alink_zigbee_report_event(ieee_addr, endpoint_id, event_name, event_args);
    lua_pushinteger(lua_fd, ret);

    return 1;
}

/*
 *描述:上报设备事件
 *入参:devid, online_or_not
 *出参:result
*/
static int c_alink_zigbee_update_online_status(lua_State* lua_fd)
{
    int32_t ret = SERVICE_RESULT_ERR;
    const uint8_t *devid = luaL_checkstring(lua_fd,1);
    uint8_t online = luaL_checkinteger(lua_fd,2);

    if (devid == NULL){
        log_error("invalid arguments");
        return 0;
    }

    log_trace("devid:%s, online:%d", devid, online);

    uint8_t ieee_addr[IEEE_ADDR_BYTES] = {0};
    get_ieeeaddr_by_string((char *)devid, ieee_addr);

    ret = alink_zigbee_update_online_status(ieee_addr, online);
    lua_pushinteger(lua_fd, ret);

    return 1;
}



int stdd_lua_load_profile_context(void **lua_fd, uint32_t model_id)
{
    int ret = SERVICE_RESULT_ERR;
    char profile_path[MAX_PROFILE_NAME_LEN] = {0};

    ret = stdd_get_device_profile_name(DEV_TYPE_ZIGBEE, model_id, profile_path, sizeof(profile_path));
    if(strlen(profile_path) == 0 || ret != SERVICE_RESULT_OK){
        log_warn("get device profile path fail, model id:0x%08x", model_id);
        //profile不存在返回失败
        return SERVICE_RESULT_ERR;
    }
    if(NULL == *lua_fd){
        *lua_fd = stdd_lua_open();
        PTR_RETURN(lua_fd, ret, "lua open fail");
    }

    ret = __lua_load_file(*lua_fd, profile_path);
    RET_RETURN(ret, "lua load file fail, file:%s", profile_path);

    return ret;
}


int stdd_lua_get_global_function(void *lua_fd, const char *function_name)
{
    int ret = SERVICE_RESULT_OK;
    const char *buff = NULL;

    //push lua fuction
    if(0 == lua_getglobal(lua_fd, function_name))
    {
        ret = SERVICE_RESULT_ERR;
        RET_RETURN(ret, "lua_getglobal %s fail, error:%s\n", function_name, lua_tostring(lua_fd,-1));
    }
    return ret;
}


int stdd_lua_get_global_variable(void *lua_fd, const char *global_name, char **result)
{
    int ret = SERVICE_RESULT_OK;
    const char *buff = NULL;

    //push lua fuction
    if(0 == lua_getglobal(lua_fd, global_name))
    {
        ret = SERVICE_RESULT_ERR;
        RET_RETURN(ret, "lua_getglobal %s fail, error:%s\n", global_name, lua_tostring(lua_fd,-1));
    }

    if(result){
        *result = (char *)lua_tostring(lua_fd, -1);
        if(NULL == *result)
            ret = SERVICE_RESULT_ERR;
    }

    lua_pop(lua_fd, 1);
    return ret;
}


/*
 *arg类型全部为char *
*/
int stdd_lua_call_function(void *lua_fd, const char *function_name, char *result[], int result_num, int arg_num, ...)
{
    int ret = SERVICE_RESULT_OK;
    int i = arg_num;
    char *arg = NULL;
    va_list ap;
    va_start(ap, arg_num);

    //push lua fuction
    if(0 == lua_getglobal(lua_fd, function_name)){
        ret = SERVICE_RESULT_ERR;
        RET_GOTO(ret, err, "lua_getglobal %s return 0, error:%s\n", function_name, lua_tostring(lua_fd,-1));
    }

    //参数压栈
    while(i-- > 0){
        arg = va_arg(ap, char *);
        PTR_GOTO(arg, err, "get arguments fail");
        lua_pushstring(lua_fd, arg);
    }

    //lua函数调用
    ret = lua_pcall(lua_fd, arg_num, result_num, 0);
    RET_GOTO(ret, err, "lua_pcall %s return %d, error:%s\n", function_name, ret, lua_tostring(lua_fd,-1));

    //从栈顶获取lua函数调用返回值
    i = 0;
    while(i < result_num && result){
        result[i] = (char *)lua_tostring(lua_fd, i - result_num);
        if(NULL == result[i]){
            log_error("call lua function result:NULL");
            return SERVICE_RESULT_ERR;
        }
        i++;
    }
    //返回值出栈
    while(result_num-- > 0)
        lua_pop(lua_fd, 1);

    return SERVICE_RESULT_OK;

err:
    lua_pop(lua_fd, 1);

    return ret;
}


int stdd_lua_load_file(void *lua_fd, const char *file_path)
{
    int ret = SERVICE_RESULT_ERR;

    ret = __lua_load_file((lua_State *)lua_fd, file_path);
    RET_RETURN(ret, "lua_load_file:%s fail, ret = %d\n", file_path, ret);

    return SERVICE_RESULT_OK;
}


void *stdd_lua_open(void)
{
    lua_State *lua_fd = luaL_newstate();
    PTR_RETURN(lua_fd, NULL, "lua new state fail");

    luaL_openlibs(lua_fd);

    /*注册zigbee device lib*/
    luaL_requiref(lua_fd, ZIGBEE_LIB_NAME, stdd_load_zigbee_lib, 1);
    lua_pop(lua_fd, 1);  /* remove lib */

    /*注册global lib*/
    luaL_requiref(lua_fd, COMMON_LIB_NAME, stdd_load_commom_lib, 1);
    lua_pop(lua_fd, 1);  /* remove lib */

    /*注册alink api lib*/
    luaL_requiref(lua_fd, API_LIB_NAME, stdd_load_api_lib, 1);
    lua_pop(lua_fd, 1);  /* remove lib */

    return lua_fd;
}


void stdd_lua_close(void *lua_fd)
{
    if(lua_fd)
        lua_close(lua_fd);
}



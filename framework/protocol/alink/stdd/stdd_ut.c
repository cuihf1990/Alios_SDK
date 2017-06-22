#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "stdd_ut.h"
#include "stdd.h"
#include "stdd_lua.h"
#include "stdd_dataype.h"


static LIST_HEAD(attr_profile_head);
static LIST_HEAD(cmd_profile_head);
static LIST_HEAD(endpoint_profile_head);

void *alink_cb_func[ALINK_CB_MAX_NUM] = {NULL};

//all
char attr_profile[] =
    "[{\"cluster_id\":\"fe01\",\"attr_set\":[{\"attr_id\":\"0000\",\"attr_name\":\"NeighborTableSize\",\"data_type\":\"uint8\"},{\"attr_id\":\"0x0001\",\"attr_name\":\"NeighborTable\",\"data_type\":\"array\",\"element\":{\"data_type\":\"struct\",\"element\":[{\"element_name\":\"extAddr\",\"data_type\":\"uint8\"},{\"element_name\":\"rssi\",\"data_type\":\"int8\"}]}},{\"attr_id\":\"0x0002\",\"attr_name\":\"ChildTableSize\",\"data_type\":\"uint8\"},{\"attr_id\":\"0x0003\",\"attr_name\":\"ChildTable\",\"data_type\":\"array\",\"element\":{\"data_type\":\"struct\",\"element\":[{\"element_name\":\"extAddr\",\"data_type\":\"int8\"},{\"element_name\":\"childRssi\",\"data_type\":\"int8\"},{\"element_name\":\"parentRssi\",\"data_type\":\"int8\"}]}}]}]";
//“ªŒ¨∏¥‘”¿‡–Õ
//char attr_profile[] = "[{\"cluster_id\":\"fe01\",\"attr_set\":[{\"attr_id\":\"0000\",\"attr_name\":\"NeighborTableSize\",\"data_type\":\"uint8\"},{\"attr_id\":\"0x0001\",\"attr_name\":\"NeighborTable\",\"data_type\":\"array\",\"element\":{\"data_type\":\"uint8\",}},{\"attr_id\":\"0x0002\",\"attr_name\":\"ChildTableSize\",\"data_type\":\"uint8\"},{\"attr_id\":\"0x0003\",\"attr_name\":\"ChildTable\",\"data_type\":\"struct\",\"element\":[{\"element_name\":\"extAddr\",\"data_type\":\"int8\"},{\"element_name\":\"childRssi\",\"data_type\":\"int8\"},{\"element_name\":\"parentRssi\",\"data_type\":\"int8\"}]}]}]";
//only array
//char attr_profile[] = "[{\"cluster_id\":\"fe01\",\"attr_set\":[{\"attr_id\":\"0000\",\"attr_name\":\"NeighborTableSize\",\"data_type\":\"uint8\"},{\"attr_id\":\"0x0001\",\"attr_name\":\"NeighborTable\",\"data_type\":\"array\",\"element\":{\"data_type\":\"uint8\",}},{\"attr_id\":\"0x0002\",\"attr_name\":\"ChildTableSize\",\"data_type\":\"uint8\"}]}]";
//simple
//char attr_profile[] = "[{\"cluster_id\":\"fe01\",\"attr_set\":[{\"attr_id\":\"0000\",\"attr_name\":\"NeighborTableSize\",\"data_type\":\"uint8\"},{\"attr_id\":\"0X0002\",\"attr_name\":\"ChildTableSize\",\"data_type\":\"uint8\"}]}]";

char cmd_profile[] =
    "[{\"cluster_id\":\"0300\",\"cmd_set\":[{\"cmd_id\":\"01\",\"cmd_name\":\"MoveHue\",\"direction\":\"00\",\"arg_list\":[{\"arg_name\":\"Hue\",\"data_type\":\"uint8\"},{\"arg_name\":\"Direction\",\"data_type\":\"uint8\"},{\"arg_name\":\"TransitionTime\",\"data_type\":\"uint16\"}]}]}]";
char endpoint_profile[] =
    "[{\"endpoint_id\":\"01\",\"cmd_mapping\":[],\"attr_mapping\":[{\"user_name\":\"OnOff_A\",\"std_name\":\"OnOff\"},{\"user_name\":\"MaxPower_A\",\"std_name\":\"MaxPower\"}]},{\"endpoint_id\":\"02\",\"cmd_mapping\":[],\"attr_mapping\":[{\"user_name\":\"OnOff_B\",\"std_name\":\"OnOff\"},{\"user_name\":\"MaxPower_B\",\"std_name\":\"MaxPower\"}]}]";

char attr_set[] =
    "[\"NeighborTableSize\",\"NeighborTable\",\"ChildTableSize\",\"ChildTable\"]";
char cmd_set[] = "[\"MoveHue\"]";
char private_attr_profile[] = "[]";
char private_cmd_profile[] = "[]";
char model_name[] = "TEST_DEVICE_MODEL";
char model_id[] = "1024";


int stdd_zbnet_update_attr_profile_cb(attr_profile_t *profile[])
{
    return 0;
}

int stdd_zbnet_update_cmd_profile_cb(cmd_profile_t *profile[])
{
    return 0;
}

int stdd_zbnet_get_attr_cb(uint8_t ieee_addr[8], \
                           uint8_t endpoint_id, const char *attr_set[])
{
    return 0;
}

int stdd_zbnet_set_attr_cb(uint8_t ieee_addr[8], uint8_t endpoint_id, \
                           const char *attr_name, const char *attr_value)
{
    return 0;
}

int stdd_zbnet_exec_cmd_cb(uint8_t ieee_addr[8], uint8_t endpoint_id, \
                           const char *cmd_name, const char *cmd_args)
{
    return 0;
}

#if 0
int stdd_zbnet_get_attr(const char *devid_or_uuid, const char *attr_name)
{
    return 0;
}

int stdd_zbnet_set_attr(const char *devid_or_uuid, const char *attr_name,
                        const char *attr_value)
{
    return 0;
}

int stdd_zbnet_exec_rpc(const char *devid_or_uuid, const char *rpc_name,
                        const char *rpc_args)
{
    return 0;
}
#endif

dev_info_t *devmgr_get_devinfo(const char *devid_or_uuid)
{
    return (dev_info_t *)0x01010101;
}

dev_info_t *devmgr_get_devinfo_by_ieeeaddr(char ieee_addr[8])
{
    return (dev_info_t *)0x01010101;
}

int devmgr_update_attr_cache(const char *devid_or_uuid, const char *attr_name,
                             const char *attr_value)
{
    return 0;
}
void devmgr_put_devinfo_ref(dev_info_t *devinfo)
{
    return;
}

int msdp_report_device_status(const char *uuid, const char *attr_name,
                              const char *attr_value)
{
    return 0;
}

/*

int stdd_lua_get_global_variable(void *lua_fd, const char *global_name, char**result)
{
    if(strcmp(global_name, LUA_GLOBAL_ATTR_PROFILE) == 0)
        *result = attr_profile;
    else if (strcmp(global_name, LUA_GLOBAL_CMD_PROFILE) == 0)
        *result = cmd_profile;
    else if (strcmp(global_name, LUA_ENDPOINT_PROFILE) == 0)
        *result = endpoint_profile;
    else if (strcmp(global_name, LUA_DEVICE_MODEL_NAME) == 0)
        *result = model_name;
    else if (strcmp(global_name, LUA_DEVICE_MODEL_ID) == 0)
        *result = model_id;
    else if (strcmp(global_name, LUA_DEVICE_ATTR_SET) == 0)
        *result = attr_set;
    else if (strcmp(global_name, LUA_DEVICE_CMD_SET) == 0)
        *result = cmd_set;
    else if(strcmp(global_name, LUA_PRIVATE_ATTR_PROFILE) == 0)
        *result = private_attr_profile;
    else if(strcmp(global_name, LUA_PRIVATE_CMD_PROFILE) == 0)
        *result = private_cmd_profile;
    else{
        return -1;
    }

    return 0;
}


int stdd_lua_call_function(void *lua_fd, const char *function_name, void **result, int arg_num, ...)
{
    return 0;
}


void int stdd_lua_load_file(void *lua_fd, const char *file_path)
{
    return 0;
}


void *stdd_lua_open(const char *file_path)
{
    return (void *)0x00000001;
}


void stdd_lua_close(void *lua_fd)
{
    return;
}
*/

int devmgr_get_all_device_modelid(uint32_t model_id[], int *num)
{
    model_id[0] = 1024;
    *num = 1;

    return 0;
}


int get_device_profile_file(uint8_t dev_type, uint32_t model_id,
                            char file_name[], int max_name_length)
{
    strcpy(file_name, "./model_profile_sample.lua");
    return 0;
}

int get_global_profile(char file_name[], int max_name_length)
{
    //strcpy(file_name, "./mylua.lua");
    get_profile_path_prefix(file_name);
    strcat(file_name, "global_profile_sample.lua");
    return 0;
}


int attr_profile_ut(char *profile)
{
    log_print("attr profile:%s, len:%d", profile, strlen(profile));
    int ret = stdd_parse_attr_profile(profile, &attr_profile_head);
    RET_RETURN(ret, "parse attr profile fail");

    __dump_attr_profile_list(&attr_profile_head);
    return 0;
}


int cmd_profile_ut(char *profile)
{
    log_print("cmd profile:%s, len:%d", profile, strlen(profile));
    int ret = stdd_parse_cmd_profile(profile, &cmd_profile_head);
    RET_RETURN(ret, "parse cmd profile fail");

    __dump_cmd_profile_list(&cmd_profile_head);
    return 0;
}


int endpoint_profile_ut(char *profile)
{
    log_print("endpoint profile:%s, len:%d", profile, strlen(profile));
    int ret = stdd_parse_endpoint_profile(profile, &endpoint_profile_head);
    RET_RETURN(ret, "parse endpoint profile fail");

    __dump_endpoint_profile_list(&endpoint_profile_head);
    return 0;
}


int main(int argc, char *argv[])
{
    stdd_init();
    //attr_profile_ut(attr_profile);
    //cmd_profile_ut(cmd_profile);
    //endpoint_profile_ut(endpoint_profile);

    stdd_dump_attr_profile();
    stdd_dump_cmd_profile();
    stdd_dump_device_profile();

    return 0;
}


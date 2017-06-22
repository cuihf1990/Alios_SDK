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

#ifndef __STDD_UT__H__
#define __STDD_UT__H__
#include <stdint.h>
#include <unistd.h>
#include "list.h"
#include "alink_export_zigbee.h"

#ifdef __cplusplus
extern "C" {
#endif

#define os_mutex_init()  \
    ((void *)0x010101010)

#define os_mutex_lock(mutex)
#define os_mutex_unlock(mutex)
#define os_mutex_destroy(mutex) (mutex = NULL)

#define CALL_FUCTION_FAILED         "Call function \"%s\" failed\n"

#define MAX_UUID_LEN    33
#define MAX_ATTR_NAME_LEN                     80
#define MAX_SERVICE_NAME_LEN                  80


typedef int (*ALINK_SERVICE_EXECUTE_CB)(const char *args,
                                        char *json_out_buf, unsigned int buf_sz);

#define log_print(fmt, args...) \
    do {  \
        printf("[STDD] %s:%4d %20s: " fmt "\n", __FILE__, __LINE__, __func__, ##args); \
    } while(0)

#define log_fatal(FMT, args...)     log_print(FMT, ##args)
#define log_error(FMT, args...)     log_print(FMT, ##args)
#define log_info(FMT, args...)      log_print(FMT, ##args)
#define log_dump(FMT, args...)      log_print(FMT, ##args)
#define log_debug(FMT, args...)     log_print(FMT, ##args)
#define log_trace(FMT, args...)     log_print(FMT, ##args)
#define log_warn(FMT, args...)     log_print(FMT, ##args)


#define RET_FAILED(ret)  (ret != SERVICE_RESULT_OK)

#define RET_GOTO(Ret,gotoTag,strError, args...)         \
      {\
        if ( RET_FAILED(Ret) )    \
        {   \
            log_trace(strError, ##args); \
            goto gotoTag; \
        }\
      }

#define RET_FALSE(Ret,strError,args...)         \
    {\
        if ( RET_FAILED(Ret) )    \
        {   \
            log_trace(strError, ##args); \
            return false; \
        }\
     }

#define RET_RETURN(Ret,strError,args...)         \
    {\
        if ( RET_FAILED(Ret) )    \
        {   \
            log_trace(strError, ##args); \
            return Ret; \
        }\
    }
#define RET_LOG(Ret,strError,args...)         \
    {\
        if ( RET_FAILED(Ret) )    \
        {   \
            log_error(strError, ##args); \
        }\
    }

#define PTR_RETURN(Pointer,Ret,strError,args...)         \
    {\
        if ( !Pointer)    \
        {   \
            log_trace(strError, ##args); \
            return Ret; \
        }\
     }

#define PTR_FALSE(Pointer,strError,args...)         \
    {\
        if ( !Pointer)    \
        {   \
            log_trace(strError, ##args); \
            return FALSE; \
        }\
    }
#define PTR_LOG(Pointer,strError,args...)         \
    {\
        if ( !Pointer)    \
        {   \
            log_error(strError, ##args); \
        }\
    }


#define PTR_GOTO(Pointer, gotoTag, strError, args...)         \
    {\
        if ( !Pointer)    \
        {   \
            log_trace(strError, ##args); \
            goto gotoTag; \
        }\
     }

#define POINTER_RETURN(Pointer,strError,args...)         \
    {\
        if ( !Pointer)    \
        {   \
            log_trace(strError, ##args); \
            return Pointer; \
        }\
     }



enum SERVICE_CODE {
    SERVICE_BUFFER_INSUFFICENT = -3,
    SERVICE_CODE_UNKNOWN       = -2,
    SERVICE_RESULT_ERR         = -1,
    SERVICE_RESULT_OK          =  0,

    SERVICE_CODE_BEGIN         = 0x0100,
    SERVICE_EVENT,
    SERVICE_DATA,
    SERVICE_CONN_INIT,
    SERVICE_CONN_READY,
    SERVICE_CONN_CLOSE,
    SERVICE_ATTACH,
    SERVICE_DETACH,
    SERVICE_STATE_INIT,
    SERVICE_STATE_PREPARE,
    SERVICE_STATE_READY,
    SERVICE_STATE_BUSY,
    SERVICE_STATE_STOP,
    SERVICE_CODE_END           = 0x01FF,
};


/*
* 设备类型定义
*/
typedef enum {
    DEV_TYPE_GATEWAY        = 0,
    DEV_TYPE_WIFI           = 1,
    DEV_TYPE_ZIGBEE         = 2,
    DEV_TYPE_BLE            = 3,
    DEV_TYPE_WIFI_SUB       = 4,
    DEV_TYPE_MAX
} devtype_t;



typedef struct dev_base_s {
    uint8_t dev_type;
    uint32_t model_id;
    union {
        char ether_mac[6];
        char ieee_addr[8];
    } u;
    char rand[16];
    char sign[17];
    char dev_id[18];
    char uuid[33];
    char token[7];
    uint16_t crc;
} dev_base_t;


typedef struct dev_info_s {
    struct list_head list_node;
    struct list_head attr_head;
    dev_base_t dev_base;
    //char model_str[33];
    uint8_t device_idx; //0xff为无效值
    uint8_t cloud_state;
    uint8_t link_state;
    void *dev_mutex;
    uint8_t ref_cnt;
} dev_info_t;

extern void *alink_cb_func[ALINK_CB_MAX_NUM];

int devmgr_get_dev_type(const char *devid_or_uuid, uint8_t *dev_type);
int32_t alink_post(const char *method, char *buff);
int devmgr_read_attr_cache(const char *devid_or_uuid, const char *attr_name,
                           char **attr_value);
dev_info_t *devmgr_get_devinfo(const char *devid_or_uuid);
dev_info_t *devmgr_get_devinfo_by_ieeeaddr(char ieee_addr[8]);
int devmgr_update_attr_cache(const char *devid_or_uuid, const char *attr_name,
                             const char *attr_value);

int msdp_report_device_status(const char *uuid, const char *attr_name,
                              const char *attr_value);
void devmgr_put_devinfo_ref(dev_info_t *devinfo);


const char *config_get_main_uuid(void);

int stdd_get_device_attrset(const char *devid_or_uuid, char *attrset_buff,
                            int buff_size);

/*
int stdd_lua_get_global_variable(void *lua_fd, const char *global_name, char **result);

int stdd_lua_call_function(void *lua_fd, const char *function_name, char **result, int arg_num, ...);

int stdd_lua_load_file(void *lua_fd, const char *file_path);

void *stdd_lua_open();

void stdd_lua_close(void *lua_fd);
*/

int devmgr_get_all_device_modelid(uint32_t model_id[], int *num);

int get_device_profile_file(uint8_t dev_type, uint32_t model_id,
                            char file_name[], int max_name_length);


#ifdef __cplusplus
}
#endif
#endif


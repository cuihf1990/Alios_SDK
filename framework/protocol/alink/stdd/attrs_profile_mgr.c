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
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "accs.h"
#include "alink_protocol.h"
#include "stdlib.h"
#include "json_parser.h"
#include "connectivity_manager.h"
#include "digest_algorithm.h"
#include "device.h"
#include "list.h"
#include "log.h"
#include "os.h"
#include "service_manager.h"
#include "wsf/wsf.h"
#include "work_queue.h"
#include "kvmgr.h"
#include "attrs_profile_mgr.h"
#include "http.h"
#include "devmgr.h"
#include "stdd_lua.h"
#include "stdd.h"

extern int devmgr_get_all_device_modelid(uint32_t model_id[], int *num);

static int check_attrs_profile_directory(void);

static int attrs_profile_download(struct work_struct *work);

static int load_default_attrs_profile(void);

static int32_t alink_get_attrs_profile(dlist_t *info_head);
static int attrs_profile_listener(int type, void *data, int dlen, void *result,
                                  int *rlen);
static int attrs_profile_sync(struct work_struct *work);
int set_attrs_profile(char *request);

static dlist_t g_list;
static void *g_attrs_mutex = NULL;

#define DOWNLOAD_WORK_POLLING_CYCLE    (10 * 1000)
#define TIMING_SYNC_CYCLE       (30 * 60 * 1000)
#define MAX_DOWNLOAD_FILES      3

static struct work_struct attrs_profile_download_work = {
    .func = (work_func_t) &attrs_profile_download,
    .prio = DOWNLOAD_WORK_PRIO,
    .name = "attrs profile download work",
};

/*定时同步任务*/
static struct work_struct sync_work = {
    .func = (work_func_t) &attrs_profile_sync,
    .prio = DOWNLOAD_WORK_PRIO,
    .name = "attrs profile sync work",
};


static void free_node(pfnode_t *node)
{
    if (node->url) {
        os_free(node->url);
    }
    os_free(node);
}

static void free_global_list(void)
{
    pfnode_t *node, *n;

    os_mutex_lock(g_attrs_mutex);
    list_for_each_entry_safe_t(node, n, &g_list, list_node, pfnode_t) {
        list_del(&(node->list_node));
        free_node(node);
    }
    os_mutex_unlock(g_attrs_mutex);
}

int attrs_profile_init(void)
{
    list_init_head(&g_list);
    g_attrs_mutex = os_mutex_init();

    sm_attach_service("accs", &attrs_profile_listener);
    queue_work(&attrs_profile_download_work);
    queue_work(&sync_work);

    check_attrs_profile_directory();
    load_default_attrs_profile();

    return 0;
}

int attrs_profile_exit(void)
{
    cancel_work(&sync_work);
    cancel_work(&attrs_profile_download_work);

    free_global_list();

    sm_detach_service("accs", &attrs_profile_listener);
    if (g_attrs_mutex) {
        os_mutex_destroy(g_attrs_mutex);
        g_attrs_mutex = NULL;
    }
}

static int attrs_profile_listener(int type, void *data, int dlen, void *result,
                                  int *rlen)
{
    if (type == SERVICE_EVENT) {
        int st = *((int *)data);

        log_info("Attr Profile recv %s, %s", sm_code2string(type), sm_code2string(st));
        if (*(int *)data == SERVICE_STATE_READY) {
            queue_delayed_work(&sync_work, 0);
        }
    } else if (type == SERVICE_DATA) {
        alink_data_t *p = (alink_data_t *) data;

        if (!strcmp(p->method, "setAttributeProfile")) {
            set_attrs_profile(p->data);
            return EVENT_CONSUMED;
        } else {
            return EVENT_IGNORE;
        }
    }
    return EVENT_IGNORE;
}

static char *get_attrs_profile_path_prefix(char dir[STR_LONG_LEN])
{
    snprintf(dir, STR_LONG_LEN, "%s%s/", os_get_storage_directory(),
             PROFILE_DIR_NAME);
    return dir;
}

static char *get_attrs_profile_path(char *file_name, char *file_path,
                                    unsigned max_path_len)
{
    char *path = NULL;
    file_path[0] = '\0';

    int len = strlen(get_attrs_profile_path_prefix(file_path));
    snprintf(file_path + len, max_path_len - len, "%s", file_name);

    return file_path;
}

static int check_attrs_profile_directory(void)
{
    DIR *d;
    char path[STR_LONG_LEN] = { 0 };

    get_attrs_profile_path_prefix(path);
    if (!(d = opendir(path))) {
        if (0 != mkdir(path, 0777) || !(d = opendir(path))) {
            log_error("error opendir %s!!!", path);
            return -1;
        }
    }
    return 0;
}

static int is_file_exist(char *file)
{
    char path[STR_LONG_LEN] = { 0 };
    get_attrs_profile_path(file, path, sizeof(path));

    return access(path, F_OK);
}

int attrs_profile_download_each(struct pfnode *node)
{
    int ret = SERVICE_RESULT_OK;
    struct profile_info info = node->info;
    char *filepath, *model, md5[MD5_LEN];

    filepath = (char *)os_malloc(STR_LONG_LEN);
    model = (char *)os_malloc(OS_PRODUCT_MODEL_LEN);
    OS_CHECK_MALLOC(filepath && model);
    memset(filepath, 0, STR_LONG_LEN);

    get_attrs_profile_path(info.filename, filepath, STR_LONG_LEN);

    log_info("Download %s.", filepath);

    ret = download_file(node->url, filepath);
    RET_LOG(ret, CALL_FUCTION_FAILED, "download_file");
    if (!ret) {
        digest_md5_file(filepath, md5);
        if (!strcmp(info.md5, md5)) {
            memset(model, 0, OS_PRODUCT_MODEL_LEN);
            os_product_get_model(model);
            if (!strncmp(info.model, model, OS_PRODUCT_MODEL_LEN)) {
                stdd_update_global_profile(filepath);
                log_info("download global attrs profile success");
                ret = set_kv_in_flash(info.model, (char *)&info, sizeof(info), 1);
            } else {
                stdd_update_device_profile(DEV_TYPE_ZIGBEE, info.model_id, filepath);
                snprintf(model, OS_PRODUCT_MODEL_LEN, "%08x", info.model_id);
                log_info("download  subdeivce 0x%08x success");
                ret = set_kv_in_flash(model, (char *)&info, sizeof(info), 1);
            }
        } else {
            log_warn("downloading file %s md5 check failed", info.filename);
            ret = -1;
        }
    }
    os_free(model);
    os_free(filepath);
    return ret;
}

static int attrs_profile_download(struct work_struct *work)
{
    int ret = SERVICE_RESULT_OK;
    struct pfnode *node, *n;

    LIST_HEAD(info_head);
    int i = 0, cycle = DOWNLOAD_WORK_POLLING_CYCLE;

    if (!cloud_is_connected()) {
        queue_delayed_work(&attrs_profile_download_work, cycle);
        log_info("cloud isn't connected");
        return SERVICE_RESULT_ERR;
    }

    os_mutex_lock(g_attrs_mutex);
    list_for_each_entry_safe_t(node, n, &g_list, list_node, pfnode_t) {
        os_mutex_unlock(g_attrs_mutex);
        if (i >= MAX_DOWNLOAD_FILES) {
            break;
        }
        if (node->url == NULL) {
            os_mutex_lock(g_attrs_mutex);
            list_add(&node->url_node, &info_head);
            os_mutex_unlock(g_attrs_mutex);
            log_info("model_id %08x md5 %s model %s", node->info.model_id, node->info.md5,
                     node->info.model);
        } else {
            ret = attrs_profile_download_each(node);
            if (!ret) {
                os_mutex_lock(g_attrs_mutex);
                list_del(&(node->list_node));
                os_mutex_unlock(g_attrs_mutex);
                free_node(node);
                /*
                 * in case of missing some work(added by other thread),
                 * queue a realtime work.
                 * Note: queue a realtime work only when download success,
                 * otherwise bad-work cause a dead loop.
                 */
                cycle = 0;
            }
        }
        i++;
        os_mutex_lock(g_attrs_mutex);
    }
    os_mutex_unlock(g_attrs_mutex);

    if (i > 0 && alink_get_attrs_profile(&info_head) == SERVICE_RESULT_OK) {
        ret = SERVICE_RESULT_OK;
        log_info("sync profile file success");

        if (!list_empty(&g_list)) {
            cycle = 0;
        }
    }

    queue_delayed_work(&attrs_profile_download_work, cycle);
    return ret;
}

static int load_subdev_attrs_profile_array_each(char *array_obj, int obj_len)
{
    int ret = SERVICE_RESULT_ERR;
    char *pos, *path;
    int len;
    char filename[STR_LONG_LEN] = { 0 };
    struct profile_info info;

    path = (char *)os_malloc(STR_LONG_LEN);
    OS_CHECK_MALLOC(path);
    memset((char *)&info, 0, sizeof(struct profile_info));

    pos = json_get_value_by_name(array_obj, obj_len, "file_name", &len, 0);
    strncpy(filename, pos, (sizeof(filename) - 1 > len) ? len : (sizeof(
                                                                     filename) - 1));

    char *short_model_set = json_get_value_by_name(array_obj, obj_len,
                                                   "modelid_set", &len, 0);
    char *entry;
    int entry_len, type;

    char back_chr = 0;
    backup_json_str_last_char(short_model_set, len, back_chr);
    json_array_for_each_entry(short_model_set, pos, entry, entry_len, type) {
        int info_size = sizeof(struct profile_info);
        char key[16] = { 0 };
        uint32_t shortmodel = 0;
        sscanf(entry, "%x", &shortmodel);
        snprintf(key, sizeof(key) - 1, "%08x", shortmodel);
        log_info("filename %s, model_id 0x%08x", filename, shortmodel);

        ret = get_kv(key, (char *)&info, &info_size);
        if (ret != SERVICE_RESULT_OK) {
            info.model_id = shortmodel;
            strncpy(info.filename, filename, sizeof(filename));
            if (!is_file_exist(info.filename)) {
                get_attrs_profile_path(info.filename, path, STR_LONG_LEN);
                digest_md5_file(path, info.md5);
                char key[16] = { 0 };
                snprintf(key, sizeof(key) - 1, "%08x", info.model_id);
                ret = set_kv_in_flash(key, (char *)&info, sizeof(struct profile_info), 1);
            }
        }
    }
    restore_json_str_last_char(short_model_set, len, back_chr);

    os_free(path);
    return ret;
}

static int load_default_attrs_profile(void)
{
    int ret = SERVICE_RESULT_ERR;
    struct profile_info info;
    char *shortmodel, *model_name, *file_path, *pos;
    int len;
    char *value = NULL;

    model_name = (char *)os_malloc(OS_PRODUCT_MODEL_LEN);
    file_path = (char *)os_malloc(STR_LONG_LEN);
    OS_CHECK_MALLOC(model_name && file_path);
    memset((char *)&info, 0x00, sizeof(struct profile_info));
    memset(model_name, 0, OS_PRODUCT_MODEL_LEN);

    os_product_get_model(model_name);
    strncpy(info.model, model_name, OS_PRODUCT_MODEL_LEN);

    void *L = stdd_lua_open();
    get_attrs_profile_path("profile_config.lua", file_path, STR_LONG_LEN);

    ret = stdd_lua_load_file(L, file_path);
    if (ret != SERVICE_RESULT_OK) {
        log_warn("profile_config.lua isn't exist");
        goto exit;
    }
    ret = stdd_lua_get_global_variable(L, "profile_config", &value);
    if (ret != SERVICE_RESULT_OK) {
        log_warn("load profile config failed");
        goto exit;
    }

    info.model_id = 0;
    len = sizeof(struct profile_info);
    ret = get_kv(model_name, (char *)&info, &len);
    if (ret != SERVICE_RESULT_OK) {
        pos = json_get_value_by_name(value, strlen(value), "global_profile", &len, 0);
        if (NULL != pos) {
            strncpy(info.filename, pos, len);
            log_info("model_name %s filename %s", model_name, info.filename);
            if (!is_file_exist(info.filename)) {
                get_attrs_profile_path(info.filename, file_path, STR_LONG_LEN);
                digest_md5_file(file_path, info.md5);
                ret = set_kv_in_flash(model_name, (char *)&info, sizeof(struct profile_info),
                                      1);
            }
        }
    }

    char *profile_set = json_get_value_by_name(value, strlen(value),
                                               "device_profile", &len, 0);
    if (NULL != profile_set) {
        char *entry;
        int entry_len, type;

        char back_chr = 0;
        backup_json_str_last_char(profile_set, len, back_chr);
        json_array_for_each_entry(profile_set, pos, entry, entry_len, type) {
            load_subdev_attrs_profile_array_each(entry, entry_len);
        }
        restore_json_str_last_char(profile_set, len, back_chr);
    }

exit:
    os_free(model_name);
    os_free(file_path);
    stdd_lua_close(L);

    return ret;
}

int add_node_to_list(pfnode_t *pfnode)
{
    int ret = SERVICE_RESULT_ERR;
    int exist = 0;
    struct pfnode *node, *n;

    os_mutex_lock(g_attrs_mutex);
    list_for_each_entry_safe_t(node, n, &g_list, list_node, struct pfnode) {
        if (node->info.model_id == pfnode->info.model_id ||
            (pfnode->info.model[0] != '\0' &&
             strcmp(pfnode->info.model, node->info.model) == 0)) {
            exist = 1;
            break;
        }
    }

    if (!exist) {
        list_add(&pfnode->list_node, &g_list);
        ret = SERVICE_RESULT_OK;
    }
    os_mutex_unlock(g_attrs_mutex);

    if (!exist) {
        queue_delayed_work(&attrs_profile_download_work, 0);
    }

    return ret;
}


static int get_attrs_profile_file_md5(char *file_name, char *md5_buff,
                                      unsigned buff_size)
{
    int ret = SERVICE_RESULT_ERR;
    char md5_file[MD5_LEN] = { '\0' };
    char file_path[STR_LONG_LEN];

    if (is_file_exist(file_name) == 0) {
        get_attrs_profile_path(file_name, file_path, STR_LONG_LEN);
        digest_md5_file(file_path, md5_file);
        strncpy(md5_buff, md5_file, buff_size - 1);

        log_info("profile %s md5:%s", file_name, md5_buff);
        ret = SERVICE_RESULT_OK;
    } else {
        log_info("profile %s is not exist", file_name);
    }

    return ret;
}

int get_global_profile(char *file_name, int max_name_length)
{
    int ret = SERVICE_RESULT_ERR;
    pfnode_t *node;
    int len = sizeof(struct profile_info);
    char *model_name;

    node = (pfnode_t *) os_malloc(sizeof(pfnode_t));
    model_name = (char *)os_malloc(OS_PRODUCT_MODEL_LEN);
    OS_CHECK_MALLOC(node && model_name);

    memset(model_name, 0, OS_PRODUCT_MODEL_LEN);
    os_product_get_model(model_name);

    memset((char *)node, 0, sizeof(pfnode_t));
    list_init_head(&node->url_node);
    list_init_head(&node->list_node);

    ret = get_kv(model_name, (char *)&node->info, &len);
    if (ret == SERVICE_RESULT_OK && is_file_exist(node->info.filename) == 0) {
        log_info("global profile is exist");
        get_attrs_profile_path(node->info.filename, file_name, max_name_length);
    } else {
        log_info("update global profile");
        memset(node->info.md5, 0, MD5_LEN);
        node->info.model_id = 0;
        strncpy(node->info.model, model_name, OS_PRODUCT_MODEL_LEN);
        if (add_node_to_list(node) == SERVICE_RESULT_OK) {
            node = NULL;
        } else {
            log_info("global device have added to list");
        }

        ret = SERVICE_RESULT_ERR;
    }

    if (model_name) {
        os_free(model_name);
    }
    if (node) {
        free_node(node);
    }

    return ret;
}

int get_device_profile_file(uint8_t dev_type, uint32_t model_id,
                            char *file_name, int max_name_length)
{
    int ret = SERVICE_RESULT_ERR;
    pfnode_t *node;
    int len = sizeof(struct profile_info);
    char shortmodel[16], *prefix;

    node = (pfnode_t *) os_malloc(sizeof(pfnode_t));
    OS_CHECK_MALLOC(node);
    memset((char *)node, 0, sizeof(pfnode_t));

    snprintf(shortmodel, sizeof(shortmodel), "%08x", model_id);
    ret = get_kv(shortmodel, (char *)&node->info, &len);

    if (ret == SERVICE_RESULT_OK && is_file_exist(node->info.filename) == 0) {
        log_info("profile 0x%x is exist", model_id);
        get_attrs_profile_path(node->info.filename, file_name, max_name_length);
    } else {
        log_info("update subdevice 0x%08x profile", model_id);
        node->info.model_id = model_id;
        memset(node->info.md5, 0, MD5_LEN);
        if (add_node_to_list(node) == SERVICE_RESULT_OK) {
            node = NULL;
        } else {
            log_info("subdevice 0x%x have added to list", model_id);
        }
        ret = SERVICE_RESULT_ERR;
    }
    if (node) {
        free_node(node);
    }

    return ret;
}

int sync_global_profile()
{
    int ret = SERVICE_RESULT_OK;
    pfnode_t *node;
    int len = sizeof(struct profile_info);
    char *model_name;

    node = (pfnode_t *) os_malloc(sizeof(pfnode_t));
    model_name = (char *)os_malloc(OS_PRODUCT_MODEL_LEN);
    OS_CHECK_MALLOC(node && model_name);

    memset((char *)node, 0, sizeof(struct pfnode));
    memset(model_name, 0, OS_PRODUCT_MODEL_LEN);
    os_product_get_model(model_name);
    strncpy(node->info.model, model_name, OS_PRODUCT_MODEL_LEN);
    if (get_kv(model_name, (char *)&node->info, &len) == SERVICE_RESULT_OK) {
        get_attrs_profile_file_md5(node->info.filename, node->info.md5,
                                   sizeof(node->info.md5));
        node->info.model_id = 0;
    }

    if (add_node_to_list(node) == SERVICE_RESULT_OK) {
        node = NULL;
    } else {
        log_info("add global prfile to sync list");
        ret = SERVICE_RESULT_ERR;
    }

    if (model_name) {
        os_free(model_name);
    }
    if (node) {
        free_node(node);
    }

    return ret;
}

int sync_device_profile(uint8_t dev_type, uint32_t model_id)
{
    int ret = SERVICE_RESULT_OK;
    pfnode_t *node;
    int len = sizeof(struct profile_info);
    char shortmodel[16], *prefix;
    char file_name[STR_LONG_LEN] = { 0 };

    node = (pfnode_t *) os_malloc(sizeof(pfnode_t));
    OS_CHECK_MALLOC(node);
    memset((char *)node, 0, sizeof(struct pfnode));

    node->info.model_id = model_id;
    snprintf(shortmodel, sizeof(shortmodel), "%08x", model_id);
    if (get_kv(shortmodel, (char *)&node->info, &len) == SERVICE_RESULT_OK) {
        node->info.md5[0] = '\0';
        get_attrs_profile_file_md5(node->info.filename, node->info.md5,
                                   sizeof(node->info.md5));
    }

    if (add_node_to_list(node) == SERVICE_RESULT_OK) {
        node = NULL;
    } else {
        log_info("add subdevice 0x%x profile to sync list", model_id);
    }

    if (node) {
        free_node(node);
    }

    return ret;
}

static int attrs_profile_download_request_each(char *params, int str_len)
{
    int ret = SERVICE_RESULT_ERR;
    char *str_pos, *url, *shortmodel;
    struct profile_info *info;
    struct pfnode *node, *pos, *next;
    int len, new_node = 1;

    url = (char *)os_malloc(MAX_URL_LEN);
    shortmodel = (char *)os_malloc(STR_SHORT_LEN);
    node = (struct pfnode *)os_malloc(sizeof(struct pfnode));
    OS_CHECK_MALLOC(url && shortmodel && node);
    memset(url, 0, MAX_URL_LEN);
    memset(shortmodel, 0, STR_SHORT_LEN);
    memset((char *)node, 0, sizeof(struct pfnode));

    info = &node->info;
    str_pos = json_get_value_by_name(params, str_len, "fileName", &len, NULL);
    PTR_GOTO(str_pos, exit, "get filename fail, params:%s", params);
    strncpy(info->filename, str_pos, len);

    str_pos = json_get_value_by_name(params, str_len, "url", &len, NULL);
    PTR_GOTO(str_pos, exit, "get url fail, params:%s", params);
    strncpy(url, str_pos, len);

    str_pos = json_get_value_by_name(params, str_len, "model", &len, NULL);
    PTR_LOG(str_pos, "get model fail, params:%s", params);
    if (str_pos != NULL) {
        strncpy(info->model, str_pos, len);
    }

    str_pos = json_get_value_by_name(params, str_len, "shortModel", &len, NULL);
    if (str_pos != NULL) {
        strncpy(shortmodel, str_pos, len);
        sscanf(shortmodel, "%08x", &info->model_id);
    } else {
        info->model_id = 0;
    }

    str_pos = json_get_value_by_name(params, str_len, "md5", &len, NULL);
    PTR_GOTO(str_pos, exit, "get md5 fail, params:%s", params);
    strncpy(info->md5, str_pos, len);

    os_mutex_lock(g_attrs_mutex);
    list_for_each_entry_safe_t(pos, next, &g_list, list_node, pfnode_t) {
        if (pos->info.model_id == info->model_id ||
            (pos->info.model_id == 0 && strcmp(pos->info.model, info->model) == 0)) {

            if (pos->url) {
                os_free(pos->url);
            }
            pos->url = url;
            strcpy(pos->info.md5, info->md5);
            strcpy(pos->info.filename, info->filename);

            log_info("url: %s", url);
            new_node = 0;
            break;
        }
    }
    os_mutex_unlock(g_attrs_mutex);

    if (new_node == 1) {
        node->url = url;
        os_mutex_lock(g_attrs_mutex);
        list_add(&node->list_node, &g_list);
        os_mutex_unlock(g_attrs_mutex);
    } else {
        free_node(node);
    }
    /* kick the work */
    queue_delayed_work(&attrs_profile_download_work, 0);
    os_free(shortmodel);
    ret = SERVICE_RESULT_OK;

    return ret;
exit:
    os_free(url);
    return ret;
}

static int attrs_profile_download_request_hander(char *data)
{
    int ret = SERVICE_RESULT_OK;
    char *pos, *entry;
    int len, type;

    log_trace("data: %s", data);
    json_array_for_each_entry(data, pos, entry, len, type) {
        attrs_profile_download_request_each(entry, len);
    }

    return ret;
}

static int attrs_profile_sync(struct work_struct *work)
{
    int ret = SERVICE_RESULT_OK;
    uint32_t model_id[MAX_MODEL];
    int i, num = MAX_MODEL - 1;

    ret = devmgr_get_all_device_modelid(model_id, &num);
    RET_RETURN(ret, CALL_FUCTION_FAILED, "devmgr_get_all_device_modelid");

    for (i = 0; i < num; i++) {
        log_trace("shortMode:0x%08x", model_id[i]);
        ret = sync_device_profile(DEV_TYPE_ZIGBEE, model_id[i]);
        if (ret == SERVICE_RESULT_ERR) {
            log_info("sync device:0x%08x profile fail", model_id[i]);
            continue;
        }
        log_info("sync device:0x%08x profile later", model_id[i]);
    }

    ret = sync_global_profile();
    if (ret == SERVICE_RESULT_ERR) {
        log_trace("sync global profile fail");
    } else {
        log_trace("sync global profile later");
    }

    queue_delayed_work(work, TIMING_SYNC_CYCLE);

    return ret;
}

int set_attrs_profile(char *request)
{
    int ret = SERVICE_RESULT_OK, code;
    char *data, *str_pos;
    int len, num;

    str_pos = json_get_value_by_name(request, strlen(request), "data", &len, NULL);
    PTR_RETURN(str_pos, SERVICE_RESULT_ERR, "get filename fail, params:%s",
               request);

    if (len == 0 || (data = os_malloc(strlen(request))) == NULL) {
        return SERVICE_RESULT_ERR;
    }

    memset(data, 0, strlen(request));
    strncpy(data, str_pos, len);

    log_debug("request: %s", data);

    if (json_get_array_size(data, strlen(data)) == 0) {
        os_free(data);
        log_info("no file need update");
    } else {
        attrs_profile_download_request_hander(data);
    }
    os_free(data);
    return SERVICE_RESULT_OK;
}

static int32_t alink_get_attrs_profile(dlist_t *info_head)
{
    int ret = SERVICE_RESULT_ERR;
    char *params = NULL;
    char *out = NULL;
    int len = 0;
    pfnode_t *pos, *next;

    if (list_empty(info_head)) {
        return ret;
    }

    params = os_malloc(ALINK_PARAMS_SIZE);
    OS_CHECK_MALLOC(params);
    memset(params, 0, ALINK_PARAMS_SIZE);

    len += snprintf(params + len, ALINK_PARAMS_SIZE - len, "[");
    list_for_each_entry_t(pos, info_head, url_node, pfnode_t) {
        if (pos->info.model_id == 0) {
            len += snprintf(params + len, ALINK_PARAMS_SIZE - len,
                            "{\"type\":\"zigbee_global\",\"model\":\"%s\",\"md5\":\"%s\"}",
                            pos->info.model, pos->info.md5);
        } else {
            len += snprintf(params + len, ALINK_PARAMS_SIZE - len,
                            "{\"type\":\"zigbee\",\"shortModel\":\"%08x\",\"md5\":\"%s\"}",
                            pos->info.model_id, pos->info.md5);
        }

        if (info_head != pos->url_node.next) {
            len += snprintf(params + len, ALINK_PARAMS_SIZE - len, ",");
        }
    }
    len += snprintf(params + len, ALINK_PARAMS_SIZE - len, "]");
    if (len == ALINK_PARAMS_SIZE) {
        log_error("buffer size too small");
        goto out;
    }

    out = os_malloc(ALINK_PARAMS_SIZE);
    OS_CHECK_MALLOC(out);
    memset(out, 0, ALINK_PARAMS_SIZE);
    alink_data_t data = { "device.getProfileFiles", params };
    ret = ((service_t *) sm_get_service("accs"))->get((void *)&data, strlen(params),
                                                      out, ALINK_PARAMS_SIZE);
    if (ret == SERVICE_RESULT_OK) {
        if (json_get_array_size(out, strlen(out)) > 0) {
            attrs_profile_download_request_hander(out);
        }
        //remove node which needn't to sync
        os_mutex_lock(g_attrs_mutex);
        list_for_each_entry_safe_t(pos, next, info_head, url_node, pfnode_t) {
            list_del(&(pos->url_node));
            list_init_head(&(pos->url_node));

            if (pos->url == NULL) {
                list_del(&(pos->list_node));
                free_node(pos);
            }
        }
        os_mutex_unlock(g_attrs_mutex);
    } else {
        log_error("Sync profile files from cloud failed, err:%d", ret);
        log_error("Sync request: %s", params);
    }

out:
    if (params) {
        os_free(params);
    }
    if (out) {
        os_free(out);
    }

    return ret;
}

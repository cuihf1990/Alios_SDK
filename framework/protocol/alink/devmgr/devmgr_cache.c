#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "list.h"
#include "mpool.h"
#include "devmgr.h"
#include "devmgr_cache.h"

#define ATTR_MPOOL_SIZE     2

typedef struct attr_cache_s{
    struct list_head list_node;
    char *attr_name;
    char *attr_value;
}attr_cache_t;

static mpool_t *cache_mpool = NULL;
static void *__cache_dup_string(const char *src)
{
    return pstrdup(cache_mpool, src);
}

static void *__cache_new_buff(unsigned int buff_size)
{
    void *buff = pmalloc(cache_mpool, buff_size);
    if(NULL != buff)
        memset(buff, 0, buff_size);

    return buff;
}

static void *__cache_renew_buff(char *src, uint32_t new_size)
{
    return premalloc(cache_mpool, src, new_size);
}

static void __cache_free_buff(void *buff)
{
    if(buff)
        pfree(cache_mpool, buff);
}

static attr_cache_t *__new_attr_node(const char *attr_name, const char *attr_value)
{
    attr_cache_t *attr_node = __cache_new_buff(sizeof(attr_cache_t));
    PTR_GOTO(attr_node, err, "pstrdup error");
    memset(attr_node, 0, sizeof(attr_cache_t));

    attr_node->attr_name = __cache_dup_string(attr_name);
    PTR_GOTO(attr_node->attr_name, err, "pstrdup error");

    attr_node->attr_value = __cache_dup_string(attr_value);
    PTR_GOTO(attr_node->attr_value, err, "pstrdup error");

    return attr_node;

err:
    if(attr_node != NULL){
        if(attr_node->attr_name)
            __cache_free_buff(attr_node->attr_name);

        if(attr_node->attr_value)
            __cache_free_buff(attr_node->attr_value);

        __cache_free_buff((char *)attr_node);
    }

    return NULL;
}


static void __free_attr_node(attr_cache_t *cache)
{
    if(cache != NULL){
        if(cache->attr_name)
            __cache_free_buff(cache->attr_name);

        if(cache->attr_value)
            __cache_free_buff(cache->attr_value);

        __cache_free_buff((char *)cache);
    }
}


static int __get_attr_cache(struct list_head *attr_head, const char *attr_name, char *attr_value_buff, int buff_size)
{
    attr_cache_t *attr_node = NULL;
    char *buff = NULL;

    list_for_each_entry_t(attr_node, attr_head, list_node, attr_cache_t){
        if(strcmp(attr_node->attr_name, attr_name) == 0){
            if(buff_size <= strlen(attr_node->attr_value))
                return SERVICE_BUFFER_INSUFFICENT;

            strncpy(attr_value_buff, attr_node->attr_value, buff_size);
            return SERVICE_RESULT_OK;
        }
    }

    return SERVICE_RESULT_ERR;
}


static int __read_attr_cache(struct list_head *attr_head, const char *attr_name, char **attr_value)
{
    attr_cache_t *attr_node = NULL;
    char *buff = NULL;

    list_for_each_entry_t(attr_node, attr_head, list_node, attr_cache_t){
        if(strcmp(attr_node->attr_name, attr_name) == 0){
            buff = os_malloc(strlen(attr_node->attr_value) + 1);
            PTR_RETURN(buff, SERVICE_RESULT_ERR, "pmalloc failed");

            strncpy(buff, attr_node->attr_value, strlen(attr_node->attr_value) + 1);
            *attr_value = buff;

            return SERVICE_RESULT_OK;
        }
    }

    return SERVICE_RESULT_ERR;
}


static int __update_attr_cache(struct list_head *attr_head, const char *attr_name, const char *attr_value)
{
    attr_cache_t *attr_node = NULL;
    char *buff = NULL;

    list_for_each_entry_t(attr_node, attr_head, list_node, attr_cache_t){
        if(strcmp(attr_node->attr_name, attr_name) == 0){
            uint32_t buff_size = pbuff_size(attr_node->attr_value);
            if(buff_size < strlen(attr_value) + 1)
                attr_node->attr_value = __cache_renew_buff(attr_node->attr_value, strlen(attr_value) + 1);

            strncpy(attr_node->attr_value, attr_value, strlen(attr_value) + 1);

            return SERVICE_RESULT_OK;
        }
    }

    attr_node = __new_attr_node(attr_name, attr_value);
    PTR_RETURN(attr_node, SERVICE_RESULT_ERR, CALL_FUCTION_FAILED, "__new_attr_node")

    list_add_tail(&attr_node->list_node, attr_head);

    return SERVICE_RESULT_OK;
}


static void __free_attr_cache_list(struct list_head *cache_head)
{
    attr_cache_t *cache, *next;
    list_for_each_entry_safe_t(cache, next, cache_head, list_node, attr_cache_t) {
        __free_attr_node(cache);
    }
}


void __dump_attr_cache(list_head_t *attr_head)
{
    os_printf("\tattr cache:\n");
    attr_cache_t *cache;
    list_for_each_entry_t(cache, attr_head, list_node, attr_cache_t){
        os_printf("\t\t%s: %s\n", cache->attr_name, cache->attr_value);
    }
}

/*读取子设备属性缓存*/
int devmgr_get_attr_cache(const char *devid_or_uuid, const char *attr_name, char *attr_value_buff, int buff_size)
{
    int ret = SERVICE_RESULT_ERR;

    dev_info_t *devinfo = devmgr_get_devinfo(devid_or_uuid);
    PTR_RETURN(devinfo, SERVICE_RESULT_ERR, CALL_FUCTION_FAILED, "devmgr_get_devinfo");

    os_mutex_lock(devinfo->dev_mutex);
    ret = __get_attr_cache(&devinfo->attr_head, attr_name, attr_value_buff, buff_size);
    os_mutex_unlock(devinfo->dev_mutex);
    devmgr_put_devinfo_ref(devinfo);

    return ret;
}


/*读取子设备属性缓存*/
int devmgr_read_attr_cache(const char *devid_or_uuid, const char *attr_name, char **attr_value)
{
    int ret = SERVICE_RESULT_ERR;

    dev_info_t *devinfo = devmgr_get_devinfo(devid_or_uuid);
    PTR_RETURN(devinfo, SERVICE_RESULT_ERR, CALL_FUCTION_FAILED, "devmgr_get_devinfo");

    os_mutex_lock(devinfo->dev_mutex);
    ret = __read_attr_cache(&devinfo->attr_head, attr_name, attr_value);
    os_mutex_unlock(devinfo->dev_mutex);
    devmgr_put_devinfo_ref(devinfo);

    return ret;
}


/*更新子设备属性缓存*/
int devmgr_update_attr_cache(const char *devid_or_uuid, const char *attr_name, const char *attr_value)
{
    int ret = SERVICE_RESULT_ERR;

    dev_info_t *devinfo = devmgr_get_devinfo(devid_or_uuid);
    PTR_RETURN(devinfo, ret, CALL_FUCTION_FAILED, "devmgr_get_devinfo")

    os_mutex_lock(devinfo->dev_mutex);
    ret = __update_attr_cache(&devinfo->attr_head, attr_name, attr_value);
    os_mutex_unlock(devinfo->dev_mutex);

    devmgr_put_devinfo_ref(devinfo);

    return ret;
}


void devmgr_free_device_cache(dev_info_t *devinfo)
{
    __free_attr_cache_list(&devinfo->attr_head);
}


int devmgr_cache_init()
{
    cache_mpool = mpool_create("attr", ATTR_MPOOL_SIZE);
    if(NULL == cache_mpool)
        return SERVICE_RESULT_ERR;

    return SERVICE_RESULT_OK;
}


void devmgr_cache_exit()
{
    if(cache_mpool){
        mpool_destroy(cache_mpool);
        cache_mpool = NULL;
    }
    return;
}


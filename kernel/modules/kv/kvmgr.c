/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "yos/log.h"
#include "yos/framework.h"
#include "kvmgr.h"
#include "digest_algorithm.h"
#include "crc.h"
#include "hashtable.h"
#include "yos/kernel.h"
#include "k_api.h"
#include "yos/cli.h"

/* KV header include fsize part(4bytes) and crc part (4bytes)
   |       KV Header       |
   |  kv_fize  |    crc    | ---> kv_file: 4 bytes; crc: 4 bytes
*/
#define KV_CRC_LENGTH   4
#define KV_FSIZE_LENGTH 4
#define KV_HEADER_SIZE (KV_FSIZE_LENGTH + KV_CRC_LENGTH)

#define INT2BYTE(p,value) \
    *p ++ = value & 0xff, \
*p ++ = (value >> 8) & 0xff, \
*p ++ = (value >> 16) & 0xff, \
*p ++ = (value >> 24) & 0xff

#define BYTE2INT(p, value) \
    value = (*p ++ & 0xff), \
value |= ((*p ++ << 8) & 0xff00), \
value |= ((*p ++ << 16) & 0xff0000), \
value |= ((*p ++ << 24) & 0xff000000)

static void *g_ht = NULL;
static int save_key_value();

typedef struct {
    char *key;
    char *val;
    uint16_t len_val;
    uint8_t sync;
} kv_item_t;


#define MODULE_NAME_KV "kv"

static void handle_kv_cmd(char *pwbuf, int blen, int argc, char **argv)
{
    const char *rtype = argc > 1 ? argv[1] : "";
    int ret = 0;
    if (strcmp(rtype, "set") == 0) {
        if (argc != 4)
            return ;
        ret = yos_kv_set(argv[2], argv[3], strlen(argv[3]), 1);
        if (ret != 0)
            LOGW(MODULE_NAME_KV, "cli set kv failed");
        else
            LOGD(MODULE_NAME_KV, "cli set kv success");
    } else if (strcmp(rtype, "get") == 0) {
        if (argc != 3)
            return ;
        char buf[KV_BUFFER_SIZE] = {0};
        int len = sizeof(buf);
        ret = yos_kv_get(argv[2], buf, &len);
        if (ret != 0)
            LOGW(MODULE_NAME_KV, "cli get kv failed");
        else
            LOGD(MODULE_NAME_KV, "value is %s", buf);
    } else if (strcmp(rtype, "del") == 0) {
        if (argc != 3)
            return;
        ret = yos_kv_del(argv[2]);
        if (ret != 0)
            LOGD(MODULE_NAME_KV, "cli kv del failed");
        else
            LOGW(MODULE_NAME_KV, "cli kv del success");
    }
    return;
}


static struct cli_command ncmd = {
    .name = "kv",
    .help = "kv [set key value | get key | del key]",
    .function = handle_kv_cmd,
};


static kv_item_t *new_kv_item(const char *skey, const char *cvalue, int nlength,
                              int sync)
{
    kv_item_t *new = NULL;

    if (!skey || !cvalue) {
        return NULL;
    }

    new = yos_malloc(sizeof(kv_item_t));
    if (!new) {
        return NULL;
    }
    memset(new, 0, sizeof(kv_item_t));
    new->key = yos_malloc(strlen(skey) + 1);
    if (!new->key) {
        yos_free(new);
        return NULL;
    }
    memcpy(new->key, skey, strlen(skey) + 1);

    new->val = yos_malloc(nlength);
    if (!new->val) {
        yos_free(new->key);
        yos_free(new);
        return NULL;
    }
    memcpy(new->val, cvalue, nlength);
    new->len_val = nlength;
    new->sync = sync;

    return new;
}

static int hash_table_insert(const char *skey, const char *cvalue, int nlength,
                             int sync)
{
    kv_item_t *new = NULL;
    int ret = -1;
    kv_item_t *item = NULL;
    void *found = NULL;
    if (strlen(skey) > MAX_KV_LEN || nlength > MAX_KV_LEN) {
        LOGI(MODULE_NAME_KV, "key/value too long!");
        return ret;
    }

    found = ht_find_lockless(g_ht, skey, strlen(skey) + 1, NULL, NULL);
    if (found) { //replace the repeated value with the current.
        item = *((kv_item_t **)found);
        yos_free(item->val);
        item->val = NULL;
        item->len_val = nlength;
        item->val = yos_malloc(nlength);
        if (!item->val) {
            yos_free(item->key);
            yos_free(item);
            return -1;
        }
        memcpy(item->val, cvalue, nlength);
        item->sync = sync;
        return (sync == 0 ? 0 : save_key_value());
    }

    new = new_kv_item(skey, cvalue, nlength, sync);

    if (!new) {
        return ret;
    }

    ret = ht_add_lockless(g_ht, skey, strlen(skey) + 1, &new, sizeof(new));
    if (0 == ret && 1 == sync) {
        ret = save_key_value();
    }

    //LOGD(MODULE_NAME_KV,"sel kv, key: %s, %p-%p-%p\n",skey,new,new->key,new->val);
    return ret;
}

static int load_kvfile(const char *file, char *buffer, int buffer_len)
{
    int fsize = 0;
    uint32_t header_size = 0, kvdata_size = 0;
    int fd = -1;
    uint32_t crc32_value;
    char *kv_header, *p;

    kv_header = (char *)yos_malloc(KV_HEADER_SIZE);
    if(!kv_header) {
        goto exit;
    }

    fd = yos_open(file, O_RDONLY);
    if (fd < 0) {
        LOGI(MODULE_NAME_KV, "%s not exist", file);
        goto exit;
    }

    if (!(header_size = yos_read(fd, kv_header, KV_HEADER_SIZE))) {
        LOGI(MODULE_NAME_KV, "read KVheader failed");
        goto exit;
    }

    p = kv_header;
    BYTE2INT(p, kvdata_size);

    if (!kvdata_size) {
        LOGI(MODULE_NAME_KV, "KV file is empty");
        goto exit;
    } else if (kvdata_size > KV_BUFFER_SIZE - KV_HEADER_SIZE) {
        LOGI(MODULE_NAME_KV, "KV header is invalid");
        goto exit;
    }

    BYTE2INT(p, crc32_value);

    /* reopen the kvfile to clear the offset in file struct */
    yos_close(fd);
    fd = yos_open(file, O_RDONLY);

    if (!(fsize = yos_read(fd, buffer, kvdata_size + KV_HEADER_SIZE))) {
        LOGI(MODULE_NAME_KV, "read KVfile failed");
        goto exit;
    }

    p = buffer + KV_HEADER_SIZE;
    if (crc32_value != utils_crc32((uint8_t *)p, kvdata_size)) {
        fsize = 0;
        goto exit;
    }

    LOGD(MODULE_NAME_KV, "load %s KV success, file size %d", file, fsize);

exit:
    if (fd >= 0) {
        yos_close(fd);
    }
    if (kv_header) {
        yos_free(kv_header);
    }
    return fsize;
}

static int update_kvfile(const char *file, char *buffer, int filelen)
{
    int ret = -1;

    int fd = yos_open(file, O_WRONLY);
    if (fd < 0) {
        LOGI(MODULE_NAME_KV, "open %s failed", file);
        goto exit;
    }

    if (!yos_write(fd, buffer, filelen)) {
        LOGI(MODULE_NAME_KV, "write %s failed", file);
    } else {
        ret = 0;
    }

exit:
    if (fd >= 0) {
        yos_close(fd);
    }
    return ret;
}

static int restore_kvfile(const char *src_file, const char *dst_file)
{
    int fd_src = -1, fd_dst = -1;
    char *buffer = NULL;
    int fsize = KV_BUFFER_SIZE, ret = -1;

    fd_src = yos_open(src_file, O_RDWR);
    if (fd_src < 0) {
        LOGI(MODULE_NAME_KV, "open %s failed", src_file);
        goto exit;
    }

    buffer = (char *)yos_malloc(fsize);
    if (!buffer) {
        goto exit;
    }

    fd_dst = yos_open(dst_file, O_WRONLY);
    if (fd_dst < 0) {
        LOGI(MODULE_NAME_KV, "open %s failed", dst_file);
        goto exit;
    }
    ret = yos_read(fd_src, buffer, fsize);

    if (ret > 0) {
        fsize = ret;
        if ((ret = yos_write(fd_dst, buffer, fsize)) == fsize) {
            ret = 0;
            LOGI(MODULE_NAME_KV, "restore key/value success");
        }
    } else {
        ret = -1;
    }

exit:
    if (buffer) {
        yos_free(buffer);
    }
    if (fd_src >= 0) {
        yos_close(fd_src);
    }
    if (fd_dst >= 0) {
        yos_close(fd_dst);
    }

    return ret;
}

static int check_file_same(const char *src_file, const char *dst_file)
{
    char md5_src[33], md5_dst[33];
    int ret = -1, fsize = 0;
    char *kv_buffer;

    memset(md5_src, 0, sizeof(md5_src));
    memset(md5_dst, 0, sizeof(md5_dst));

    kv_buffer = (char *)yos_malloc(KV_BUFFER_SIZE);
    if (!kv_buffer) {
        goto exit;
    }

    fsize = load_kvfile(src_file, kv_buffer, KV_BUFFER_SIZE);

    if (digest_md5(kv_buffer, fsize, (uint8_t *)md5_src) != 0) {
        LOGI(MODULE_NAME_KV, "getting the MD5 of %s is failed", src_file);
        goto exit;
    }

    memset(kv_buffer, 0, KV_BUFFER_SIZE);
    fsize = load_kvfile(dst_file, kv_buffer, KV_BUFFER_SIZE);

    if (digest_md5(kv_buffer, fsize, (uint8_t *)md5_dst) != 0) {
        LOGI(MODULE_NAME_KV, "getting the MD5 of %s is failed", dst_file);
        goto exit;
    }

    if (!strncmp(md5_src, md5_dst, sizeof(md5_src))) {
        LOGI(MODULE_NAME_KV, "the files(KVfile, KVfile_backup) are same");
        ret = 0;
    }

exit:
    if (kv_buffer) {
        yos_free(kv_buffer);
    }
    return ret;
}

typedef struct {
    char *p;
    int len;
    int ret;
} kv_storeage_t;
static void *__get_kv_inflash_cb(void *key, void *val, void *extra)
{
    kv_item_t *item = NULL;
    kv_storeage_t *store = extra;
    int len = 0;

    item = *((kv_item_t **)val);

    if (store->len + 4 + strlen(item->key) + 4 + item->len_val + 4 >=
        KV_BUFFER_SIZE) {
        store->ret = -1;
        return NULL;
    }

    len = strlen(item->key);
    INT2BYTE(store->p, len);
    store->len += 4;

    memcpy(store->p, item->key, len);
    store->len += len;
    store->p += len;

    INT2BYTE(store->p, item->len_val);
    store->len += 4;
    memcpy(store->p, item->val, item->len_val);

    store->p += item->len_val;
    store->len += item->len_val;

    store->ret = 0;
    return NULL;
}

static int save_key_value()
{
    char *kv_buffer, *p;
    kv_storeage_t store;
    uint32_t fsize = 0;

    store.p = (char *)yos_malloc(KV_BUFFER_SIZE);
    if (!store.p) {
        return -1;
    }

    kv_buffer = store.p;
    store.len = 0;
    store.p += KV_HEADER_SIZE;
    store.ret = 0;
    ht_iterator_lockless(g_ht, __get_kv_inflash_cb, &store);

    fsize = store.len;
    p = kv_buffer + KV_HEADER_SIZE;

    uint32_t crc32_value = utils_crc32((uint8_t *)p, store.len);
    p -= KV_CRC_LENGTH;
    INT2BYTE(p, crc32_value);
    store.len += KV_CRC_LENGTH;

    p -= KV_HEADER_SIZE;
    INT2BYTE(p, fsize);
    store.len += KV_FSIZE_LENGTH;

    if (!update_kvfile(KVFILE_NAME, kv_buffer, store.len)) {
        update_kvfile(KVFILE_NAME_BACKUP, kv_buffer, store.len);
    }
    yos_free(kv_buffer);
    return store.ret;
}

static int load_key_value(const char *file)
{
    char *key, *value, *p, *kv_buffer;
    int fsize, len;
    int ret = -1;

    kv_buffer = (char *)yos_malloc(KV_BUFFER_SIZE);
    if (!kv_buffer) {
        return -1;
    }


    key = (char *)yos_malloc(MAX_KV_LEN);
    if (!key) {
        yos_free(kv_buffer);
        return -1;
    }
    value = (char *)yos_malloc(MAX_KV_LEN);
    if (!value) {
        yos_free(kv_buffer);
        yos_free(key);
        return -1;
    }

    fsize = load_kvfile(file, kv_buffer, KV_BUFFER_SIZE);
    if (fsize == 0) {
        LOGI(MODULE_NAME_KV, "read kvfile failed");
        goto exit;
    }

    p = kv_buffer + KV_HEADER_SIZE;
    fsize -= KV_HEADER_SIZE;

    ht_lock(g_ht);
    while (fsize) {
        BYTE2INT(p, len);
        memcpy(key, p, len);
        key[len] = '\0';
        p += len;
        fsize -= (len + 4);
        BYTE2INT(p, len);
        memcpy(value, p, len);
        p += len;
        fsize -= (len + 4);
        ret = hash_table_insert(key, value, len, 0);
    }
    ht_unlock(g_ht);
exit:
    if (key) {
        yos_free(key);
    }
    if (value) {
        yos_free(value);
    }
    if (kv_buffer) {
        yos_free(kv_buffer);
    }

    return ret;
}

int yos_kv_init()
{
    int ret = -1;
    if (g_ht) {
        return 0;
    }

    cli_register_command(&ncmd);
    g_ht = ht_init(HASH_TABLE_MAX_SIZE);

    if ((ret = load_key_value(KVFILE_NAME)) != 0) {
        LOGI(MODULE_NAME_KV, "load backup key/value file");
        if ((ret = load_key_value(KVFILE_NAME_BACKUP)) == 0) {
            ret = restore_kvfile(KVFILE_NAME_BACKUP, KVFILE_NAME);
        }
    } else {
        if ((ret = check_file_same(KVFILE_NAME, KVFILE_NAME_BACKUP)) != 0) {
            ret = restore_kvfile(KVFILE_NAME, KVFILE_NAME_BACKUP);
        }
    }
    return ret;
}

static void *__del_all_kv_cb(void *key, void *val, void *extra)
{
    kv_item_t *item = NULL;

    item = *((kv_item_t **)val);

    LOGD(MODULE_NAME_KV, "del kv, key: %s, %p-%p-%p\n", key, item, item->key,
         item->val);
    yos_free(item->val);
    yos_free(item->key);
    yos_free(item);

    return NULL;
}

void yos_kv_deinit()
{
    if (!g_ht) {
        return;
    }

    ht_lock(g_ht);
    ht_iterator_lockless(g_ht, __del_all_kv_cb, NULL);
    ht_unlock(g_ht);
    ht_destroy(g_ht);
    g_ht = NULL;
}

int yos_kv_set(const char *key, const void *value, int len, int sync)
{
    int ret = 0;

    if (!key || !value || len <= 0) {
        return -1;
    }
    ht_lock(g_ht);
    ret = hash_table_insert(key, value, len, sync);
    ht_unlock(g_ht);

    return ret;
}

int yos_kv_get(const char *key, void *buffer, int *buffer_len)
{
    kv_item_t *item = NULL;
    void *ret = NULL;

    if (!key || !buffer || !buffer_len || *buffer_len <= 0) {
        return -1;
    }


    ret = ht_find(g_ht, key, strlen(key) + 1, NULL, NULL);
    if (!ret) {
        return -1;
    }

    item = *((kv_item_t **)ret);
    if (*buffer_len < item->len_val) {
        *buffer_len = item->len_val;
        return -1;
    } else {
        memcpy(buffer, item->val, item->len_val);
        *buffer_len = item->len_val;
    }

    //LOGD(MODULE_NAME_KV,"gel kv, key: %s, %p-%p-%p\n",key,item,item->key,item->val);
    return 0;
}

int yos_kv_del(const char *key)
{
    kv_item_t *item = NULL;
    void *ret = NULL;

    if (!key) {
        return -1;
    }


    ht_lock(g_ht);
    ret = ht_find_lockless(g_ht, key, strlen(key) + 1, NULL, NULL);
    if (!ret) {
        ht_unlock(g_ht);
        return -1;
    }

    item = *((kv_item_t **)ret);

    LOGD(MODULE_NAME_KV, "del kv, key: %s, %p-%p-%p\n", key, item, item->key,
         item->val);
    yos_free(item->val);
    yos_free(item->key);
    yos_free(item);
    ht_del_lockless(g_ht, key, strlen(key) + 1);
    save_key_value();
    ht_unlock(g_ht);

    return 0;
}


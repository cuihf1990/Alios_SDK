#include "yos/log.h"
#include "os.h"
#include "kvmgr.h"
#include "digest_algorithm.h"
#include "crc.h"
#include "hashtable.h"

#define KV_BUFFER_SIZE  (128 * 1024)
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
static void *g_mutex = NULL;
static void save_key_value();

typedef struct{
    char *key;
    char *val;
    uint16_t len_val; 
    uint8_t sync;
}kv_item_t;


#define MODULE_NAME_KV "kv"

static kv_item_t *new_kv_item(const char *skey, const char *cvalue, int nlength, int sync)
{
    kv_item_t *new = NULL;

    if(!skey || !cvalue)
        return NULL;

    new = os_malloc(sizeof(kv_item_t));
    if(!new)
        return NULL;
    memset(new,0,sizeof(kv_item_t));
    new->key = os_malloc(strlen(skey)+1);
    if(!new->key){
        os_free(new);
        return NULL;
    }
    memcpy(new->key,skey,strlen(skey)+1);

    new->val = os_malloc(nlength);
    if(!new->val){
        os_free(new->key);
        os_free(new);
        return NULL;
    }
    memcpy(new->val,cvalue,nlength);
    new->len_val = nlength;
    new->sync = sync;

    return new;
}

static int hash_table_insert(const char *skey, const char *cvalue, int nlength, int sync)
{
    kv_item_t *new = NULL;
    int ret = -1;

    if (strlen(skey) > MAX_KV_LEN || nlength > MAX_KV_LEN) {
        LOGI(MODULE_NAME_KV,"key/value too long!");
        return ret;
    }

    new = new_kv_item(skey,cvalue,nlength,sync);

    if(!new)
        return ret;

    ret = ht_add_lockless(g_ht,skey,strlen(skey)+1,&new,sizeof(new)); 
    if (0 == ret && 1 == sync)
        save_key_value();

    LOGD(MODULE_NAME_KV,"sel kv, key: %s, %p-%p-%p\n",skey,new,new->key,new->val);
    return ret;
}

static int load_kvfile(const char *file, char *buffer, int buffer_len)
{
    int fsize = 0;
    char *filepath;
    FILE *fd;

    filepath = (char *)os_malloc(STR_LONG_LEN);
    OS_CHECK_MALLOC(filepath);
    snprintf(filepath, STR_LONG_LEN, "%s/%s", KVFILE_PATH, file);

    fd = fopen(filepath, "r+");
    if (!fd) {
        LOGI(MODULE_NAME_KV,"%s not exist", file);
        goto exit;
    }
    fseek(fd, 0L, SEEK_END);
    fsize = ftell(fd);
    fseek(fd, 0L, SEEK_SET);
    if (fsize > buffer_len || fsize == 0) {
        LOGI(MODULE_NAME_KV,"file size too large or file null");
        goto exit;
    }

    if (!(fsize = fread(buffer, 1, fsize, fd))) {
        LOGI(MODULE_NAME_KV,"read KVfile failed");
        goto exit;
    }
    LOGI(MODULE_NAME_KV,"file size %d", fsize);

exit:
    if (fd)
        fclose(fd);
    if (filepath)
        os_free(filepath);

    return fsize;
}

static int update_kvfile(const char *file, char *buffer, int filelen)
{
    char *filepath;
    int ret = -1;

    filepath = (char *)os_malloc(STR_LONG_LEN);
    OS_CHECK_MALLOC(filepath);
    snprintf(filepath, STR_LONG_LEN, "%s/%s", KVFILE_PATH, file);

    FILE *fd = fopen(filepath, "w+");
    if (!fd) {
        LOGI(MODULE_NAME_KV,"open %s failed", file);
        goto exit;
    }

    if (!fwrite(buffer, 1, filelen, fd)) {
        LOGI(MODULE_NAME_KV,"write %s failed", file);
    } else {
        ret = 0;
    }

exit:
    if (fd)
        fclose(fd);
    if (filepath)
        os_free(filepath);
    return ret;
}

static int restore_kvfile(const char *src_file, const char *dst_file)
{
    FILE *fd_src = NULL, *fd_dst = NULL;
    char *filepath = NULL, *buffer = NULL;
    int fsize, ret = -1;

    filepath = (char *)os_malloc(STR_LONG_LEN);
    OS_CHECK_MALLOC(filepath);
    snprintf(filepath, STR_LONG_LEN, "%s/%s", KVFILE_PATH, src_file);

    fd_src = fopen(filepath, "r+");
    if (!fd_src) {
        LOGI(MODULE_NAME_KV,"open %s failed", src_file);
        goto exit;
    }
    fseek(fd_src, 0L, SEEK_END);
    fsize = ftell(fd_src);
    fseek(fd_src, 0L, SEEK_SET);

    buffer = (char *)os_malloc(fsize);
    OS_CHECK_MALLOC(buffer);

    snprintf(filepath, STR_LONG_LEN, "%s/%s", KVFILE_PATH, dst_file);
    fd_dst = fopen(filepath, "w+");
    if (!fd_dst) {
        LOGI(MODULE_NAME_KV,"open %s failed", dst_file);
        goto exit;
    }

    ret = fread(buffer, 1, fsize, fd_src);

    if (ret == fsize) {
        if ((ret = fwrite(buffer, 1, fsize, fd_dst)) == fsize) {
            ret = 0;
            LOGI(MODULE_NAME_KV,"restore key/value success");
        }
    } else {
        ret = -1;
    }

exit:
    if (filepath)
        os_free(filepath);
    if (buffer)
        os_free(buffer);
    if (fd_src)
        fclose(fd_src);
    if (fd_dst)
        fclose(fd_dst);

    return ret;
}

static int check_file_same(const char *src_file, const char *dst_file)
{
    char *filepath;
    char md5_src[33], md5_dst[33];
    int ret = -1;

    filepath = (char *)os_malloc(STR_LONG_LEN);
    OS_CHECK_MALLOC(filepath);
    snprintf(filepath, STR_LONG_LEN, "%s/%s", KVFILE_PATH, src_file);

    memset(md5_src, 0, sizeof(md5_src));
    memset(md5_dst, 0, sizeof(md5_dst));
    if (digest_md5_file(filepath, (uint8_t *)md5_src) != 0) {
        LOGI(MODULE_NAME_KV,"getting the MD5 of file %s is failed", filepath);
        goto exit;
    }

    snprintf(filepath, STR_LONG_LEN, "%s/%s", KVFILE_PATH, dst_file);
    if (digest_md5_file(filepath, (uint8_t *)md5_dst) != 0) {
        LOGI(MODULE_NAME_KV,"getting the MD5 of file %s is failed", filepath);
        goto exit;
    }

    if (!strncmp(md5_src, md5_dst, sizeof(md5_src))) {
        LOGI(MODULE_NAME_KV,"the files(KVfile, KVfile_backup) are same");
        ret = 0;
    }

exit:
    if (filepath)
        os_free(filepath);

    return ret;
}

typedef struct{
    char *p;
    int len;
}kv_storeage_t;
static void *__get_kv_inflash_cb(void *key, void *val, void *extra)
{
    kv_item_t *item = NULL;
    kv_storeage_t *store = extra;
    int len = 0;

    item = *((kv_item_t **)val);

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

    return NULL;
}

static void save_key_value()
{
    char *kv_buffer,*p;
    kv_storeage_t store;

    store.p = (char *)os_malloc(KV_BUFFER_SIZE);
    OS_CHECK_MALLOC(store.p);

    kv_buffer = store.p;
    store.len = 0; 
    store.p += 4; 
    ht_iterator_lockless(g_ht,__get_kv_inflash_cb,&store); 

    p = kv_buffer + 4;
    uint32_t crc32_value = utils_crc32((uint8_t *)p, store.len);
    p -= 4;
    INT2BYTE(p, crc32_value);
    store.len += 4;

    if (!update_kvfile(KVFILE_NAME, kv_buffer, store.len)) {
        update_kvfile(KVFILE_NAME_BACKUP, kv_buffer, store.len);
    }
    os_free(kv_buffer);
}

static int load_key_value(const char *file)
{
    char *key, *value, *p, *kv_buffer;
    int fsize, len;
    uint32_t crc32_value;
    int ret = -1;

    kv_buffer = (char *)os_malloc(KV_BUFFER_SIZE);
    key = (char *)os_malloc(MAX_KV_LEN);
    value = (char *)os_malloc(MAX_KV_LEN);
    OS_CHECK_MALLOC(kv_buffer && key && value);

    fsize = load_kvfile(file, kv_buffer, KV_BUFFER_SIZE);
    if (fsize == 0) {
        LOGI(MODULE_NAME_KV,"read kvfile failed");
        goto exit;
    }

    p = kv_buffer;
    BYTE2INT(p, crc32_value);
    fsize -= 4;
    if (crc32_value != utils_crc32((uint8_t *)p, fsize)) {
        LOGI(MODULE_NAME_KV,"KV file is not complete");
        goto exit;
    }

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
    if(key)
        os_free(key);
    if(value)
        os_free(value);
    if(kv_buffer)
        os_free(kv_buffer);

    return ret;
}

int yos_kv_init()
{
    int ret = -1;

    if(g_ht)
        return ret;

    g_ht = ht_init(HASH_TABLE_MAX_SIZE); 

    if ((ret = load_key_value(KVFILE_NAME)) != 0) {
        LOGI(MODULE_NAME_KV,"load backup key/value file");
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

    LOGD(MODULE_NAME_KV,"del kv, key: %s, %p-%p-%p\n",key,item,item->key,item->val);
    os_free(item->val);
    os_free(item->key);
    os_free(item);
    
    return NULL;
}

void yos_kv_deinit()
{
    if(!g_ht)
        return;
    
    ht_lock(g_ht);
    ht_iterator_lockless(g_ht,__del_all_kv_cb,NULL); 
    ht_unlock(g_ht);
    ht_destroy(g_ht);
    g_ht = NULL;
}

int yos_kv_set(const char *key, const void *value, int len, int sync)
{
    int ret = 0;

    if (!key || !value || len <= 0)
        return -1;
    ht_lock(g_ht);
    ret = hash_table_insert(key, value, len, sync);
    ht_unlock(g_ht); 
    
    return ret; 
}

int yos_kv_get(const char *key, char *buffer, int *buffer_len)
{
    kv_item_t *item = NULL;
    void *ret = NULL;

    ret = ht_find(g_ht,key,strlen(key)+1,NULL,NULL); 
    if(!ret)
        return -1;

    item = *((kv_item_t **)ret);
    if (*buffer_len < item->len_val) {
        *buffer_len = item->len_val;
        return -1;
    } else {
        memcpy(buffer, item->val, item->len_val);
        *buffer_len = item->len_val;
    }

    LOGD(MODULE_NAME_KV,"gel kv, key: %s, %p-%p-%p\n",key,item,item->key,item->val);
    return 0;
}

int yos_kv_del(const char *key)
{
    kv_item_t *item = NULL;
    void *ret = NULL;

    ht_lock(g_ht);
    ret = ht_find_lockless(g_ht,key,strlen(key)+1,NULL,NULL); 
    if(!ret){
        ht_unlock(g_ht); 
        return -1;
    }

    item = *((kv_item_t **)ret);

    LOGD(MODULE_NAME_KV,"del kv, key: %s, %p-%p-%p\n",key,item,item->key,item->val);
    os_free(item->val);
    os_free(item->key);
    os_free(item);
    ht_del_lockless(g_ht,key,strlen(key)+1);
    ht_unlock(g_ht);
}


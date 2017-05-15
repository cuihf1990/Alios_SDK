#include "service.h"
#include "log.h"
#include "os.h"
#include "log.h"
#include "os.h"
#include "kvmgr.h"
#include "crc.h"
#include "digest_algorithm.h"

#define KV_BUFFER_SIZE  (128 * 1024)
#define KVFILE_NAME "KVfile"
#define KVFILE_NAME_BACKUP "KVfile_backup"

char *alink_base64_encode_alloc(void *data, int len);
static HashNode *hashTable[HASH_TABLE_MAX_SIZE] = { NULL };

static int hash_table_size;
static void *kv_mutex = NULL;

static void hash_table_init();
static int hash_table_insert(const char *skey, const char *cvalue, int len, int inflash, int sync);
static int hash_table_remove(const char *skey);
static HashNode *hash_table_lookup(const char *skey);
static void hash_table_print();
static void hash_table_release();
static void save_key_value();

void hash_table_init()
{
    hash_table_size = 0;
    memset(hashTable, 0, sizeof(HashNode *) * HASH_TABLE_MAX_SIZE);
}

unsigned int hash_table_hash_str(const char *skey)
{
    const signed char *p = (const signed char *)skey;
    unsigned int h = *p;
    if (h) {
        for (p += 1; *p != '\0'; ++p)
            h = (h << 5) - h + *p;
    }
    return h;
}

int hash_table_insert(const char *skey, const char *cvalue, int nlength, int inflash, int sync)
{
    if (strlen(skey) > MAX_KV_LEN || nlength > MAX_KV_LEN) {
        LOGI("key/value too long!");
        return SERVICE_RESULT_ERR;
    }

    if (hash_table_size >= HASH_TABLE_MAX_SIZE) {
        LOGI("out of hash table memory!");
        return SERVICE_RESULT_ERR;
    }
    unsigned int pos = hash_table_hash_str(skey) % HASH_TABLE_MAX_SIZE;

    platform_mutex_lock(kv_mutex);
    HashNode *pHead = hashTable[pos];
    while (pHead) {
        if (strcmp(pHead->sKey, skey) == 0) {
            if (pHead->nLength == nlength && !memcmp(pHead->cValue, cvalue, nlength)) {
                LOGI("%s already exists!", skey);
                platform_mutex_unlock(kv_mutex);
                return SERVICE_RESULT_OK;
            } else {
                if (pHead->cValue)
                    os_free(pHead->cValue);

                pHead->cValue = (char *)os_malloc(nlength);
                if (pHead->cValue == NULL) {
                    platform_mutex_unlock(kv_mutex);
                    return SERVICE_RESULT_ERR;
                }
                pHead->nLength = nlength;
                memcpy(pHead->cValue, cvalue, nlength);
                if (sync == 1)
                    save_key_value();
                platform_mutex_unlock(kv_mutex);
                return SERVICE_RESULT_OK;
            }
        }
        pHead = pHead->pNext;
    }

    HashNode *pNewNode = (HashNode *) os_malloc(sizeof(HashNode));
    memset(pNewNode, 0, sizeof(HashNode));
    pNewNode->sKey = (char *)os_malloc(sizeof(char) * (strlen(skey) + 1));
    strcpy(pNewNode->sKey, skey);

    pNewNode->cValue = (char *)os_malloc(nlength);
    if (pNewNode->cValue == NULL) {
        platform_mutex_unlock(kv_mutex);
        return SERVICE_RESULT_ERR;
    }
    memcpy(pNewNode->cValue, cvalue, nlength);
    pNewNode->nLength = nlength;
    pNewNode->inflash = inflash;
    pNewNode->pNext = hashTable[pos];
    hashTable[pos] = pNewNode;

    hash_table_size++;

    if (sync == 1)
        save_key_value();
    platform_mutex_unlock(kv_mutex);

    return SERVICE_RESULT_OK;
}

int hash_table_remove(const char *skey)
{
    unsigned int pos = hash_table_hash_str(skey) % HASH_TABLE_MAX_SIZE;

    platform_mutex_lock(kv_mutex);
    if (hashTable[pos]) {
        HashNode *pHead = hashTable[pos];
        HashNode *pLast = NULL;
        HashNode *pRemove = NULL;
        while (pHead) {
            if (strcmp(skey, pHead->sKey) == 0) {
                pRemove = pHead;
                break;
            }
            pLast = pHead;
            pHead = pHead->pNext;
        }
        if (pRemove) {
            if (pLast)
                pLast->pNext = pRemove->pNext;
            else
                hashTable[pos] = pRemove->pNext;

            hash_table_size--;
            os_free(pRemove->sKey);
            os_free(pRemove->cValue);
            os_free(pRemove);
            pRemove = NULL;
        } else {
            LOGI("key/value no exists");
            platform_mutex_unlock(kv_mutex);
            return SERVICE_RESULT_ERR;
        }
    }

    save_key_value();
    platform_mutex_unlock(kv_mutex);

    return SERVICE_RESULT_OK;
}

HashNode *hash_table_lookup(const char *skey)
{
    unsigned int pos = hash_table_hash_str(skey) % HASH_TABLE_MAX_SIZE;

    platform_mutex_lock(kv_mutex);

    if (hashTable[pos]) {
        HashNode *pHead = hashTable[pos];
        while (pHead) {
            if (strcmp(skey, pHead->sKey) == 0) {
                platform_mutex_unlock(kv_mutex);
                return pHead;
            }
            pHead = pHead->pNext;
        }
    }
    platform_mutex_unlock(kv_mutex);
    return NULL;
}

void hash_table_print()
{
    int i;

    platform_mutex_lock(kv_mutex);
    for (i = 0; i < HASH_TABLE_MAX_SIZE; ++i) {
        if (hashTable[i]) {
            HashNode *pHead = hashTable[i];
            while (pHead) {
                pHead = pHead->pNext;
            }
        }
    }
    platform_mutex_unlock(kv_mutex);
}

static int load_kvfile(const char *file, char *buffer, int buffer_len)
{
    int fsize = 0;
    char *filepath;
    FILE *fd;

    filepath = (char *)os_malloc(STR_LONG_LEN);
    OS_CHECK_MALLOC(filepath);
    snprintf(filepath, STR_LONG_LEN, "%s/%s", os_get_storage_directory(), file);

    fd = fopen(filepath, "r+");
    if (!fd) {
        LOGI("%s not exist", file);
        goto exit;
    }
    fseek(fd, 0L, SEEK_END);
    fsize = ftell(fd);
    fseek(fd, 0L, SEEK_SET);
    if (fsize > buffer_len || fsize == 0) {
        LOGI("file size too large or file null");
        goto exit;
    }

    if (!(fsize = fread(buffer, 1, fsize, fd))) {
        LOGI("read KVfile failed");
        goto exit;
    }
    LOGI("file size %d", fsize);

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
    snprintf(filepath, STR_LONG_LEN, "%s/%s", os_get_storage_directory(), file);

    FILE *fd = fopen(filepath, "w+");
    if (!fd) {
        LOGI("open %s failed", file);
        goto exit;
    }

    if (!fwrite(buffer, 1, filelen, fd)) {
        LOGI("write %s failed", file);
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
    snprintf(filepath, STR_LONG_LEN, "%s/%s", os_get_storage_directory(), src_file);

    fd_src = fopen(filepath, "r+");
    if (!fd_src) {
        LOGI("open %s failed", src_file);
        goto exit;
    }
    fseek(fd_src, 0L, SEEK_END);
    fsize = ftell(fd_src);
    fseek(fd_src, 0L, SEEK_SET);

    buffer = (char *)os_malloc(fsize);
    OS_CHECK_MALLOC(buffer);

    snprintf(filepath, STR_LONG_LEN, "%s/%s", os_get_storage_directory(), dst_file);
    fd_dst = fopen(filepath, "w+");
    if (!fd_dst) {
        LOGI("open %s failed", dst_file);
        goto exit;
    }

    ret = fread(buffer, 1, fsize, fd_src);

    if (ret == fsize) {
        if ((ret = fwrite(buffer, 1, fsize, fd_dst)) == fsize) {
            ret = 0;
            LOGI("restore key/value success");
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
    snprintf(filepath, STR_LONG_LEN, "%s/%s", os_get_storage_directory(), src_file);

    memset(md5_src, 0, sizeof(md5_src));
    memset(md5_dst, 0, sizeof(md5_dst));
    if (digest_md5_file(filepath, (uint8_t *)md5_src) != 0) {
        LOGI("getting the MD5 of file %s is failed", filepath);
        goto exit;
    }

    snprintf(filepath, STR_LONG_LEN, "%s/%s", os_get_storage_directory(), dst_file);
    if (digest_md5_file(filepath, (uint8_t *)md5_dst) != 0) {
        LOGI("getting the MD5 of file %s is failed", filepath);
        goto exit;
    }

    if (!strncmp(md5_src, md5_dst, sizeof(md5_src))) {
        LOGI("the files(KVfile, KVfile_backup) are same");
        ret = 0;
    }

exit:
    if (filepath)
        os_free(filepath);

    return ret;
}

static void save_key_value()
{
    int i, len = 0, filelen = 0;
    char *p, *kv_buffer;

    kv_buffer = (char *)os_malloc(KV_BUFFER_SIZE);
    OS_CHECK_MALLOC(kv_buffer);

    p = kv_buffer + 4;
    for (i = 0; i < HASH_TABLE_MAX_SIZE; ++i) {
        if (hashTable[i]) {
            HashNode *pHead = hashTable[i];
            while (pHead && pHead->inflash == INFLASH) {
                len = strlen(pHead->sKey);
                INT2BYTE(p, len);
                filelen += 4;
                memcpy(p, pHead->sKey, len);
                filelen += len;
                p += len;
                INT2BYTE(p, pHead->nLength);
                filelen += 4;
                memcpy(p, pHead->cValue, pHead->nLength);
                p += pHead->nLength;
                filelen += pHead->nLength;
                pHead = pHead->pNext;
            }
        }
    }

    p = kv_buffer + 4;
    uint32_t crc32_value = utils_crc32((uint8_t *)p, filelen);
    p = kv_buffer;
    INT2BYTE(p, crc32_value);
    filelen += 4;

    if (!update_kvfile(KVFILE_NAME, kv_buffer, filelen)) {
        update_kvfile(KVFILE_NAME_BACKUP, kv_buffer, filelen);
    }
    os_free(kv_buffer);
    return;
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
        LOGI("read kvfile failed");
        goto exit;
    }

    p = kv_buffer;
    BYTE2INT(p, crc32_value);
    fsize -= 4;
    if (crc32_value != utils_crc32((uint8_t *)p, fsize)) {
        LOGI("KV file is not complete");
        goto exit;
    }

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
        ret = hash_table_insert(key, value, len, INFLASH, 0);
    }
    hash_table_print();

exit:
    if(key)
        os_free(key);
    if(value)
        os_free(value);
    if(kv_buffer)
        os_free(kv_buffer);

    return ret;
}

//os_free the memory of the hash table
void hash_table_release()
{
    int i;

    platform_mutex_lock(kv_mutex);
    for (i = 0; i < HASH_TABLE_MAX_SIZE; ++i) {
        if (hashTable[i]) {
            HashNode *pHead = hashTable[i];
            while (pHead) {
                HashNode *pTemp = pHead;
                pHead = pHead->pNext;
                if (pTemp) {
                    os_free(pTemp->sKey);
                    os_free(pTemp->cValue);
                    os_free(pTemp);
                }

            }
        }
    }
    platform_mutex_unlock(kv_mutex);
}

#define MAX_STR_LEN 20
#define MIN_STR_LEN 10

void rand_str(char r[])
{
    int i;
    int len = MIN_STR_LEN + rand() % (MAX_STR_LEN - MIN_STR_LEN);
    for (i = 0; i < len - 1; ++i)
        r[i] = 'a' + rand() % ('z' - 'a');
    r[len - 1] = '\0';
}

int rand_value(char r[])
{
    int i;
    int len = MIN_STR_LEN + rand() % (MAX_STR_LEN - MIN_STR_LEN);
    for (i = 0; i < len - 1; ++i) {
        r[i] = rand() % 0x7f;
    }
    return len - 1;
}

static char a2x(char ch)
{
    switch (ch) {
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case 'A':
        case 'a':
            return 10;
        case 'B':
        case 'b':
            return 11;
        case 'C':
        case 'c':
            return 12;
        case 'D':
        case 'd':
            return 13;
        case 'E':
        case 'e':
            return 14;
        case 'F':
        case 'f':
            return 15;
        default:
            break;;
    }
    return 0;
}

void *alink_base64_decode_alloc(const char *str, int *len)
{
    int i = 0;
    int str_len = strlen(str);
    char *buf = (char *)os_malloc(str_len * sizeof(char));

    while (i <= str_len) {
        buf[i / 2] = (a2x(str[i]) << 4) | a2x(str[i + 1]);
        i += 2;
    }
    buf[i / 2] = '\0';
    *len = i / 2;

    return buf;
}

static char base64_buffer[512];

char *alink_base64_encode_alloc(void *data, int len)
{
    int i;
    unsigned char *ptr = (unsigned char *)data;

    for (i = 0; i < len; i++) {
        sprintf(base64_buffer + 2 * i, "%02X", ptr[i]);
    }
    base64_buffer[len * 2] = '\0';

    return base64_buffer;
}

void alink_base64_release(void *data)
{
    os_free(data);
}

int init_kv()
{
    int ret = -1;

    kv_mutex = platform_mutex_init();
    hash_table_init();

    if ((ret = load_key_value(KVFILE_NAME)) != 0) {
        LOGI("load backup key/value file");
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

void deinit_kv()
{
    hash_table_release();
    platform_mutex_destroy(kv_mutex);
}

int set_kv_in_flash(const char *key, const void *value, int len, int sync)
{
    if (len <= 0)
        return SERVICE_RESULT_ERR;
    return hash_table_insert(key, value, len, INFLASH, sync);
}

int get_kv(const char *key, char *buffer, int *buffer_len)
{
    HashNode *node;

    node = hash_table_lookup(key);
    if (node == NULL || *buffer_len < node->nLength) {
        return SERVICE_RESULT_ERR;
    } else {
        memcpy(buffer, node->cValue, node->nLength);
        *buffer_len = node->nLength;
    }
    return SERVICE_RESULT_OK;
}

int remove_kv(const char *key)
{
    return hash_table_remove(key);
}

int set_kv_in_ram(const char *key, const void *value, int len)
{
    if (len <= 0)
        return SERVICE_RESULT_ERR;
    return hash_table_insert(key, value, len, INRAM, 0);
}

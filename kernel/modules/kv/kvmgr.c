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

#include "yos/cli.h"
#include "yos/log.h"
#include "yos/kernel.h"
#include "yos/framework.h"
#include "hal/soc/flash.h"
#include "kvmgr.h"
#include <string.h>

/* Key-value function return code description */
typedef enum {
    RES_OK = 0,             /* (0) Successed */
    RES_CONT,               /* (1) Loop continued */
    RES_SEM_ERR,            /* (2) Error related to semaphore */
    RES_WORK_ERR,           /* (3) Error related to work (workqueue) */
    RES_MUTEX_ERR,          /* (4) Error related to mutex */
    RES_NO_SPACE,           /* (5) The space is out of range */
    RES_INVALID_PARAM,      /* (6) The parameter is invalid */
    RES_INVALID_HEADER,     /* (7) The item/block header is invalid */
    RES_MALLOC_FAILED,      /* (8) Error related to malloc */
    RES_ITEM_NOT_FOUND,     /* (9) Could not find the key-value item */
    RES_FLASH_READ_ERR,     /* (10) The flash read operation failed */
    RES_FLASH_WRITE_ERR,    /* (11) The flash write operation failed */
    RES_FLASH_EARSE_ERR     /* (12) The flash earse operation failed */
}result_e;

/* Defination of block information */
#define BLK_BITS                12                          /* The number of bits in block size */
#define BLK_SIZE                (1 << BLK_BITS)             /* Block size, current is 4k bytes */
#define BLK_NUMS                (KV_TOTAL_SIZE >> BLK_BITS) /* The number of blocks */
#define BLK_OFF_MASK            ~(BLK_SIZE - 1)             /* The mask of block offset in key-value store */
#define BLK_STATE_USED          0xCC                        /* Block state: USED --> block is inused and without dirty data */
#define BLK_STATE_CLEAN         0xEE                        /* Block state: CLEAN --> block is clean, ready for used */
#define BLK_STATE_DIRTY         0x44                        /* Block state: DIRTY --> block is inused and with dirty data */
#define BLK_HEADER_SIZE         4                           /* The block header size 4bytes */

/* Defination of key-value item information */
#define ITEM_HEADER_SIZE        8                           /* The key-value item header size 8bytes */
#define ITEM_STATE_NORMAL       0xEE                        /* Key-value item state: NORMAL --> the key-value item is valid */
#define ITEM_STATE_DELETE       0                           /* Key-value item state: DELETE --> the key-value item is deleted */
#define ITEM_MAX_KEY_LEN        255                         /* The max key length for key-value item */
#define ITEM_MAX_VAL_LEN        512                         /* The max value length for key-value item */
#define ITEM_MAX_LEN            (ITEM_HEADER_SIZE + ITEM_MAX_KEY_LEN + ITEM_MAX_VAL_LEN)

/* Defination of key-value store information */
#define KV_STATE_OFF            1                           /* The offset of block/item state in header structure */
#define KV_ALIGN_MASK           ~(sizeof(void *) - 1)       /* The mask of key-value store alignment */
#define KV_GC_RESERVED          1                           /* The reserved block for garbage collection */ 

/* Flash block header description */
typedef struct _block_header_t
{
    uint8_t     magic;                                      /* The magic number of block */
    uint8_t     state;                                      /* The state of the block */
    uint8_t     reserved[2];
}block_hdr_t;

/* Key-value item header description */
typedef struct _item_header_t
{
    uint8_t     magic;                                      /* The magic number of key-value item */
    uint8_t     state;                                      /* The state of key-value item */
    uint8_t     crc;                                        /* The crc-8 value of key-value item */
    uint8_t     key_len;                                    /* The length of the key */
    uint16_t    val_len;                                    /* The length of the value */
    uint16_t    origin_off;                                 /* The origin key-value item offset, it will be used when updating */
}item_hdr_t;

/* Key-value item description */
typedef struct _kv_item_t
{
    item_hdr_t  hdr;                                        /* The header of the key-value item, detail see the item_hdr_t structure */
    char       *store;                                      /* The store buffer for key-value */
    uint16_t    len;                                        /* The length of the buffer */
    uint16_t    pos;                                        /* The store position of the key-value item */
}kv_item_t;

/* Block information structure for management */
typedef struct _block_info_t
{
    uint16_t    space;                                      /* Free space in current block */
    uint8_t     state;                                      /* The state of current block */
}block_info_t;

static yos_mutex_t  g_kv_mutex;
static yos_work_t   g_gc_work;
static yos_sem_t    g_gc_sem;

static block_info_t g_block_info[BLK_NUMS];                 /* The array to record block management information */

static uint16_t g_write_pos;                                /* Current write position for key-value item */

static const uint8_t BLK_MAGIC_NUM  = 'K';                  /* The block header magic number */
static const uint8_t ITEM_MAGIC_NUM = 'I';                  /* The key-value item header magic number */

static uint8_t g_kv_init;                                   /* The flag to indicate the key-value store is initialized */
static uint8_t g_gc_triggered;                              /* The flag to indicate garbage collection is triggered */
static uint8_t g_clean_blk_nums;                            /* The number of block which state is clean */

static void yos_kv_gc(void *arg);

/* CRC-8: the poly is 0x31 (x^8 + x^5 + x^4 + 1) */
static uint8_t utils_crc8(uint8_t *buf, uint16_t length)
{
    uint8_t crc = 0x00;
    uint8_t i;

    while (length--) {
        crc ^= *buf++;
        for (i = 8; i > 0; i--) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc <<= 1;
        }
    }

    return crc;
}

static int raw_read(uint32_t offset, void *buf, size_t nbytes)
{
#ifdef CONFIG_YOS_KV_MULTIPTN_MODE
    int off = offset - KV_PTN_SIZE;
    if (off >= 0)
        return hal_flash_read(KV_SECOND_PTN, &off, buf, nbytes);
#endif
    return hal_flash_read(KV_PTN, &offset, buf, nbytes);
}

static int raw_write(uint32_t offset, const void *buf, size_t nbytes)
{
#ifdef CONFIG_YOS_KV_MULTIPTN_MODE
    int off = offset - KV_PTN_SIZE;
    if (off >= 0)
        return hal_flash_write(KV_SECOND_PTN, &off, buf, nbytes);
#endif
    return hal_flash_write(KV_PTN, &offset, buf, nbytes);
}

static int raw_erase(uint32_t offset, uint32_t size)
{
#ifdef CONFIG_YOS_KV_MULTIPTN_MODE
    int off = offset - KV_PTN_SIZE;
    if (off >= 0)
        return hal_flash_erase(KV_SECOND_PTN, off, size);
#endif
    return hal_flash_erase(KV_PTN, offset, size);
}

static void trigger_gc(void)
{
    if (g_gc_triggered)
        return;

    g_gc_triggered = 1;
    yos_work_sched(&g_gc_work);
}

static void kv_item_free(kv_item_t *item)
{
    if (item) {
        if (item->store)
            yos_free(item->store);
        yos_free(item);
    }
}

static int kv_state_set(uint16_t pos, uint8_t state)
{
    return raw_write(pos + KV_STATE_OFF, &state, 1);
}

static int kv_block_format(uint8_t index)
{
    block_hdr_t hdr;
    uint16_t pos = index << BLK_BITS;

    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = BLK_MAGIC_NUM;
    if(!raw_erase(pos, BLK_SIZE))
        hdr.state = BLK_STATE_CLEAN;
    else
        return RES_FLASH_EARSE_ERR;

    if (raw_write(pos, &hdr, BLK_HEADER_SIZE) != RES_OK)
        return RES_FLASH_WRITE_ERR;

    g_block_info[index].state = BLK_STATE_CLEAN;
    g_block_info[index].space = BLK_SIZE - BLK_HEADER_SIZE;
    g_clean_blk_nums++;
    return RES_OK;
}

static uint16_t kv_item_calc_pos(uint16_t len)
{
    block_info_t *blk_info;
    uint8_t blk_index = g_write_pos >> BLK_BITS;
    uint8_t i;

    blk_info = &(g_block_info[blk_index]);
    if (blk_info->space > len) {
        if (((blk_info->space - len) < ITEM_MAX_LEN) && (g_clean_blk_nums <= KV_GC_RESERVED)) {
            trigger_gc();
        }
        return g_write_pos;
    }

#if BLK_NUMS > KV_GC_RESERVED + 1
    for (i = blk_index + 1; i != blk_index; i++) {
        if (i == BLK_NUMS)
            i = 0;

        blk_info = &(g_block_info[i]);
        if ((blk_info->space) > len) {
            g_write_pos = (i << BLK_BITS) + BLK_SIZE - blk_info->space;
            if (blk_info->state == BLK_STATE_CLEAN) {
                if (kv_state_set((i << BLK_BITS), BLK_STATE_USED) != RES_OK)
                    return 0;
                blk_info->state = BLK_STATE_USED;
                g_clean_blk_nums--;
            }
            return g_write_pos;
        }
    }
#endif

    return 0;
}

int kv_item_del(kv_item_t *item)
{
    int ret = RES_OK;
    uint8_t state;
    uint8_t i = item->pos >> BLK_BITS;

    if (g_block_info[i].state != BLK_STATE_CLEAN) {
        ret = kv_state_set(item->pos, ITEM_STATE_DELETE);
        if (ret != RES_OK)
            return ret;

        ret = kv_state_set((item->pos & BLK_OFF_MASK), BLK_STATE_DIRTY);
        g_block_info[i].state = BLK_STATE_DIRTY;
    }
    return ret;
}

/*the function to be invoked while polling the used block*/
typedef int (*item_func)(kv_item_t *item, const char *key);

static int __item_recovery_cb(kv_item_t *item, const char *key)
{
    char *p = (char *)yos_malloc(item->len);
    if (!p)
        return RES_MALLOC_FAILED;

    if(raw_read(item->pos + ITEM_HEADER_SIZE, p, item->len) != RES_OK)
        return RES_FLASH_READ_ERR;

    if (item->hdr.crc == utils_crc8((uint8_t *)p , item->len)) {
        if (item->hdr.origin_off != 0) {
            item->pos = item->hdr.origin_off;
            item->len = 0;
            kv_item_del(item);
        }
    } else {
        kv_item_del(item);
    }

    yos_free(p);
    return RES_CONT;
}

static int __item_find_cb(kv_item_t *item, const char *key)
{
    if (item->hdr.key_len != strlen(key))
        return RES_CONT;

    item->store = (char *)yos_malloc(item->hdr.key_len + item->hdr.val_len);
    if (!item->store)
        return RES_MALLOC_FAILED;

    if (raw_read(item->pos + ITEM_HEADER_SIZE, item->store, item->len) != RES_OK)
        return RES_FLASH_READ_ERR;

    if (memcmp(item->store, key, strlen(key)) == 0)
        return RES_OK;

    return RES_CONT;
}

static int __item_gc_cb(kv_item_t *item, const char *key)
{
    char *p;
    int ret;
    uint16_t len;
    uint8_t index;

    len = (ITEM_HEADER_SIZE + item->len + ~KV_ALIGN_MASK) & KV_ALIGN_MASK;
    p = (char *)yos_malloc(len);
    if (!p)
        return RES_MALLOC_FAILED;

    if (raw_read(item->pos, p, len) != RES_OK) {
        ret = RES_FLASH_READ_ERR;
        goto err;
    }

    if (raw_write(g_write_pos, p, len) != RES_OK) {
        ret = RES_FLASH_WRITE_ERR;
        goto err;
    }

    g_write_pos += len;
    index = g_write_pos >> BLK_BITS;
    g_block_info[index].space -= len;
    ret = RES_CONT;

err:
    yos_free(p);
    return ret;
}

kv_item_t *kv_item_traverse(item_func func, uint8_t blk_index, const char *key)
{
    kv_item_t *item;
    item_hdr_t *hdr;
    uint16_t pos = (blk_index << BLK_BITS) + BLK_HEADER_SIZE;
    uint16_t end = (blk_index << BLK_BITS) + BLK_SIZE;
    uint16_t len = 0;

    do {
        item = (kv_item_t *)yos_malloc(sizeof(kv_item_t));
        if (!item)
            return NULL;
        memset(item, 0, sizeof(kv_item_t));
        hdr = &(item->hdr);

        raw_read(pos, hdr, ITEM_HEADER_SIZE);
        if (hdr->magic != ITEM_MAGIC_NUM) {
            if (hdr->magic == 0xFF) {
                kv_item_free(item);
                break;
            }
            hdr->val_len = 0xFFFF;
        }

        if (hdr->val_len > ITEM_MAX_VAL_LEN|| hdr->key_len > ITEM_MAX_KEY_LEN) {
            len += ITEM_HEADER_SIZE;
            continue;
        }

        len = (ITEM_HEADER_SIZE + hdr->key_len + hdr->val_len + ~KV_ALIGN_MASK) & KV_ALIGN_MASK;

        if (hdr->state == ITEM_STATE_NORMAL) {
            item->pos = pos;
            item->len = hdr->key_len + hdr->val_len;
            int ret = func(item, key);
            if (ret == RES_OK)
                return item;
            else if (ret < 0) {
                kv_item_free(item);
                return NULL;
            }
        }

        kv_item_free(item);
        pos += len;
    } while (pos < end);

    g_block_info[blk_index].space = (end - pos) > 0 ? (end - pos) : 0;
    return NULL;
}

kv_item_t *kv_item_get(const char *key)
{
    kv_item_t *item;
    uint8_t i;

    for (i = 0; i < BLK_NUMS; i++) {
        if (g_block_info[i].state != BLK_STATE_CLEAN) {
            item = kv_item_traverse(__item_find_cb, i, key);
            if (item)
                return item;
        }
    }

    return NULL;
}

typedef struct {
    char *p;
    int ret;
    uint16_t len;
} kv_storeage_t;
int kv_item_store(const char *key, const void *val, int len, uint16_t origin_off)
{
    kv_storeage_t store;
    item_hdr_t hdr;
    char *p;
    uint16_t pos;

    hdr.magic = ITEM_MAGIC_NUM;
    hdr.state = ITEM_STATE_NORMAL;
    hdr.key_len = strlen(key);
    hdr.val_len = len;
    hdr.origin_off = origin_off;

    store.len = (ITEM_HEADER_SIZE + hdr.key_len + hdr.val_len + ~KV_ALIGN_MASK) & KV_ALIGN_MASK;
    store.p = (char *)yos_malloc(store.len);
    if (!store.p)
        return RES_MALLOC_FAILED;

    memset(store.p, 0, store.len);
    p = store.p + ITEM_HEADER_SIZE;
    memcpy(p, key, hdr.key_len);
    p += hdr.key_len;
    memcpy(p, val, hdr.val_len);
    p -= hdr.key_len;
    hdr.crc = utils_crc8((uint8_t *)p, hdr.key_len + hdr.val_len);
    memcpy(store.p, &hdr, ITEM_HEADER_SIZE);

    pos = kv_item_calc_pos(store.len);
    if (pos > 0) {
        store.ret = raw_write(pos, store.p, store.len);
        if (store.ret == RES_OK) {
            g_write_pos = pos + store.len;
            uint8_t index = g_write_pos >> BLK_BITS;
            g_block_info[index].space -= store.len;
        }
    } else
        store.ret = RES_NO_SPACE;

    if (store.p)
        yos_free(store.p);
    return store.ret;
}

int kv_item_update(kv_item_t *item, const char *key, const void *val, int len)
{
    int ret;

    if (item->hdr.val_len == len) {
        if (!memcmp(item->store + item->hdr.key_len, val, len))
            return RES_OK;
    }

    ret = kv_item_store(key, val, len, item->pos);
    if (ret != RES_OK)
        return ret;

    ret = kv_item_del(item);

    return ret;
}

int kv_init(void)
{
    block_hdr_t hdr;
    int ret;
    uint8_t i;
    uint8_t tmp;

    for (i = 0; i < BLK_NUMS; i++) {
        memset(&hdr, 0, sizeof(block_hdr_t));
        raw_read((i << BLK_BITS), &hdr, BLK_HEADER_SIZE);
        if (hdr.magic == BLK_MAGIC_NUM) {
            g_block_info[i].state = hdr.state;
            kv_item_traverse(__item_recovery_cb, i, NULL);
            if (hdr.state == BLK_STATE_CLEAN) {
                if (g_block_info[i].space != (BLK_SIZE - BLK_HEADER_SIZE)) {
                    if ((ret = kv_block_format(i)) != RES_OK)
                        return ret;
                }
                else
                    g_clean_blk_nums++;
            }
        } else {
             if ((ret = kv_block_format(i)) != RES_OK)
                return ret;
        }
    }

    if (g_clean_blk_nums == BLK_NUMS) {
        g_write_pos = BLK_HEADER_SIZE;
        if (!kv_state_set((g_write_pos & BLK_OFF_MASK), BLK_STATE_USED)) {
            g_block_info[0].state = BLK_STATE_USED;
            g_clean_blk_nums--;
        }
    } else {
        for (i = 0; i < BLK_NUMS; i++) {
            if (g_block_info[i].state == BLK_STATE_CLEAN) {
                if (i == 0) {
                   i = BLK_NUMS - 1;
                   g_write_pos = (i << BLK_BITS) + BLK_SIZE - g_block_info[i].space;
                }
                else {
                   tmp = i - 1;
                   g_write_pos = (tmp << BLK_BITS) + BLK_SIZE - g_block_info[tmp].space;
                }
                break;
            }
        }
    }
  
    return RES_OK;
}

static void yos_kv_gc(void *arg)
{
    uint8_t i;
    uint8_t gc_index;
    uint16_t origin_pos = g_write_pos;

    if (yos_mutex_lock(&g_kv_mutex, YOS_WAIT_FOREVER) != 0)
        goto exit;

    if (g_clean_blk_nums == 0) {
        goto exit;
    }

    for (gc_index = 0; gc_index < BLK_NUMS; gc_index++) {
        if (g_block_info[gc_index].state == BLK_STATE_CLEAN) {
            g_write_pos = (gc_index << BLK_BITS) + BLK_HEADER_SIZE;
            break;
        }
    }

    i = (origin_pos >> BLK_BITS) + 1;
    while (1) {
        if (i == BLK_NUMS)
            i = 0;

        if (g_block_info[i].state == BLK_STATE_DIRTY) {
            kv_item_traverse(__item_gc_cb, i, NULL);

            if (kv_block_format(i) != RES_OK)
                goto exit;

            kv_state_set((g_write_pos & BLK_OFF_MASK), BLK_STATE_USED);
            g_block_info[gc_index].state = BLK_STATE_USED;
            g_clean_blk_nums--;
            break;
        }
        if (i == (origin_pos >> BLK_BITS))
            break;
            i++;
    }

exit:
    yos_mutex_unlock(&g_kv_mutex);
    g_gc_triggered = 0;
    yos_sem_signal_all(&g_gc_sem);
}

int yos_kv_del(const char *key)
{
    kv_item_t *item;
    int ret;
    if (yos_mutex_lock(&g_kv_mutex, YOS_WAIT_FOREVER) != 0)
        return RES_MUTEX_ERR;

    item = kv_item_get(key);
    if (!item) {
        yos_mutex_unlock(&g_kv_mutex);
        return RES_ITEM_NOT_FOUND;
    }

    ret = kv_item_del(item);
    kv_item_free(item);
    yos_mutex_unlock(&g_kv_mutex);
    return ret;
}

int yos_kv_set(const char *key, const void *val, int len, int sync)
{
    kv_item_t *item;
    int ret;
    if (!key || !val || len <= 0 || strlen(key) > ITEM_MAX_KEY_LEN || len > ITEM_MAX_VAL_LEN)
        return RES_INVALID_PARAM;

    if (g_gc_triggered)
        yos_sem_wait(&g_gc_sem, YOS_WAIT_FOREVER);
    
    if (yos_mutex_lock(&g_kv_mutex, YOS_WAIT_FOREVER) != 0)
        return RES_MUTEX_ERR;

    item = kv_item_get(key);
    if (item) {
        ret = kv_item_update(item, key, val, len);
        kv_item_free(item);
    }
    else
        ret = kv_item_store(key, val, len, 0);

    yos_mutex_unlock(&g_kv_mutex);
    return ret;
}

int yos_kv_get(const char *key, void *buffer, int *buffer_len)
{
    kv_item_t *item = NULL;

    if (!key || !buffer || !buffer_len || *buffer_len <= 0) {
        return RES_INVALID_PARAM;
    }

    item = kv_item_get(key);
    if (!item)
        return RES_ITEM_NOT_FOUND;

    if (*buffer_len < item->hdr.val_len) {
        *buffer_len = item->hdr.val_len;
        kv_item_free(item);
        return RES_NO_SPACE;
    } else {
        memcpy(buffer, (item->store + item->hdr.key_len), item->hdr.val_len);
        *buffer_len = item->hdr.val_len;
    }

    kv_item_free(item);
    return RES_OK;
}

/* CLI Support */
static int __item_print_cb(kv_item_t *item, const char *key)
{
    char *p_key = (char *)yos_malloc(item->hdr.key_len + 1);
    if (!p_key)
        return RES_MALLOC_FAILED;
    memset(p_key, 0, item->hdr.key_len + 1);
    raw_read(item->pos + ITEM_HEADER_SIZE, p_key, item->hdr.key_len);

    char *p_val = (char *)yos_malloc(item->hdr.val_len + 1);
    if (!p_val) {
        yos_free(p_key);
        return RES_MALLOC_FAILED;
    }
    memset(p_val, 0, item->hdr.val_len + 1);
    raw_read(item->pos + ITEM_HEADER_SIZE + item->hdr.key_len, p_val, item->hdr.val_len);

    LOGI("KV", "%s = %s", p_key, p_val);
    yos_free(p_key);
    yos_free(p_val);

    return RES_CONT;
}

static void handle_kv_cmd(char *pwbuf, int blen, int argc, char **argv)
{
    const char *rtype = argc > 1 ? argv[1] : "";
    int ret = 0;
    char *buffer = NULL;

    if (strcmp(rtype, "set") == 0) {
        if (argc != 4) {
            return ;
        }
        ret = yos_kv_set(argv[2], argv[3], strlen(argv[3]), 1);
        if (ret != 0) {
            LOGW("KV", "cli set kv failed");
        }
    } else if (strcmp(rtype, "get") == 0) {
        if (argc != 3) {
            return ;
        }
        buffer = yos_malloc(BLK_SIZE);
        if (!buffer) {
            LOGW("KV", "cli get kv failed");
            return;
        }

        memset(buffer, 0, BLK_SIZE);
        int len = BLK_SIZE;

        ret = yos_kv_get(argv[2], buffer, &len);
        if (ret != 0) {
            LOGW("KV", "cli: no paired kv");
        } else {
            LOGI("KV", "value is %s", buffer);
        }

        if (buffer) {
            yos_free(buffer);
        }
    } else if (strcmp(rtype, "del") == 0) {
        if (argc != 3) {
            return;
        }
        ret = yos_kv_del(argv[2]);
        if (ret != 0) {
            LOGW("KV", "cli kv del failed");
        }
    } else if (strcmp(rtype, "list") == 0) {
        for (int i = 0; i < BLK_NUMS; i++) {
            kv_item_traverse(__item_print_cb, i, NULL);
        }
    }
    return;
}


static struct cli_command ncmd = {
    .name = "kv",
    .help = "kv [set key value | get key | del key | list]",
    .function = handle_kv_cmd,
};

int yos_kv_init(void)
{
    int ret;

    if (g_kv_init)
        return RES_OK;

    if (yos_mutex_new(&g_kv_mutex) != 0) {
        return RES_MUTEX_ERR;
    }

    cli_register_command(&ncmd);

    if (kv_init() != RES_OK)
        return -1;

    if (yos_work_init(&g_gc_work, yos_kv_gc, NULL, 0) != RES_OK)
        return RES_WORK_ERR;

    if (yos_sem_new(&g_gc_sem, 0) != RES_OK)
        return RES_SEM_ERR;

    g_kv_init = 1;

    if (((g_write_pos & (~BLK_OFF_MASK)) > (BLK_SIZE - ITEM_MAX_LEN)) && 
        (g_clean_blk_nums < KV_GC_RESERVED + 1))
        trigger_gc();

    return RES_OK;
}


void yos_kv_deinit(void)
{
    g_kv_init = 0;
    yos_work_cancel(&g_gc_work);
    yos_work_destroy(&g_gc_work);
    yos_sem_free(&g_gc_sem);
    yos_mutex_free(&g_kv_mutex);
}


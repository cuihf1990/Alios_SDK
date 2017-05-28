/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
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
#ifndef K_MM_RINGBUF_H
#define K_MM_RINGBUF_H

#define RINGBUF_TYPE_FIX              0
#define RINGBUF_TYPE_DYN              1

#define RINGBUF_LEN_MAX_SIZE          3
#define RINGBUF_LEN_MASK_ONEBIT       0x80 //1000 0000
#define RINGBUF_LEN_MASK_TWOBIT       0xC0 //1100 0000
#define RINGBUF_LEN_MASK_CLEAN_TWOBIT 0x3f //0011 1111


#define RINGBUF_LEN_VLE_2BYTES        0x80 //1000 0000
#define RINGBUF_LEN_VLE_3BYTES        0xC0 //1100 0000

#define RINGBUF_LEN_1BYTE_MAXVALUE    0x7f //0111 1111; 127
#define RINGBUF_LEN_2BYTES_MAXVALUE   0x3fff // 16383
#define RINGBUF_LEN_3BYTES_MAXVALUE   0x3fffff //4194303


#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

typedef struct {
    uint8_t * buf;
    uint8_t * end;
    uint8_t * head;
    uint8_t * tail;
    size_t    freesize;
    size_t    type:         1;
    size_t    blk_size:     (8 * sizeof(size_t) -1);
} k_ringbuf_t;

#define COMPRESS_LEN(x) ((x) <= RINGBUF_LEN_1BYTE_MAXVALUE ? 1: (x) <= RINGBUF_LEN_2BYTES_MAXVALUE ? 2: \
                        (x) <= RINGBUF_LEN_3BYTES_MAXVALUE ? 3 : YUNOS_INV_PARAM)


#if (YUNOS_CONFIG_RINGBUF_VENDOR > 0)
/**
 * This function will init the mm ring buffer.
 * @param[in]  p_ringbuf   pointer to ring buffer
 * @param[in]  buf         pointer to memory buffer
 * @param[in]  len         length of memory buffer
 * @param[in]  type        type of ring buffer, fix length or dynamic length
 * @param[in]  block_size  block size of fix length ringbuf, if dynamic ringbuffer, ignore this parameter
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_ringbuf_init   (k_ringbuf_t *p_ringbuf, void *buf, size_t len, size_t type, size_t block_size);

/**
 * This function will clean all data in mm ring buffer.
 * @param[in]  p_ringbuf   pointer to ring buffer
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_ringbuf_reset(k_ringbuf_t *p_ringbuf);

/**
 * This function will push the data to ring buffer end.
 * @param[in]  p_ringbuf   pointer to ring buffer
 * @param[in]  data        pointer to data
 * @param[in]  len         length of data
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_ringbuf_push(k_ringbuf_t *p_ringbuf, void *data, size_t len);

/**
 * This function will push the data to ring buffer head.
 * @param[in]  p_ringbuf   pointer to ring buffer
 * @param[in]  data        pointer to data
 * @param[in]  len         length of data
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_ringbuf_head_push(k_ringbuf_t *p_ringbuf, void * data, size_t len);

/**
 * This function will pop the data from ring buffer head.
 * @param[in]       p_ringbuf   pointer to ring buffer
 * @param[out]      pdata        pointer to data
 * @param[out]      plen         length of data
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_ringbuf_pop(k_ringbuf_t *p_ringbuf, void * pdata, size_t * plen);


/**
 * This function will check if  the ring buffer is full.
 * @param[in]       p_ringbuf   pointer to ring buffer
 * @return  the ringbuf status, true is full. else is not.
 */
uint8_t    yunos_ringbuf_is_full(k_ringbuf_t *p_ringbuf);

/**
 * This function will check if  the ring buffer is empty.
 * @param[in]       p_ringbuf   pointer to ring buffer
 * @return  the ringbuf status, true is empty. else is not.
 */
uint8_t    yunos_ringbuf_is_empty(k_ringbuf_t *p_ringbuf);
#endif

#endif

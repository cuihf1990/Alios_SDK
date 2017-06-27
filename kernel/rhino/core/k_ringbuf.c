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

#include <k_api.h>

kstat_t ringbuf_init(k_ringbuf_t *p_ringbuf, void *buf, size_t len, size_t type,
                     size_t block_size)
{
    p_ringbuf->type     = type;
    p_ringbuf->buf      = buf;
    p_ringbuf->end      = (uint8_t *)buf + len;
    p_ringbuf->blk_size = block_size;

    ringbuf_reset(p_ringbuf);

    return YUNOS_SUCCESS;

}
static size_t ringbuf_headlen_compress(size_t head_len, uint8_t *cmp_buf)
{
    size_t len_bytes = 0;
    uint8_t *p_len   = NULL;
    size_t   be_len  = 0;

    be_len = yunos_ntohl(head_len);
    p_len = (uint8_t *)&be_len;
    len_bytes = COMPRESS_LEN(head_len);

    if (len_bytes == 1) {
        cmp_buf[0] = RINGBUF_LEN_1BYTE_MAXVALUE & p_len[3];
    } else if (len_bytes == 2) {
        cmp_buf[0] = RINGBUF_LEN_VLE_2BYTES | p_len[2];
        cmp_buf[1] = p_len[3];
    } else if (len_bytes == 3) {
        cmp_buf[0] = RINGBUF_LEN_VLE_3BYTES | p_len[1];
        cmp_buf[1] = p_len[2];
        cmp_buf[2] = p_len[3];
    }

    return len_bytes;
}

static size_t ringbuf_headlen_decompress(size_t buf_len, uint8_t *cmp_buf)
{
    size_t   data_len = 0;
    size_t   be_len   = 0;
    uint8_t *len_buf  = (uint8_t *)&be_len;

    memcpy(&len_buf[sizeof(size_t) - buf_len], cmp_buf, buf_len);

    if (buf_len > 1) {
        len_buf[sizeof(size_t) - buf_len] &= RINGBUF_LEN_MASK_CLEAN_TWOBIT;
    }

    data_len = yunos_ntohl(be_len);

    return data_len;
}

kstat_t ringbuf_push(k_ringbuf_t *p_ringbuf, void *data, size_t len)
{
    size_t   len_bytes                   = 0;
    size_t   split_len                   = 0;
    uint8_t  c_len[RINGBUF_LEN_MAX_SIZE] = {0};

    if (ringbuf_is_full(p_ringbuf)) {
        return YUNOS_RINGBUF_FULL;
    }

    if (p_ringbuf->type == RINGBUF_TYPE_FIX) {
        if (p_ringbuf->tail == p_ringbuf->end) {
            p_ringbuf->tail = p_ringbuf->buf;
        }

        memcpy(p_ringbuf->tail, data, len);
        p_ringbuf->tail += len;
        p_ringbuf->freesize -= len;

        return YUNOS_SUCCESS;
    } else {
        len_bytes = ringbuf_headlen_compress(len, c_len);
        if (len_bytes == 0 || len_bytes > RINGBUF_LEN_MAX_SIZE ) {
            return YUNOS_INV_PARAM;
        }

        /* for dynamic length ringbuf */
        if (p_ringbuf->freesize < len_bytes + len ) {
            return YUNOS_RINGBUF_FULL;
        }

        if (p_ringbuf->tail == p_ringbuf->end) {
            p_ringbuf->tail = p_ringbuf->buf;
        }

        /* copy length data to buffer */
        if (p_ringbuf->tail >= p_ringbuf->head &&
            (split_len = p_ringbuf->end - p_ringbuf->tail) < len_bytes && split_len > 0) {
            memcpy(p_ringbuf->tail, &c_len[0], split_len);
            len_bytes -= split_len;
            p_ringbuf->tail =  p_ringbuf->buf;
            p_ringbuf->freesize -= split_len;
        } else {
            split_len = 0;
        }

        if (len_bytes > 0) {
            memcpy(p_ringbuf->tail, &c_len[split_len], len_bytes);
            p_ringbuf->freesize -= len_bytes;
            p_ringbuf->tail += len_bytes;
        }

        /* copy data to ringbuf, if break by buffer end, split data and copy to buffer head*/
        split_len = 0;

        if (p_ringbuf->tail == p_ringbuf->end) {
            p_ringbuf->tail = p_ringbuf->buf;
        }

        if (p_ringbuf->tail >= p_ringbuf->head &&
            ((split_len = p_ringbuf->end - p_ringbuf->tail) < len) &&
            split_len > 0) {
            memcpy(p_ringbuf->tail, data, split_len);
            data = (uint8_t *)data + split_len;
            len -= split_len;
            p_ringbuf->tail =  p_ringbuf->buf;
            p_ringbuf->freesize -= split_len;
        }

        memcpy(p_ringbuf->tail, data, len);
        p_ringbuf->tail += len;
        p_ringbuf->freesize -= len;


        return YUNOS_SUCCESS;

    }
    return YUNOS_SYS_FATAL_ERR;
}

kstat_t ringbuf_head_push(k_ringbuf_t *p_ringbuf, void *data, size_t len)
{
    size_t   len_bytes                   = 0;
    size_t   split_len                   = 0;
    uint8_t  c_len[RINGBUF_LEN_MAX_SIZE] = {0};

    if (ringbuf_is_full(p_ringbuf)) {
        return YUNOS_RINGBUF_FULL;
    }
    if (p_ringbuf->type == RINGBUF_TYPE_FIX) {
        if (p_ringbuf->head == p_ringbuf->buf) {
            p_ringbuf->head = p_ringbuf->end;
        }

        p_ringbuf->head -= len;
        memcpy(p_ringbuf->head, data, len);
        p_ringbuf->freesize -= len;

        return YUNOS_SUCCESS;
    } else {
        len_bytes = ringbuf_headlen_compress(len, c_len);
        if (len_bytes == 0 || len_bytes > RINGBUF_LEN_MAX_SIZE ) {
            return YUNOS_INV_PARAM;
        }

        if (p_ringbuf->freesize < len_bytes + len ) {
            return YUNOS_RINGBUF_FULL;
        }

        if (p_ringbuf->head == p_ringbuf->buf) {
            p_ringbuf->head = p_ringbuf->end;
        }

        if (p_ringbuf->head <= p_ringbuf->tail &&
            ((split_len = p_ringbuf->head - p_ringbuf->buf) < len) &&
            split_len > 0) {

            memcpy(p_ringbuf->buf, data, split_len);
            data = (uint8_t *)data + split_len;
            len -= split_len;

            p_ringbuf->head =  p_ringbuf->end;
            p_ringbuf->freesize -= split_len;
        }

        p_ringbuf->head -= len;
        memcpy(p_ringbuf->head, data, len);
        p_ringbuf->freesize -= len;

        /* copy length data to buffer */
        if (p_ringbuf->head == p_ringbuf->buf) {
            p_ringbuf->head = p_ringbuf->end;
        }

        split_len = 0;

        if ( p_ringbuf->head <= p_ringbuf->tail &&
             ((split_len = p_ringbuf->head - p_ringbuf->buf) < len_bytes) &&
             split_len > 0) {

            memcpy(p_ringbuf->buf, &c_len[len_bytes - split_len], split_len);
            len_bytes -= split_len;

            p_ringbuf->head =  p_ringbuf->end;
            p_ringbuf->freesize -= split_len;
        }

        if (len_bytes > 0) {
            p_ringbuf->head -= len_bytes;
            memcpy(p_ringbuf->head, &c_len[0], len_bytes);
            p_ringbuf->freesize -= len_bytes;
        }

        return YUNOS_SUCCESS;

    }
}

kstat_t ringbuf_pop(k_ringbuf_t *p_ringbuf, void *pdata, size_t *plen)
{
    size_t   split_len = 0;
    uint8_t *data      = pdata;
    size_t   len       = 0;
    uint8_t  c_len[RINGBUF_LEN_MAX_SIZE] = {0};
    size_t   len_bytes = 0;

    if (ringbuf_is_empty(p_ringbuf)) {
        return YUNOS_RINGBUF_EMPTY;
    }

    if (p_ringbuf->type == RINGBUF_TYPE_FIX) {
        if (p_ringbuf->head == p_ringbuf->end) {
            p_ringbuf->head = p_ringbuf->buf;
        }

        memcpy(pdata, p_ringbuf->head, p_ringbuf->blk_size);
        p_ringbuf->head += p_ringbuf->blk_size;
        p_ringbuf->freesize += p_ringbuf->blk_size;

        if (plen != NULL) {
            *plen = p_ringbuf->blk_size;
        }

        return YUNOS_SUCCESS;
    } else {
        if (p_ringbuf->head == p_ringbuf->end) {
            p_ringbuf->head = p_ringbuf->buf;
        }

        /*decode length */
        if ((*p_ringbuf->head & RINGBUF_LEN_MASK_ONEBIT) == 0 ) {
            /*length use one byte*/
            len_bytes = 1;
        } else if ((*p_ringbuf->head & RINGBUF_LEN_MASK_TWOBIT) ==
                   RINGBUF_LEN_VLE_2BYTES) {
            /*length use 2 bytes*/
            len_bytes = 2;
        } else if ((*p_ringbuf->head & RINGBUF_LEN_MASK_TWOBIT) ==
                   RINGBUF_LEN_VLE_3BYTES) {
            /*length use 3 bytes*/
            len_bytes = 3;
        } else {
            return YUNOS_INV_PARAM;
        }


        if (((split_len = p_ringbuf->end - p_ringbuf->head) < len_bytes) &&
            split_len > 0) {

            memcpy(&c_len[0], p_ringbuf->head, split_len);

            p_ringbuf->head      =  p_ringbuf->buf;
            p_ringbuf->freesize += split_len;
        } else {
            split_len = 0;
        }

        if (len_bytes - split_len > 0) {
            memcpy(&c_len[split_len], p_ringbuf->head, (len_bytes - split_len));
            p_ringbuf->head     += (len_bytes - split_len);
            p_ringbuf->freesize += (len_bytes - split_len);
        }

        *plen = len = ringbuf_headlen_decompress(len_bytes, c_len);

        if (p_ringbuf->head == p_ringbuf->end) {
            p_ringbuf->head = p_ringbuf->buf;
        }

        if (p_ringbuf->head > p_ringbuf->tail &&
            (split_len = p_ringbuf->end - p_ringbuf->head) < len) {
            memcpy(pdata, p_ringbuf->head, split_len);
            data = (uint8_t *)pdata + split_len;
            len -= split_len;
            p_ringbuf->head      = p_ringbuf->buf;
            p_ringbuf->freesize += split_len;
        }

        memcpy(data, p_ringbuf->head, len);
        p_ringbuf->head     += len;
        p_ringbuf->freesize += len;

        return YUNOS_SUCCESS;

    }
}

uint8_t ringbuf_is_full(k_ringbuf_t *p_ringbuf)
{
    if (p_ringbuf->type == RINGBUF_TYPE_DYN && p_ringbuf->freesize < 2) {
        return 1;
    }

    if (p_ringbuf->type == RINGBUF_TYPE_FIX &&
        p_ringbuf->freesize < p_ringbuf->blk_size) {
        return 1;
    }

    return false;
}

uint8_t ringbuf_is_empty(k_ringbuf_t *p_ringbuf)
{
    if (p_ringbuf->freesize == (size_t)(p_ringbuf->end - p_ringbuf->buf)) {
        return true;
    }

    return false;
}
/*external api*/

kstat_t ringbuf_reset(k_ringbuf_t *p_ringbuf)
{
    p_ringbuf->head = p_ringbuf->buf;
    p_ringbuf->tail = p_ringbuf->buf;
    p_ringbuf->freesize = p_ringbuf->end - p_ringbuf->buf;

    return YUNOS_SUCCESS;
}

#if (YUNOS_CONFIG_RINGBUF_VENDOR > 0)

kstat_t yunos_ringbuf_reset(k_ringbuf_t *p_ringbuf)
{
    NULL_PARA_CHK(p_ringbuf);

    return ringbuf_reset(p_ringbuf);
}
kstat_t yunos_ringbuf_init(k_ringbuf_t *p_ringbuf, void *buf, size_t len,
                           size_t type, size_t block_size)
{
    NULL_PARA_CHK(p_ringbuf);
    NULL_PARA_CHK(buf);

    if (len == 0 || (type != RINGBUF_TYPE_DYN && type != RINGBUF_TYPE_FIX)) {
        return YUNOS_INV_PARAM;
    }

    if (type == RINGBUF_TYPE_FIX) {
        if (len == 0 || block_size == 0 || len % block_size) {
            return YUNOS_INV_PARAM;
        }
    }

    return ringbuf_init(p_ringbuf, buf, len, type, block_size);
}

kstat_t yunos_ringbuf_push(k_ringbuf_t *p_ringbuf, void *data, size_t len)
{
    NULL_PARA_CHK(p_ringbuf);
    NULL_PARA_CHK(data);

    if (len <= 0 || len > RINGBUF_LEN_3BYTES_MAXVALUE ||
        (p_ringbuf->type == RINGBUF_TYPE_FIX && len != p_ringbuf->blk_size) ) {

        return YUNOS_INV_PARAM;
    }

    return ringbuf_push(p_ringbuf, data, len);
}

kstat_t yunos_ringbuf_head_push(k_ringbuf_t *p_ringbuf, void *data, size_t len)
{
    NULL_PARA_CHK(p_ringbuf);
    NULL_PARA_CHK(data);

    if (len <= 0 || len > RINGBUF_LEN_3BYTES_MAXVALUE ||
        (p_ringbuf->type == RINGBUF_TYPE_FIX && len != p_ringbuf->blk_size) ) {
        return YUNOS_INV_PARAM;
    }

    return ringbuf_head_push(p_ringbuf, data, len);
}

kstat_t yunos_ringbuf_pop(k_ringbuf_t *p_ringbuf, void *pdata, size_t *plen)
{
    NULL_PARA_CHK(p_ringbuf);
    NULL_PARA_CHK(pdata);

    if (p_ringbuf->type == RINGBUF_TYPE_DYN && plen == NULL) {
        return YUNOS_INV_PARAM;
    }

    return ringbuf_pop(p_ringbuf, pdata, plen);

}

uint8_t yunos_ringbuf_is_empty(k_ringbuf_t *p_ringbuf)
{
    NULL_PARA_CHK(p_ringbuf);

    return ringbuf_is_empty(p_ringbuf);
}

uint8_t yunos_ringbuf_is_full(k_ringbuf_t *p_ringbuf)
{
    NULL_PARA_CHK(p_ringbuf);

    return ringbuf_is_full(p_ringbuf);
}
#endif


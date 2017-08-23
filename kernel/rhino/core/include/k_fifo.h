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

#ifndef FIFO_H
#define FIFO_H

#ifdef __cplusplus
extern "C" {
#endif

struct k_fifo {

    uint32_t     in;
    uint32_t     out;
    uint32_t     mask;
    void        *data;
    uint32_t     free_bytes;
    uint32_t     size;

};

#define fifo_min(x, y) ((x) > (y)?(y):(x))
#define fifo_max(x, y) ((x) > (y)?(x):(y))

/**
 * This function will init the fifo.
 * @param[in]  fifo         pointer to fifo
 * @param[in]  buffer       pointer to fifo buffer
 * @param[in]  size         size of fifo buffer
 * @return  the operation status, 0 is OK, others is error
 */
int8_t fifo_init(struct k_fifo *fifo, void *buffer, uint32_t size);

/**
 * This function will write buf to fifo.
 * @param[in]  fifo         pointer to fifo
 * @param[in]  buf          pointer to buffer write
 * @param[in]  size         size of buffer
 * @return  the size has been written to the fifo
 */
uint32_t fifo_in(struct k_fifo *fifo, const void *buf, uint32_t len);

/**
 * This function will write buf to fifo， when fifo is full, reject write.
 * @param[in]  fifo         pointer to fifo
 * @param[in]  buf          pointer to buffer write
 * @param[in]  len          len of buffer
 * @return  the size has been written to the fifo
 */
uint32_t fifo_in_full_reject(struct k_fifo *fifo, const void *buf, uint32_t len);

/**
 * This function will lock fifo and write to it， when fifo is full, reject write.
 * @param[in]  fifo         pointer to fifo
 * @param[in]  buf          pointer to buffer write
 * @param[in]  len          len of buffer
 * @return  the size has been written to the fifo
 */
uint32_t fifo_in_full_reject_lock(struct k_fifo *fifo, const void *buf, uint32_t len);

/**
 * This function will read fifo data to buf
 * @param[in]  fifo         pointer to fifo
 * @param[in]  buf          pointer to buffer read
 * @param[in]  len          len of buffer
 * @return  the size has read
 */
uint32_t fifo_out(struct k_fifo *fifo, void *buf, uint32_t len);

/**
 * This function will read fifo data to buf，but data remain in fifo
 * @param[in]  fifo         pointer to fifo
 * @param[in]  buf          pointer to buffer read
 * @param[in]  len          len of buffer
 * @return  the size has read
 */
uint32_t fifo_out_peek(struct k_fifo *fifo,
                        void *buf, uint32_t len);

/**
 * This function will read fifo all data
 * @param[in]  fifo         pointer to fifo
 * @param[in]  buf          pointer to buffer read
 * @return  the size has read
 */
uint32_t fifo_out_all(struct k_fifo *fifo, void *buf);

#ifdef __cplusplus
}
#endif

#endif


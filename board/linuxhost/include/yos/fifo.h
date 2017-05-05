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

    int8_t fifo_init(struct k_fifo *fifo, void *buffer, uint32_t size);

    uint32_t fifo_in(struct k_fifo *fifo, const void *buf, uint32_t len);
    uint32_t fifo_in_full_reject(struct k_fifo *fifo, const void *buf, uint32_t len);
    uint32_t fifo_in_full_reject_lock(struct k_fifo *fifo, const void *buf, uint32_t len);

    uint32_t fifo_out(struct k_fifo *fifo, void *buf, uint32_t len);


    uint32_t fifo_out_peek(struct k_fifo *fifo,
                           void *buf, uint32_t len);



    uint32_t fifo_out_all(struct k_fifo *fifo, void *buf);


#ifdef __cplusplus
}
#endif

#endif


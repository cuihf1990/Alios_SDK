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

/**
 * @file hal/soc/rng.h
 * @brief RNG HAL
 * @version since 5.5.0
 */

#ifndef YOS_RNG_H
#define YOS_RNG_H

typedef struct {
    uint8_t      port;    /* random device port */
    void        *priv;    /* priv data */
} random_dev_t;

/**@brief Fill in a memory buffer with random data
 * @param random          : the random device
 * @param inBuffer        : Point to a valid memory buffer, this function will fill
                            in this memory with random numbers after executed
 * @param inByteCount     : Length of the memory buffer (bytes)
 *
 * @return    0   : on success.
 * @return    EIO : if an error occurred with any step
 */
int32_t hal_random_num_read(random_dev_t random, void *in_buf, int32_t bytes);

#endif



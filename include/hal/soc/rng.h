/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

/**
 * @file hal/soc/rng.h
 * @brief RNG HAL
 * @version since 5.5.0
 */

#ifndef AOS_RNG_H
#define AOS_RNG_H

typedef struct {
    uint8_t      port;    /* random device port */
    void        *priv;    /* priv data */
} random_dev_t;

/**
 * Fill in a memory buffer with random data
 *
 * @param[in]   random  the random device
 * @param[out]  buf     Point to a valid memory buffer
 * @param[in]   bytes   Length of the memory buffer (bytes)
 *
 * @return      0       on success.
 * @return      EIO     if an error occurred with any step
 */
int32_t hal_random_num_read(random_dev_t random, void *buf, int32_t bytes);

#endif



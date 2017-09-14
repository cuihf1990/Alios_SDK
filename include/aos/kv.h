/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef AOS_KV_H
#define AOS_KV_H

#include <aos/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Add another KV pair.
 *
 * @param[in]  key    the key of the KV pair.
 * @param[in]  value  the value of the KV pair.
 * @param[in]  len    the length of the value.
 * @param[in]  sync   save the KV pair to flash right now (should always be 1)
 *
 * @retval  0 on success, negative error on failure
 */
int aos_kv_set(const char *key, const void *value, int len, int sync);

/**
 * Get the KV value stored in buffer by its key.
 *
 * @param[in]   key         the key of the KV you want to get.
 * @param[out]  buffer      the memory to store KV value.
 * @param[out]  buffer_len  the real length of value.
 * @note: the buffer_len should be large enough to store the value,
 *            otherwise buffer would be NULL.
 *
 * @retval  0 on success, negative error on failure
 */
int aos_kv_get(const char *key, void *buffer, int *buffer_len);

/**
 * Delete the KV pair by its key.
 *
 * @param[in]  key  the key of the KV pair you want to delete.
 *
 * @retval  0 on success, negative error on failure
 */
int aos_kv_del(const char *key);

#ifdef __cplusplus
}
#endif

#endif /* AOS_KV_H */


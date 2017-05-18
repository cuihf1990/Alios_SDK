#ifndef _key_value_h_
#define _key_value_h

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

#define HASH_TABLE_MAX_SIZE 1024
#define MAX_KV_LEN 256
#define KVFILE_PATH "./"
#define KVFILE_NAME "KVfile"
#define KVFILE_NAME_BACKUP "KVfile_backup"

/**
 * @brief init the kv module.
 *
 * @param[in] none.
 *
 * @note: the default KV size is @HASH_TABLE_MAX_SIZE, the path to store
 *        the kv file is @KVFILE_PATH.
 * @retval  0 on success, otherwise -1 will be returned
 */
int yos_kv_init();

/**
 * @brief deinit the kv module.
 *
 * @param[in] none.
 *
 * @note: all the KV in RAM will be released.
 * @retval none. 
 */
void yos_kv_deinit();

/**
 * @brief add another KV pair .
 *
 * @param[in] @key  the key of the KV pair.
 * @param[in] @value  the value of the KV pair.
 * @param[in] @len  the length of the @value.
 * @param[in] @sync  1: save the KV pair to flash right now, 
 *                   0: do not save the pair this time.
 *
 * @retval  0 on success, otherwise -1 will be returned
 */
int yos_kv_set(const char *key, const void *value, int len, int sync);

/**
 * @brief get the KV value stored in @buffer by its key @key.
 *
 * @param[in] @key, the key of the KV you want to get.
 * @param[out] @buffer, the memory to store KV value.
 * @param[out] @buff_len, the real lenght of value.
 *
 * @note: the @buffer_len should be large enough to store the value, 
 *            otherwise @buffer would be NULL.
 * @retval  0 on success, otherwise -1 will be returned
 */
int yos_kv_get(const char *key, char *buffer, int *buffer_len);

/**
 * @brief delete the KV pair by its key @key.
 *
 * @param[in] key , the key of the KV pair you want to delete.
 *
 * @retval  0 on success, otherwise -1 will be returned
 */
int yos_kv_del(const char *key);

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif

#endif

#ifndef _TFS_PAL_H
#define _TFS_PAL_H

#include <stdint.h>
// memory
void *pal_memory_malloc(int size);
void pal_memory_free(void *ptr);

// rand
int pal_get_random();

// network
int pal_network_create(const char *server, int port);
int pal_network_send(int sockFd, const char *buf, int len);
int pal_network_recv(int sockfd, char *buf, int *len);
int pal_network_close(int fd);

// base64
void pal_base64_encode(const unsigned char *src, int len,
                       unsigned char *dst, int *out_len);

// md5
void pal_md5_sum(const uint8_t *addr, const int len, uint8_t *mac);

// json
/*
 * @json_str: string needs to be parsed
 * @tokens: the path of values
 * @tokens_size: the length of path
 * @value: the value to get
 * @example
 * in json:
 * {
 *     key1:
 *     {
 *         key2:value2,
 *         key3:value3
 *     }
 *     key4:value4
 * }
 * if you look for key "key2", the code is:
 * char *tokens[2] = {"key1", "key2"};
 * char value[10];
 * pal_json_get_string_value(json, tokens, 2, value);
 * @return: 0~OK, -1~ERROR
 */
int pal_json_get_string_value(char *json_str, const char **tokens,
                              int tokens_size, char *value);
int pal_json_get_number_value(char *json_str, const char **tokens,
                              int tokens_size, int *value);

// device info
struct device_info {
    const char *product_name;
    const char *imei;
    const char *hardware_id;
    const char *mac;
    const char *bt_mac;
    const char *build_time;
    const char *os_version;
    const char *dm_pixels;
    const char *dm_dpi;
    const char *cup_info;
    const char *storage_total;
    const char *camera_resolution;
};

/*
 * @return: -1~ERROR, 0~OK
 * @note product_name is must, others are optional.
 */
int pal_collect_device_info(struct device_info *pInfo);

// storage
int pal_save_info(const char *key, char *value);
int pal_get_info(const char *key, char *value);

// time
// us since 1970 in linux or ms since power on in rtos
uint64_t pal_get_current_time();

#endif // for _TFS_PAL_H

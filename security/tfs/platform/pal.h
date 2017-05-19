/*
 *  Copyright (C) 2017 YunOS Project. All rights reserved.
 */

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
int pal_json_get_string_value(char *json_str, const char **tokens, int tokens_size, char *value);
int pal_json_get_number_value(char *json_str, const char **tokens, int tokens_size, int *value);

// product info
const char *pal_get_product_name();
const char *pal_get_imei();
const char *pal_get_hardware_id();
const char *pal_get_mac();
const char *pal_get_bt_mac();
const char *pal_get_build_time();
const char *pal_get_os_version();
const char *pal_get_dm_pixels();
const char *pal_get_dm_dpi();
const char *pal_get_cpu_info();
const char *pal_get_storage_total();
const char *pal_get_camera_resolution();

// storage
int pal_save_info(const char *key, char *value);
int pal_get_info(const char *key, char *value);

// time
// us since 1970 in linux or ms since power on in rtos
uint64_t pal_get_current_time();

#endif // for _TFS_PAL_H

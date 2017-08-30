/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */



/* From device.h */
#define IOTX_PRODUCT_KEY_LEN     (11)   /* IoTx product key length  */
#define IOTX_DEVICE_NAME_LEN     (20)   /* IoTx device name length */
#define IOTX_DEVICE_ID_LEN       (64)   /* IoTx device ID length */
#define IOTX_DEVICE_SECRET_LEN   (32)   /* IoTx device secret length */
#define IOTX_URI_MAX_LEN         (135)  /* IoTx CoAP uri maximal length */

#define MODULE_VENDOR_ID    (32) // Partner ID

#define HOST_ADDRESS_LEN    (128)
#define HOST_PORT_LEN       (8)
#define CLIENT_ID_LEN       (256)
#define USER_NAME_LEN       (128)
#define PASSWORD_LEN        (128)
#define AESKEY_STR_LEN      (32)
#define AESKEY_HEX_LEN      (128/8)

typedef struct {
    char        product_key[IOTX_PRODUCT_KEY_LEN + 1];
    char        device_name[IOTX_DEVICE_NAME_LEN + 1];
    char        device_id[IOTX_DEVICE_ID_LEN + 1];
    char        device_secret[IOTX_DEVICE_SECRET_LEN + 1];
    char        module_vendor_id[MODULE_VENDOR_ID + 1];
} iotx_device_info_t, *iotx_device_info_pt;

typedef struct {
    uint16_t        port;
    char            host_name[HOST_ADDRESS_LEN + 1];
    char            client_id[CLIENT_ID_LEN + 1];
    char            username[USER_NAME_LEN + 1];
    char            password[PASSWORD_LEN + 1];
    const char     *pub_key;
#ifdef MQTT_ID2_AUTH
    char            aeskey_str[AESKEY_STR_LEN];
    uint8_t         aeskey_hex[AESKEY_HEX_LEN];
#endif
} iotx_conn_info_t, *iotx_conn_info_pt;
/* From device.h */

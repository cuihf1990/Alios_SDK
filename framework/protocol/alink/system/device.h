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

#ifndef DEVICE_H
#define DEVICE_H

#include "os.h"
#include "config.h"

#define TOKEN_LEN       64
#define SERVER_LEN      64
#define UUID_LEN        32
#define ATTRS_LEN       128
#define SERVERADDR_LEN  80

#define CONFIG_MAGIC        "Alink+"
#define CONFIG_VERSION      0x01
#define CONFIG_LEN      ((long)(&((device_config_t  *)0)->crc))

enum {
    DEVICE_OK = 0,
    DEVICE_ERR
};

typedef enum {
    MEMUSED = 0,
    DISCONNCOUNTER,
    DISAPCOUNTER,
    MALFORMEDPACKETS,
    LATESTRTT,
    AVERAGERTT,
    WIFISTRENGTH,
    MAX_STATIS
} STATISTYPE;

typedef struct device_info {
    char model[PRODUCT_MODEL_LEN]; /* <brand>_<category>_<type>_<name> */
    char key[PRODUCT_KEY_LEN];
    char secret[PRODUCT_SECRET_LEN];
    char debug_key[PRODUCT_KEY_LEN];
    char debug_secret[PRODUCT_SECRET_LEN];
    char sn[PRODUCT_SN_LEN];

    char cid[PLATFORM_CID_LEN];
    char mac[PLATFORM_MAC_LEN];   /* xx:xx:xx:xx:xx:xx */

    //TODO: move this to config area
    char alink_version[STR_SHORT_LEN];
    char os_version[STR_SHORT_LEN];
    char firmware_version[STR_SHORT_LEN];
} device_info_t;

typedef struct device {
    device_info_t *info;
    device_config_t *config;
} device_t;

void device_init(void);
void device_exit(void);
const device_t *get_main_dev(void);
void device_get_config(void* config);
char *devinfo_get_secret(void);
char *devinfo_get_key(void);
char *devinfo_get_version(void);

//TODO: remove these header
void get_wifi_rssi_dbm(char* dev_stats, int length);
void get_disconncounter(char* dev_stats, int lenght);
void get_last_rtt(char* dev_stats, int len);
void get_average_rtt(char* dev_stats, int len);
#endif


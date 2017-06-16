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

#ifndef NETMGR_H
#define NETMGR_H

#include <stdint.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C"
{
#endif

#define MAX_SSID_SIZE  32
#define MAX_BSSID_SIZE 20
#define MAX_PWD_SIZE   64

/* 1 bigger than actual size for holding \0 */
typedef struct {
   char ssid[MAX_SSID_SIZE + 1];
   char bssid[MAX_BSSID_SIZE + 1];
   char pwd[MAX_PWD_SIZE + 1];
} netmgr_ap_config_t;

typedef struct autoconfig_plugin_s {
   struct autoconfig_plugin_s *next;
   const char                 *description;

   int  (*autoconfig_start)(void);
   void (*autoconfig_stop)(void);
   void (*config_result_cb)(int result, uint32_t ip);
} autoconfig_plugin_t;

void wifi_get_ip(char ips[16]);
int  netmgr_set_ap_config(netmgr_ap_config_t *config);
void netmgr_set_smart_config(autoconfig_plugin_t *plugin);

int netmgr_init(void);
void netmgr_deinit(void);
int netmgr_start(bool autoconfig);

#if defined(__cplusplus)
}
#endif

#endif /* NETMGR_H */

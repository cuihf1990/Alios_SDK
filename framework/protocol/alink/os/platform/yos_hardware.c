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

#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "platform_config.h"

char *platform_get_multicast_ifname(char *ifname, int ifname_size)
{
}

char *platform_wifi_get_mac(char mac_str[PLATFORM_MAC_LEN])
{
}

int platform_wifi_get_rssi_dbm(void)
{
    return 0;
}

uint32_t platform_wifi_get_ip(char ip_str[PLATFORM_IP_LEN])
{
    return 0;
}

char *platform_get_chipid(char chipid[PLATFORM_CID_LEN])
{
    return chipid;
}

char *platform_get_os_version(char os_ver[STR_SHORT_LEN])
{
    return os_ver;
}

int platform_config_write(const char *buffer, int length)
{
    return 0;
}

int platform_config_read(char *buffer, int length)
{
    return 0;
}

int platform_sys_net_is_ready(void)
{
    return 1;
}

void platform_sys_reboot(void)
{
    yos_reboot();
}

char *platform_get_module_name(char name_str[STR_SHORT_LEN])
{
    return name_str;
}

const char *platform_get_storage_directory(void)
{
    return NULL;
}

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

#ifndef _AWSS_H_
#define _AWSS_H_


extern char *zc_default_ssid;
extern char *zc_default_passwd;

#ifdef CONFIG_YWSS
#define DEFAULT_SSID    zc_default_ssid
#define DEFAULT_PASSWD  zc_default_passwd
#else
#define DEFAULT_SSID    ""
#define DEFAULT_PASSWD  ""
#endif

#define WLAN_CONNECTION_TIMEOUT_MS      (30 * 1000)

#endif

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

#include <stdlib.h>
#include "yos/kernel.h"
#include "yos/framework.h"
#include "aws_lib.h"
#include "zconfig_lib.h"
#include "awss.h"
#include "yos/list.h"
#include "yos/log.h"
#include "os.h"
#include "enrollee.h"

static void awss_notify(void *arg);

#define MODULE_NAME "awss"
static int delay_ms = 0;  /* start from 200ms */

int stopAwssConnecting = 0;
int awssFinished = 0;

int awss_start(void)
{
    awss_set_enrollee_token("default", strlen("default"));

    char ssid[OS_MAX_SSID_LEN] = { 0 }, passwd[OS_MAX_PASSWD_LEN] = { 0 };
    enum AWSS_AUTH_TYPE auth = AWSS_AUTH_TYPE_INVALID;
    enum AWSS_ENC_TYPE encry = AWSS_ENC_TYPE_INVALID;
    uint8_t bssid[OS_ETH_ALEN] = { 0 };
    uint8_t channel = 0;
    int ret;

    LOGI(MODULE_NAME,"awss version: %s", zconfig_lib_version());

    /* these params is useless, keep it for compatible reason */
    aws_start(NULL, NULL, NULL, NULL);

    ret = aws_get_ssid_passwd(&ssid[0], &passwd[0], &bssid[0],
            (char *)&auth, (char *)&encry, &channel);
    if (!ret)
	    LOGW(MODULE_NAME,"awss timeout!");

    aws_destroy();

    uint32_t startAwssConnectingTimestamp = yos_now() / 1000000;
    int tryCount = 0;
    do {
        if (stopAwssConnecting){
            break;
        }
        if (strcmp(ssid, DEFAULT_SSID) == 0){
            uint32_t now = yos_now() / 1000000;
            if ((0 != os_awss_get_connect_default_ssid_timeout_interval_ms()) &&
                (now - startAwssConnectingTimestamp > os_awss_get_connect_default_ssid_timeout_interval_ms())) {
                break;
            }
        }

        if (ssid[0]) {
            ret = os_awss_connect_ap(WLAN_CONNECTION_TIMEOUT_MS, ssid, passwd,
                    auth, encry, bssid, channel);
            if (tryCount < 9999) {
                tryCount++;
            }
            if (!ret) {
                LOGI(MODULE_NAME,"awss connect ssid:%s success", ssid);
                delay_ms = 0;
                yos_schedule_work(0,awss_notify,NULL,NULL,NULL);
                goto end;
            } else {
                LOGW(MODULE_NAME,"awss connect ssid:%s passwd:%s fail", ssid, passwd);
            }
        } else {
            strncpy(ssid, DEFAULT_SSID, sizeof(ssid));
            strncpy(passwd, DEFAULT_PASSWD, sizeof(passwd));
        }

        if (1 == tryCount){
            strncpy(ssid, DEFAULT_SSID, sizeof(ssid));
            strncpy(passwd, DEFAULT_PASSWD, sizeof(passwd));
        }
    } while (1);

end:
    awssFinished = 1;
    /* never reach here */
    return 0;
}

int awss_stop(void)
{
    //LOGT();

    stopAwssConnecting = 1;
    aws_destroy();
    //stop_awss_work();

    while(1){
        if (awssFinished) break;
        yos_msleep(100);
    }
    return 0;
}


extern int aws_notify_app_nonblock(void);

#define AWSS_NOTIFY_TIMES   (50)
static void awss_notify(void *arg)
{
    int ret = aws_notify_app_nonblock();

    delay_ms += 100;

    if (!ret)
        yos_schedule_work(delay_ms,awss_notify,NULL,NULL,NULL);
}

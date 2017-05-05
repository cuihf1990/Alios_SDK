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

#include "ysh.h"
#include <hal/hal.h>
#include <hal/platform.h>
#include <yoc/conf.h>
#include <yoc/log.h>
#include <yoc/framework.h>
#include <assert.h>

extern int  smart_config_start(void);
extern void net_conf_clear(void);

hal_flash_module_t *hal_flash_get_default_module(void);

#ifdef CONFIG_ALINK_EMBEDDED
extern int yoc_netconf_start_smartconfig(void);
#else
extern int smart_config_start(void);
#endif

static uint32_t cmd_test_func(char *buf, uint32_t len, cmd_item_t *item, cmd_info_t *info)
{
    ysh_stat_t             ret;
    hal_flash_module_t    *m;
    yoc_persistent_conf_t *pconf = yoc_pconf_get();

    if ((NULL != item->items[1]) &&
        (0 == strcmp(item->items[1], "help") || 0 == strcmp(item->items[1], "?"))) {
        snprintf(buf, len, "%s\r\n", info->help_info);
    } else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "sconfig")) {
#ifdef CONFIG_ALINK_EMBEDDED
        ret = yoc_netconf_start_smartconfig();
#else
        ret = smart_config_start();
#endif
        return ret;
    } else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "cssid")) {
        net_conf_clear();
        hal_time_msleep(100);

        if (hal_arch_reboot) {
            hal_arch_reboot();
        }

        return YUNOS_CMD_SUCCESS;
    } else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "reset")) {
        m = hal_flash_get_default_module();

        if (m && m->reset)
        { m->reset(m); }

#ifdef TFS_EMULATE
    } else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "setid2")) {
        pconf->emulate_id2_index = atoi(item->items[2]);
        hal_flash_conf_write(NULL, "id2_index", (unsigned char *) & (pconf->emulate_id2_index),
                             sizeof(pconf->emulate_id2_index));
        yoc_pconf_renew_magic();

        hal_time_msleep(100);

        if (hal_arch_reboot) {
            hal_arch_reboot();
        }

#endif
    } else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "setlog")) {
        pconf->log_level = atoi(item->items[2]);
        hal_flash_conf_write(NULL, "loglevel", (unsigned char *) & (pconf->log_level),
                             sizeof(pconf->log_level));
        yoc_pconf_renew_magic();

        hal_time_msleep(100);

        if (hal_arch_reboot) {
            hal_arch_reboot();
        }
    } else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "stress")) {
        pconf->in_stress_test = atoi(item->items[2]);
        hal_flash_conf_write(NULL, "stress_test", (unsigned char *) & (pconf->in_stress_test),
                             sizeof(pconf->in_stress_test));
        yoc_pconf_renew_magic();
        hal_time_msleep(100);

        if (hal_arch_reboot) {
            hal_arch_reboot();
        }
    } else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "setwifi")) {

        if (NULL == item->items[2] || NULL == item->items[3]) {
            printf("The username or password is null.\r\n");
            return YUNOS_CMD_SUCCESS;
        }
        strcpy(pconf->wifi.ssid, item->items[2]);
        strcpy(pconf->wifi.pwd, item->items[3]);
        pconf->ft.flags = DEVICE_ACTIVATED;

        hal_flash_conf_write(NULL, "wifi", (unsigned char *) & (pconf->wifi),
                             sizeof(yoc_persistent_conf_wifi_t));

        hal_flash_conf_write(NULL, "ft", (unsigned char *) & (pconf->ft),
                             sizeof(yoc_persistent_conf_factory_test_t));

        yoc_pconf_renew_magic();
        hal_time_msleep(100);

        if(hal_arch_reboot) {
            hal_arch_reboot();
        }
    } else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "conf")) {
        char *cat = item->items[2];
        char *value = item->items[3];

        if (!cat) {
            snprintf(buf, len, "test conf category [value]\r\n");
            return YUNOS_CMD_SUCCESS;
        }

        if (!value) {
            char res[32];
            memset(res, 0, sizeof(res));
            hal_flash_conf_read(NULL, cat, (unsigned char *)res, sizeof res);
            snprintf(buf, len, "%s\r\n", res);
        } else {
            int ret = hal_flash_conf_write(NULL, cat, (unsigned char *)value, strlen(value) + 1);
            snprintf(buf, len, "%s\r\n", ret < 0 ? "fail" : "success");
        }
    } else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "publish")) {
#if defined(CONFIG_YOC_URADAR_MESH) && !defined(CONFIG_YOC_CLOUD)
        int yoc_cloud_publish(const char *idsu, const char *payload);
        yoc_cloud_publish(item->items[2], item->items[3]);
#endif
#if (YUNOS_CONFIG_MM_DEBUG > 0)
    } else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "corrupt")) {
        uint8_t* mem = (uint8_t*)malloc(12);
        uint32_t i   = 0;
        if(NULL == mem) {
            printf("test corrupt malloc failed.\r\n");
        } else {
            for (i = 0; i < 20; i++) {
                *(mem + i) = 0;
            }
            if (YUNOS_MM_CORRUPT_ERR == check_mm_info_func()) {
                assert(0);
            }
        }
#endif
    } else if (NULL != item->items[1] && 0 == strcmp(item->items[1], "event")) {
        yoc_local_event_post(0x1000, 0, 0);
    } else {
        snprintf(buf, len, "%s\r\n", info->help_info);
    }

    return YUNOS_CMD_SUCCESS;
}

void ysh_reg_cmd_test(void)
{
    cmd_info_t *tmp = NULL;

    tmp = soc_mm_alloc(sizeof(cmd_info_t));

    if (tmp == NULL) {
        return;
    }

    tmp->cmd       = "test";
    tmp->info      = "test command.";
    tmp->help_info = "test :\r\n"
                     "\ttest corrupt          : test cmd fot memory corrupt.\r\n"
                     "\ttest assistant        : test assistant.\r\n";
    tmp->func      = cmd_test_func;

    ysh_register_cmd(tmp);

    return;
}


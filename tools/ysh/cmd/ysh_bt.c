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
#include "ysh_bt.h"

static uint32_t bt_func(char *buf, uint32_t len)
{
    int32_t i;
    int32_t size;
    int32_t plen = 0;
    void   *array[10];
    char  **strings;

    size = yoc_backtrace(array, 10);
    strings = yoc_backtrace_symbols(array, size);
    if (NULL == strings) {
        perror("backtrace_synbols");
        return YUNOS_CMD_FAILED;
    }

    plen += sprintf(buf + plen, "--------------------backtrace-----------------\n");

    for (i = 0; i < size; i++) {
        plen += snprintf(buf + plen, len - plen - 1, "%s\r\n", strings[i]);
        if (plen >= len) {
            return YUNOS_CMD_BUF_OUT;
        }
    }
    plen += sprintf(buf + plen, "---------------------------------------------\n");
    free((void*)strings);

    return YUNOS_CMD_SUCCESS;
}

uint32_t cmd_bt_func(char *buf, uint32_t len, cmd_item_t *item, cmd_info_t *info)
{
    ysh_stat_t ret;
    if ((NULL != item->items[1]) &&
        (0 == strcmp(item->items[1], "help") || 0 == strcmp(item->items[1], "?"))) {
        snprintf(buf, len, "%s\r\n", info->help_info);
        return YUNOS_CMD_SUCCESS;
    } else if (NULL == item->items[1]) {
        ret = bt_func(buf, len);
        return ret;
    } else {
        snprintf(buf, len, "%s\r\n", info->help_info);
        return YUNOS_CMD_SUCCESS;
    }
}

void ysh_reg_cmd_bt(void)
{
    cmd_info_t *tmp = NULL;

    tmp = yos_malloc(sizeof(cmd_info_t));

    if (tmp == NULL) {
        return;
    }

    tmp->cmd       = "bt";
    tmp->info      = "show the backtrace info.";
    tmp->help_info = "show the backtrace info.";
    tmp->func      = cmd_bt_func;

    ysh_register_cmd(tmp);

    return;
}


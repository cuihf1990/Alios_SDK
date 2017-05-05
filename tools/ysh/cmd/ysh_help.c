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
#include "ysh_help.h"

extern cmd_list_t *g_ysh_cmd_list;

static uint32_t help_func(char *buf, uint32_t len)
{
    uint32_t i    = 0;
    uint32_t plen = 0;

    cmd_list_t *tmp = g_ysh_cmd_list->next;
    cmd_info_t *cur = NULL;

    for (; tmp != 0; tmp = tmp->next) {
        cur = tmp->info;
        plen += snprintf(buf + plen, len - plen - 1, "[%u]%-15s : %s\r\n", (unsigned int)i++, cur->cmd, cur->info);
        if (plen >= len) {
            return YUNOS_CMD_BUF_OUT;
        }
    }

    return YUNOS_CMD_SUCCESS;
}

static uint32_t cmd_help_func(char *buf, uint32_t len, cmd_item_t *item, cmd_info_t *info)
{
    if ((NULL != item->items[1]) &&
        (0 == strcmp(item->items[1], "help") || 0 == strcmp(item->items[1], "?"))) {
        snprintf(buf, len, "%s\r\n", info->help_info);
        return YUNOS_CMD_SUCCESS;
    } else if (NULL == item->items[1]) {
        return help_func(buf, len);
    } else {
        snprintf(buf, len, "%s\r\n", info->help_info);
        return YUNOS_CMD_SUCCESS;
    }
}

void ysh_reg_cmd_help(void)
{
    cmd_info_t *tmp = NULL;

    tmp = soc_mm_alloc(sizeof(cmd_info_t));
    if (tmp == NULL) {
        return;
    }

    tmp->cmd       = "help";
    tmp->info      = "show the available command.";
    tmp->help_info = "show the available command.";
    tmp->func      = cmd_help_func;

    ysh_register_cmd(tmp);
}


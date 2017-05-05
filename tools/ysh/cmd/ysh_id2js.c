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
#include "ysh_id2js.h"

static uint32_t cmd_id2js_func(char *buf, uint32_t len, cmd_item_t *item, cmd_info_t *info);

static cmd_info_t id2js_cmd = {
    .cmd       = "js",
    .info      = "js command",
    .help_info = "js [javascript code]",
    .func      = cmd_id2js_func,
};

void jshell_input(char *buf, uint16_t len, const char *code);

static uint32_t cmd_id2js_func(char *buf, uint32_t len, cmd_item_t *item, cmd_info_t *info)
{
    if (item->items[1] != NULL) {
        jshell_input(buf, len, item->items[1]);
    } else {
        snprintf(buf, len, "%s\r\n", info->help_info);
    }

    return YUNOS_CMD_SUCCESS;
}

void ysh_reg_cmd_id2js(void)
{
    ysh_register_cmd(&id2js_cmd);
}


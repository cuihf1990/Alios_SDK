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
#include "ysh_port.h"

static uint32_t cmd_ur_func(char *buf, uint32_t len, cmd_item_t *item, cmd_info_t *info);

static cmd_info_t ur_cmd = {
    .cmd       = "ur",
    .info      = "uradar command",
    .help_info = "ur [subcommands]",
    .func      = cmd_ur_func,
};

void ur_cli_input(char *buf, uint16_t length);

static uint32_t cmd_ur_func(char *buf, uint32_t len, cmd_item_t *item, cmd_info_t *info)
{
    while (1) {
        int ret = soc_term_gets(NULL, buf, len);
        if (ret <= 0)
            continue;
        if (strncmp(buf, "q", len) == 0)
            break;
        if (strncmp(buf, "quit", len) == 0)
            break;
        ur_cli_input(buf, ret);
    }

    return YUNOS_CMD_SUCCESS;
}

void ysh_reg_cmd_urmesh(void)
{
    ysh_register_cmd(&ur_cmd);
}


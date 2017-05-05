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

#ifndef YSH_H
#define YSH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include <k_api.h>

#define YSH_CMD_LEN_MAX      80

#define YSH_EXIT_SUCCESS     0
#define YSH_EXIT_FAILURE     1

#define YSH_CMD_OUT_BUF_SIZE 2048u

#define YSH_HISTORY_CMD_PREV "AA"
#define YSH_HISTORY_CMD_NEXT "BB"

#define YSH_CMD_MAX_ARG_NUM 4

typedef struct {
    char *items[YSH_CMD_MAX_ARG_NUM + 1];
} cmd_item_t;

typedef struct cmd_info_s {
    const char *cmd;
    const char *info;
    const char *help_info;
    uint32_t  (*func)(char *buf, uint32_t len, cmd_item_t *item, struct cmd_info_s *info);
} cmd_info_t;

typedef struct cmd_list_s {
    cmd_info_t        *info;
    struct cmd_list_s *next;
} cmd_list_t;

typedef uint32_t (*cmd_func_t)(char *buf, uint32_t len, cmd_item_t *item, cmd_info_t *info);

uint32_t ysh_register_cmd(cmd_info_t *info);

typedef enum {
    YUNOS_CMD_SUCCESS = 0u,
    YUNOS_CMD_FAILED,
    YUNOS_CMD_BUF_OUT,
    YUNOS_CMD_NOT_FOUND,
    YUNOS_CMD_NO_OUTPUT,

    YUNOS_CMD_HISTORY_NORMAL = 100u,
    YUNOS_CMD_HISTORY_LOOKING,
    YUNOS_CMD_HISTORY_FOUND,
} ysh_stat_t;

typedef struct ysh_ctrl_tbl_s {
    int32_t (*init)(void);
    int32_t (*puts)(struct ysh_ctrl_tbl_s *ctrl_tbl, char *buf, uint32_t len);
    int32_t (*gets)(struct ysh_ctrl_tbl_s *ctrl_tbl, char *buf, uint32_t len);
    void    (*exit)(struct ysh_ctrl_tbl_s *ctrl_tbl, int32_t stat);
} ysh_ctrl_tbl_t;

typedef struct ysh_stdio_s {
    int32_t        fd;
    char           cmd[YSH_CMD_LEN_MAX];
    char           outbuf[YSH_CMD_OUT_BUF_SIZE];
    ysh_ctrl_tbl_t func_tbl;
} ysh_stdio_t;

#ifdef __cplusplus
}
#endif

#endif /* YSH_H */


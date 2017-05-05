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
#include "ysh_history.h"
#include "ysh_register.h"

char *g_exe_cmd_failed = " Execute Command Faild.\r\n";
char *g_cmd_not_found  = " Command Not FOUND.\r\n";
char *g_greeting       = "Welcome to Shell.\r\n";
char *g_prompt         = "YSH> ";

#define YSH_TASK_STACK_SIZE 512

ktask_t     g_ysh_task;
cpu_stack_t g_ysh_task_stack[YSH_TASK_STACK_SIZE];

cmd_list_t *g_ysh_cmd_list;
ysh_stdio_t g_ysh_only_term;

char *g_history_cmd;

uint32_t ysh_register_cmd(cmd_info_t *info)
{
    cmd_list_t *cur = NULL;
    cmd_list_t *tmp = NULL;

    if (NULL == info) {
        return 0;
    }

    tmp = soc_mm_alloc(sizeof(cmd_list_t));
    if (tmp == NULL) {
        return 0;
    }

    tmp->info            = info;
    cur                  = g_ysh_cmd_list->next;
    g_ysh_cmd_list->next = tmp;
    tmp->next            = cur;

    return 1;
}

static void parse_cmd(char *cmd, cmd_item_t *item)
{
    uint32_t index = 0;
    char    *tmp   = NULL;

    item->items[index++] = strtok(cmd, " ");

    for (; index < YSH_CMD_MAX_ARG_NUM + 1; index++) {
        if ((tmp = strtok(NULL, " ")) == NULL) {
            break;
        }

        item->items[index] = tmp;
    }
}

static uint32_t ysh_parse(ysh_ctrl_tbl_t *ctrl_tbl, char *cmd)
{
    uint32_t     ret   = 0;
    cmd_info_t  *cur   = NULL;
    ysh_stdio_t *state = (ysh_stdio_t *)ctrl_tbl;
    cmd_list_t  *tmp   = g_ysh_cmd_list->next;
    cmd_item_t   item  = {{0, 0, 0, 0, 0}};

    if (0 == strcmp(cmd, "\r\n")) {
        return 1;
    }

    if (0 == strcmp(cmd, YSH_HISTORY_CMD_PREV)) {
        if (YUNOS_CMD_HISTORY_NORMAL == ysh_history_get_state()) {
            ysh_history_set_state(YUNOS_CMD_HISTORY_LOOKING);
            g_history_cmd = ysh_history_cmd_get_cur();
        } else {
            g_history_cmd = ysh_history_cmd_get_prev();
        }

        if (YUNOS_CMD_HISTORY_NORMAL == ysh_history_get_state()) {
            ysh_history_set_state(YUNOS_CMD_HISTORY_LOOKING);
        }

        return  YUNOS_CMD_NO_OUTPUT;
    } else if (0 == strcmp(cmd,YSH_HISTORY_CMD_NEXT)) {
        g_history_cmd = ysh_history_cmd_get_next();

        if (YUNOS_CMD_HISTORY_NORMAL == ysh_history_get_state()) {
            ysh_history_set_state(YUNOS_CMD_HISTORY_LOOKING);
        }
        return  YUNOS_CMD_NO_OUTPUT;
    }

    ysh_history_cmd_add(cmd);

    parse_cmd(cmd, &item);

    for (; tmp != 0; tmp = tmp->next) {
        cur = tmp->info;

        if ((NULL != item.items[0]) && (strcmp(item.items[0], cur->cmd) == 0)) {
            ret = cur->func(state->outbuf, YSH_CMD_OUT_BUF_SIZE, &item, cur);
            return ret;
        }
    }

    return YUNOS_CMD_NOT_FOUND;
}

static ysh_stdio_t *ysh_new_term(void)
{
    ysh_stdio_t *tmp = &g_ysh_only_term;

    memset(tmp, 0, sizeof(ysh_stdio_t));
    tmp->func_tbl.init = soc_term_init;
    tmp->func_tbl.puts = soc_term_puts;
    tmp->func_tbl.gets = soc_term_gets;
    tmp->func_tbl.exit = soc_term_exit;
    tmp->fd            = soc_term_init();

    return tmp;
}

static uint32_t ysh_session(ysh_stdio_t *pstate)
{
    uint8_t    ret  = 0;
    ysh_stat_t stat;

    pstate->func_tbl.puts((ysh_ctrl_tbl_t *)pstate, g_greeting, strlen(g_greeting));

    for (;;) {

        pstate->func_tbl.puts((ysh_ctrl_tbl_t *)pstate, g_prompt, strlen(g_prompt));

        if (NULL != g_history_cmd) {
            pstate->func_tbl.puts((ysh_ctrl_tbl_t *)pstate, g_history_cmd, strlen(g_history_cmd));
        }

        ret = pstate->func_tbl.gets((ysh_ctrl_tbl_t *)pstate, pstate->cmd, YSH_CMD_LEN_MAX);
        if (ret > 0) {
            stat = ysh_parse((ysh_ctrl_tbl_t *)pstate, pstate->cmd);

            if (YUNOS_CMD_FAILED == stat) {
                pstate->func_tbl.puts((ysh_ctrl_tbl_t *)pstate, g_exe_cmd_failed,
                                      strlen(g_exe_cmd_failed));
            } else if (YUNOS_CMD_SUCCESS == stat) {
                pstate->func_tbl.puts((ysh_ctrl_tbl_t *)pstate, pstate->outbuf,
                                      YSH_CMD_OUT_BUF_SIZE);
            } else if (YUNOS_CMD_NOT_FOUND == stat) {
                pstate->func_tbl.puts((ysh_ctrl_tbl_t *)pstate, g_cmd_not_found,
                                      strlen(g_cmd_not_found));
            }
        } else if (ret < 0) {
            return YSH_EXIT_FAILURE;
        } else {
            if (YUNOS_CMD_HISTORY_LOOKING == ysh_history_get_state()) {
                ysh_history_set_state(YUNOS_CMD_HISTORY_NORMAL);
                strncpy(pstate->cmd, g_history_cmd, strlen(g_history_cmd));
                stat = ysh_parse((ysh_ctrl_tbl_t *)pstate, pstate->cmd);
                if (YUNOS_CMD_FAILED == stat) {
                    pstate->func_tbl.puts((ysh_ctrl_tbl_t *)pstate, g_exe_cmd_failed,
                                          strlen(g_exe_cmd_failed));
                } else if (YUNOS_CMD_SUCCESS == stat) {
                    pstate->func_tbl.puts((ysh_ctrl_tbl_t *)pstate, pstate->outbuf,
                                          YSH_CMD_OUT_BUF_SIZE);
                } else if (YUNOS_CMD_NOT_FOUND == stat) {
                    pstate->func_tbl.puts((ysh_ctrl_tbl_t *)pstate, g_cmd_not_found,
                                          strlen(g_cmd_not_found));
                }
                g_history_cmd = NULL;
            }
        }

        memset(pstate->cmd, 0, YSH_CMD_LEN_MAX);
        memset(pstate->outbuf, 0, YSH_CMD_OUT_BUF_SIZE);
    }

    return YSH_EXIT_SUCCESS;
}

uint32_t ysh_init_cmd_list(void)
{
    g_ysh_cmd_list = soc_mm_alloc(sizeof(cmd_list_t));
    if (g_ysh_cmd_list == NULL) {
        return YUNOS_NO_MEM;
    }

    g_ysh_cmd_list->info = NULL;
    g_ysh_cmd_list->next = NULL;

    return YUNOS_SUCCESS;
}

void ysh_task_entry(void *arg)
{
    uint32_t     ret;
    ysh_stdio_t *pstate = NULL;

    ysh_register();

    pstate = ysh_new_term();

    ret = ysh_session(pstate);

    pstate->func_tbl.exit(&pstate->func_tbl, ret);
}

int32_t ysh_init(void)
{
    static int inited;

    kstat_t ret;

    if (inited) {
        return 0;
    }

    ret = ysh_init_cmd_list();
    if (ret != YUNOS_SUCCESS) {
        printf("ysh int cmd list failed.\r\n");
        return 1;
    }

    ysh_history_cmd_list_init();

    inited = 1;

    return 0;
}

void ysh_task_start(void)
{
    yunos_task_create(&g_ysh_task, "ysh_task", NULL, 9,
                      0, g_ysh_task_stack, YSH_TASK_STACK_SIZE,
                      ysh_task_entry, 1);
}


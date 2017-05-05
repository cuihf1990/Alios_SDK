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
#include "ysh_history.h"

ysh_history_list_head g_history_cmd_head;

void ysh_history_cmd_printf(void);

uint32_t ysh_history_cmd_list_init(void)
{
    klist_init(&g_history_cmd_head.head);
    g_history_cmd_head.state = YUNOS_CMD_HISTORY_NORMAL;

    return 1;
}

uint32_t ysh_history_cmd_add(char *cmd)
{
    uint32_t              len       = 0;
    char                 *ptmp_s    = NULL;
    ysh_history_cmd_list *ptmp_node = NULL;

    if (NULL == cmd) {
        printf("ysh_history_cmd_add cmd is NULL.\r\n");
        return 0;
    }

    ptmp_node = malloc(sizeof(ysh_history_cmd_list));
    if (NULL == ptmp_node) {
        printf("ysh_history_cmd_add malloc node failed.\r\n");
        return 0;
    }

    len = strlen(cmd);
    ptmp_s = malloc(len + 1);
    if (NULL == ptmp_s) {
        free(ptmp_node);
        printf("ysh_history_cmd_add malloc string failed.\r\n");
        return 0;
    }
    strncpy(ptmp_s, cmd, len);
    ptmp_s[len] = 0;

    ptmp_node->cmd = ptmp_s;
    klist_add(&g_history_cmd_head.head, &ptmp_node->list);
    g_history_cmd_head.cur = &ptmp_node->list;

    return 1;
}

void ysh_history_cmd_printf(void)
{
    klist_t *tmp;
    klist_t *head = &g_history_cmd_head.head;
    klist_t *end  = head;
    uint32_t i    = 0;

    ysh_history_cmd_list *ptmp_node = NULL;

    for (tmp = head->next; tmp != end; tmp = tmp->next) {
        ptmp_node = yunos_list_entry(tmp, ysh_history_cmd_list, list);
        printf("[%d]:%s\n", (int)i, ptmp_node->cmd);
        i++;
    }
}

char *ysh_history_cmd_get_cur(void)
{
    ysh_history_cmd_list *ptmp_node = NULL;

    if (NULL != g_history_cmd_head.cur && g_history_cmd_head.cur != &g_history_cmd_head.head) {
        ptmp_node = yunos_list_entry(g_history_cmd_head.cur, ysh_history_cmd_list, list);
        return ptmp_node->cmd;
    }

    return NULL;
}

char *ysh_history_cmd_get_prev(void)
{
    if (NULL != g_history_cmd_head.cur) {
        g_history_cmd_head.cur = g_history_cmd_head.cur->next;
        return ysh_history_cmd_get_cur();
    }

    return NULL;
}

char *ysh_history_cmd_get_next(void)
{
    if (NULL != g_history_cmd_head.cur) {
        g_history_cmd_head.cur = g_history_cmd_head.cur->prev;
        return ysh_history_cmd_get_cur();
    }

    return NULL;
}

ysh_stat_t ysh_history_get_state(void)
{
    return  g_history_cmd_head.state;
}

void ysh_history_set_state( ysh_stat_t state)
{
    g_history_cmd_head.state = state;
}


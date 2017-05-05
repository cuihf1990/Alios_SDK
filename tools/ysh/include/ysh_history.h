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

#ifndef YSH_HISTORY_H
#define YSH_HISTORY_H

typedef struct {
    klist_t list;
    char   *cmd;
}ysh_history_cmd_list;

typedef struct {
    klist_t  head;
    klist_t *cur;
    uint32_t state;
}ysh_history_list_head;

uint32_t ysh_history_cmd_list_init(void);
uint32_t ysh_history_cmd_add(char *cmd);

char *ysh_history_cmd_get_cur(void);
char *ysh_history_cmd_get_prev(void);
char *ysh_history_cmd_get_next(void);

ysh_stat_t ysh_history_get_state(void);
void       ysh_history_set_state(ysh_stat_t state);

#endif

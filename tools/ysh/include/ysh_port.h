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

#ifndef YSH_PORT_H
#define YSH_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

int32_t soc_term_init(void);
int32_t soc_term_puts(ysh_ctrl_tbl_t *ctrl_tbl, char *buf, uint32_t len);
int32_t soc_term_gets(ysh_ctrl_tbl_t *ctrl_tbl, char *buf, uint32_t len);
void    soc_term_release(ysh_ctrl_tbl_t *ctrl_tbl);
void    soc_term_exit(ysh_ctrl_tbl_t *ctrl_tbl, int32_t stat);

#ifdef __cplusplus
}
#endif

#endif /* YSH_PORT_H */


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

#include "ysh_register.h"

#include "ysh_help.h"

#ifdef CONFIG_YSH_CMD_DUMPSYS
#include "ysh_dumpsys.h"
#endif

#ifdef CONFIG_YSH_CMD_BT
#include "ysh_bt.h"
#endif

#ifdef CONFIG_YSH_CMD_TEST
#include "ysh_test.h"
#endif

#ifdef CONFIG_YOS_MESH
#include "ysh_urmesh.h"
#endif

#ifdef CONFIG_YOC_ID2JS
#include "ysh_id2js.h"
#endif

void ysh_register(void)
{
    ysh_reg_cmd_help();

#ifdef CONFIG_YSH_CMD_DUMPSYS
    ysh_reg_cmd_dumpsys();
#endif

#ifdef CONFIG_YSH_CMD_BT
    ysh_reg_cmd_bt();
#endif

#ifdef CONFIG_YSH_CMD_TEST
    ysh_reg_cmd_test();
#endif

#ifdef CONFIG_YOS_MESH
    ysh_reg_cmd_urmesh();
#endif

#ifdef CONFIG_YOC_ID2JS
    ysh_reg_cmd_id2js();
#endif
}


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

#ifndef UR_CLI_H
#define UR_CLI_H

#include "umesh_types.h"

enum {
    CMD_LINE_SIZE = 128,
};

typedef struct command_s {
    const char *name;
    void (*function)(int argc, char *argv[]);
} cli_command_t;

extern int g_cli_silent;
ur_error_t mesh_cli_init(void);

#endif  /* UR_CLI_H */

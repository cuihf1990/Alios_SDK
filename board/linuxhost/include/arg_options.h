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

#ifndef ARG_OPTIONS_H
#define ARG_OPTIONS_H

#include <stdbool.h>

enum {
    MODE_DFL = 1,
    MODE_YTS,
    MODE_OTA,
    MODE_MESH_NODE,
    MODE_MESH_MASTER,

    MODE_NONE,
};

typedef struct {
    int    argc;
    char **argv;
    int    mode;
#ifdef TFS_EMULATE
    int    id2_index;
#endif
    int    log_level;

    struct {
        bool enable;
        bool tapif;
    } lwip;

    struct {
        bool enable;
    } mesh;

    struct {
        bool enable;
    } dda;

    struct {
        bool enable;
    } ddm;

    struct {
        bool enable;
    } alink;
} options_t;

void parse_options(options_t *config);

#endif


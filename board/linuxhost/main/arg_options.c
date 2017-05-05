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

#include <stdlib.h>
#include <string.h>
#include <arg_options.h>
#include <yos/log.h>

static void shift_argv(options_t *options, int i)
{
    options->argc --;
    while (i < options->argc) {
        options->argv[i] = options->argv[i+1];
        i ++;
    }
}

void parse_options(options_t *options)
{
    char      **argv = options->argv;
    const char *mode = options->argc > 1 ? argv[1] : "";
    int         i;

    if (strlen(mode) == 0) {
        options->mode = MODE_DFL;
    } else if (strcmp(mode, "yts") == 0) {
        options->mode = MODE_YTS;
        options->lwip.tapif = false;
    } else if(strcmp(argv[1], "ota") == 0) {
        options->mode = MODE_OTA;
    } else if (strcmp(mode, "--mesh-node") == 0) {
        options->mode = MODE_MESH_NODE;
        options->lwip.tapif = false;
    } else if (strcmp(mode, "--mesh-master") == 0) {
        options->mode = MODE_MESH_MASTER;
        options->lwip.enable = false;
        options->lwip.tapif = false;
    } else {
        options->mode = MODE_NONE;
    }

    for (i = 0; i < options->argc;) {
        if (!strcmp(argv[i], "--tapif")) {
            options->lwip.tapif = true;
            shift_argv(options, i);
            continue;
        }

        if (!strcmp(argv[i], "--no-tapif")) {
            options->lwip.tapif = false;
            shift_argv(options, i);
            continue;
        }

        if (!strcmp(argv[i], "--log")) {
            shift_argv(options, i);
            if (i >= options->argc) {
                options->log_level = YOS_LL_DEBUG;
                continue;
            }

            const char *ll = argv[i];
            if (strcmp(ll, "fatal") == 0)
                options->log_level = YOS_LL_FATAL;
            else if (strcmp(ll, "error") == 0)
                options->log_level = YOS_LL_ERROR;
            else if (strcmp(ll, "warn") == 0)
                options->log_level = YOS_LL_WARN;
            else if (strcmp(ll, "info") == 0)
                options->log_level = YOS_LL_INFO;
            else if (strcmp(ll, "debug") == 0)
                options->log_level = YOS_LL_DEBUG;
            shift_argv(options, i);
            continue;
        }

        if (strcmp(argv[i], "-i")) {
            i ++;
            continue;
        }

        if (i + 1 >= options->argc) {
            i ++;
            continue;
        }

#ifdef TFS_EMULATE
        options->id2_index = atoi(argv[i + 1]);
#endif
        shift_argv(options, i);
        shift_argv(options, i);
        break;
    }
}


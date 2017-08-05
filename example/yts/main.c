/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
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

#include <yts.h>
#include <dda.h>

int application_start(int argc, char **argv)
{
    const char *mode = argc > 1 ? argv[1] : "";

    if (strcmp(mode, "--mesh-node") == 0) {
        dda_enable(atoi(argv[argc-1]));
        dda_service_init();
        dda_service_start();
        return 0;
    }
    else if (strcmp(mode, "--mesh-master") == 0) {
        ddm_run(argc, argv);
        return 0;
    }

    yts_run(argc, argv);
    exit(0);
}


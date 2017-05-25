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

#include <stdio.h>
#include <yos/kernel.h>
#include <yos/framework.h>
#include <yos/network.h>
#include <dda.h>
#include <ysh.h>

int application_start(int argc, char **argv)
{
    const char *mode = argc > 1 ? argv[1] : "";

    ysh_init();
    ysh_task_start();

    if (strcmp(mode, "--mesh-node") == 0) {
#ifdef CONFIG_YOS_DDA
        dda_enable(atoi(argv[argc-1]));
        dda_service_init();
        dda_service_start();
#endif
    }
    else if (strcmp(mode, "--mesh-master") == 0) {
#ifdef CONFIG_YOS_DDM
        ddm_run(argc, argv);
#endif
    }

    return 0;
}

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

#include <yos/kernel.h>
#include <yos/framework.h>
#include <vfs.h>
#include <kvmgr.h>
#include <vflash.h>

#include "yos/cli.h"

#ifdef MESH_GATEWAY_SERVICE
#include "gateway_service.h"
#endif

extern void ota_service_init(void);

int yos_framework_init(void)
{
#ifdef MESH_GATEWAY_SERVICE
    gateway_service_init();
#endif

    ota_service_init();

    return 0;
}


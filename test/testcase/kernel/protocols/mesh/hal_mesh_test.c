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

#include <yunit.h>
#include <yos/framework.h>
#include <yos/kernel.h>

#include "umesh_hal.h"

void test_hal_mesh_case(void)
{
    hal_umesh_get_bcast_mtu(NULL);
    hal_umesh_get_ucast_mtu(NULL);
    hal_umesh_get_channel(NULL);
    hal_umesh_get_chnlist(NULL, NULL);
    hal_umesh_set_txpower(NULL, 2);
    hal_umesh_get_txpower(NULL);
    hal_umesh_get_extnetid(NULL, NULL);
    hal_umesh_get_stats(NULL);
}

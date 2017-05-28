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
    hal_ur_mesh_start_beacons(NULL, NULL, NULL, 10);
    hal_ur_mesh_stop_beacons(NULL);
    hal_ur_mesh_get_bcast_mtu(NULL);
    hal_ur_mesh_get_ucast_mtu(NULL);
    hal_ur_mesh_get_bcast_channel(NULL);
    hal_ur_mesh_get_bcast_chnlist(NULL, NULL);
    hal_ur_mesh_get_ucast_channel(NULL);
    hal_ur_mesh_get_ucast_chnlist(NULL, NULL);
    hal_ur_mesh_set_txpower(NULL, 2);
    hal_ur_mesh_get_txpower(NULL);
    hal_ur_mesh_get_meshnetid(NULL);
    hal_ur_mesh_get_stats(NULL);
}

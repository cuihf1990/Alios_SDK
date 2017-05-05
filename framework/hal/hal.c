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

#include <hal/hal.h>
#include <hal/mesh.h>

int yoc_hal_init()
{
    int err = 0;

    if ((err = hal_wifi_init()) != 0) {
        return err;
    }

    if ((err = hal_flash_init()) != 0) {
        return err;
    }

    if ((err = hal_ur_mesh_init()) != 0) {
        return err;
    }

    if ((err = hal_sensor_init()) != 0) {
        return err;
    }

    return err;
}


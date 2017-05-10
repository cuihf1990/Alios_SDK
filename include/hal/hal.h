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

/**
 * @file hal/hal.h
 * @brief The Hardware Abstraction Layer API for YOC
 * @version since 5.5.0
 */

#ifndef YOC_HAL_H
#define YOC_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <hal/base.h>

#include <hal/time.h>

#include <hal/random.h>

#include <hal/sensor.h>
/*
 * @brief Main
 */
int yoc_hal_init(void);
void __attribute__((weak)) hal_arch_reboot(void);

#ifdef __cplusplus
}
#endif

#endif /* YOC_HAL_H */


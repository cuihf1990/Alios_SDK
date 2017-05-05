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

#ifndef YOC_HAL_PLAT_FORM_H
#define YOC_HAL_PLAT_FORM_H

/**
 * Platform need to implement it
 * if defined HAL_ARCH_USE_COUNTER
 */

/**
 * This function will get time-counter
 * @return counter of time in microsecond when system startup/reboot(zero)
 */
unsigned long hal_arch_get_time_counter(void);

/**
 * This function will sleep for a time
 * @param[in] ms  time to sleep
 * @return no return
 */
void hal_arch_time_msleep(int ms);

/**
 * This function will generate random data
 * @param[out] buf  random data to save
 * @param[in] len  random data length
 * @return 0 is ok, other is error
 */
int hal_arch_random_generate(uint8_t *buf, size_t len);

/**
 * This function will feed random seed
 * @param[in] seed  random seed
 * @return 0 is ok, other is error
 */
int hal_arch_random_seed(uint32_t seed);

#endif /* YOC_HAL_PLAT_FORM_H */

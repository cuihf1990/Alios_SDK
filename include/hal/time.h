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
 * @file hal/time.h
 * @brief TIME HAL API for YOC
 * @version since 5.5.0
 */

#ifndef HAL_TIME_H
#define HAL_TIME_H

struct timeval;

/**
 * This function will refresh base time
 * @return no return
 */
void hal_time_refresh_base(void);

/**
 * This function will modify base time
 * @param[in] base  base_time
 * @return no return
 */
void hal_time_set_base(unsigned long long base);

/**
 * This function will get the time
 * @param[out] tv  pointer to the timeval
 * @param[out] tz  pointer to the time zone
 * @return no return
 */
void hal_time_gettimeofday(struct timeval *tv, void *tz);

/**
 * This function will sleep for a time
 * @param[in] ms  time to sleep
 * @return no return
 */
void hal_time_msleep(int ms);

/**
 * This function will get the base time
 * @return the base time
 */
unsigned long long hal_time_get(void);

#endif /* HAL_TIME_H */


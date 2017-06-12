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
#ifndef OTA_PLATFORM_OS_H_
#define OTA_PLATFORM_OS_H_
#include<stdint.h>

void *ota_malloc(uint32_t size);

void ota_free(void *ptr);

void *ota_mutex_init(void);

void ota_mutex_lock(void *mutex);

void ota_mutex_unlock(void *mutex);

void ota_mutex_destroy(void *mutex);

void *ota_semaphore_init(void);

int8_t ota_semaphore_wait(void *sem, uint32_t timeout_ms);

void ota_semaphore_post(void *sem);

void ota_semaphore_destroy(void *sem);

uint32_t ota_get_time_ms(void);

int8_t ota_thread_create(const char *name, 
     void *(*start_routine)(void *), void *arg, int stack_size, int prio);

void ota_thread_exit(void *thread);

void ota_reboot(void);
#endif 

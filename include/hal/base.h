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
 * @file hal/base.h
 * @brief basic data structures for HAL
 * @version since 1.0.0
 */

#ifndef HAL_BASE_H
#define HAL_BASE_H

#include <yos/list.h>

/**
 * @brief HAL common error code
 */
enum {
    HAL_ERR_ARG = -4096,
    HAL_ERR_CAP,
};

/**
 * @brief HAL Module define
 */
typedef struct {
    dlist_t      list;
    int          magic;
    const char  *name;
    void        *priv_dev; /* Driver may want to describe it */
} hal_module_base_t;

#endif


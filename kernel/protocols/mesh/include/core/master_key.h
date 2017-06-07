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

#ifndef UMESH_MASTER_KEY_H
#define UMESH_MASTER_KEY_H

#include "core/topology.h"
#include "hal/interface_context.h"

enum {
    PINCODE_METHOD = 0,
    ID2_METHOD = 1,

    MAX_METHOD_INDEX = 1,
};

typedef void (*master_key_updated_t)(ur_error_t result);
typedef void (*master_key_handler_t)(ur_error_t result, const uint8_t *master_key,
                                     uint8_t size);

typedef struct master_key_method_s {
    slist_t                    next;
    const char                 *name;
    ur_error_t (*master_key_request_start)(master_key_handler_t handler);
    ur_error_t (*master_key_request_stop)(void);
} master_key_method_t;

ur_error_t master_key_init(master_key_updated_t updated);
ur_error_t master_key_deinit(void);
ur_error_t master_key_request_start(void);
ur_error_t master_key_request_stop(void);
ur_error_t master_key_set_value(const uint8_t *key, uint8_t size);
ur_error_t master_key_set_method(master_key_method_t *method);
ur_error_t master_key_set_method_index(uint8_t index);

#endif /* UMESH_MASTER_KEY_H */

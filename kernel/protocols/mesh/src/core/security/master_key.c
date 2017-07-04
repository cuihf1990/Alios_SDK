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

#include "core/master_key.h"
#include "core/keys_mgr.h"

YOS_SLIST_HEAD(g_methods_list);
typedef struct master_key_context_s {
    slist_t *methods;
    uint8_t method_index;
    master_key_updated_t updated;
} master_key_context_t;

static master_key_context_t g_master_key_context;
static master_key_method_t g_def_method;

static void master_key_handler(ur_error_t result, const uint8_t *master_key,
                               uint8_t size)
{
    set_master_key(master_key, size);
    g_master_key_context.updated(result);
}

ur_error_t master_key_init(master_key_updated_t updated)
{
    memset(&g_master_key_context, 0, sizeof(g_master_key_context));
    g_master_key_context.methods = &g_methods_list;
    g_master_key_context.updated = updated;
    g_master_key_context.method_index = PINCODE_METHOD;
    master_key_set_method(&g_def_method);
    return UR_ERROR_NONE;
}

ur_error_t master_key_deinit(void)
{
    master_key_method_t *method;

    while(!slist_empty(g_master_key_context.methods)) {
        method = slist_first_entry(g_master_key_context.methods, master_key_method_t, next);
        slist_del(&method->next, g_master_key_context.methods);
    }

    return UR_ERROR_NONE;
}

ur_error_t master_key_request_start(void)
{
    ur_error_t error;
    uint8_t index = PINCODE_METHOD;
    master_key_method_t *method = NULL;

    slist_for_each_entry(g_master_key_context.methods, method, master_key_method_t,
                         next) {
        if (index == g_master_key_context.method_index) {
            break;
        }
    }

    if (method == NULL) {
        return UR_ERROR_FAIL;
    }

    error = method->master_key_request_start(master_key_handler);
    return error;
}

ur_error_t master_key_request_stop(void)
{
    ur_error_t error;
    uint8_t index = PINCODE_METHOD;
    master_key_method_t *method = NULL;

    slist_for_each_entry(g_master_key_context.methods, method, master_key_method_t,
                         next) {
        if (index == g_master_key_context.method_index) {
            break;
        }
    }

    if (method == NULL) {
        return UR_ERROR_FAIL;
    }

    error = method->master_key_request_stop();
    return error;
}

ur_error_t master_key_set_value(const uint8_t *key, uint8_t size)
{
    if (g_master_key_context.method_index != PINCODE_METHOD) {
        return UR_ERROR_FAIL;
    }

    set_master_key(key, size);
    return UR_ERROR_NONE;
}

ur_error_t master_key_set_method(master_key_method_t *method)
{
    if (method == NULL) {
        return UR_ERROR_PARSE;
    }

    slist_add(&method->next, g_master_key_context.methods);
    return UR_ERROR_NONE;
}

ur_error_t master_key_set_method_index(uint8_t index)
{
    if (index > MAX_METHOD_INDEX) {
        return UR_ERROR_FAIL;
    }

    g_master_key_context.method_index = index;
    return UR_ERROR_NONE;
}

ur_error_t def_master_key_request_start(master_key_handler_t handler)
{
    const uint8_t *key = NULL;
    uint8_t size = 0;

    key = get_master_key(&size);
    if (key == NULL || size == 0) {
        return UR_ERROR_FAIL;
    }

    handler(UR_ERROR_NONE, key, size);
    return UR_ERROR_NONE;
}

ur_error_t def_master_key_request_stop(void)
{
    return UR_ERROR_NONE;
}

static master_key_method_t g_def_method = {
    .name = "def_master_key",
    .master_key_request_start = def_master_key_request_start,
    .master_key_request_stop = def_master_key_request_stop,
};

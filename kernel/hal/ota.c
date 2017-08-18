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

#include <stdio.h>
#include <hal/base.h>
#include <hal/ota.h>

static hal_ota_module_t *ota_module;

hal_ota_module_t *hal_ota_get_default_module(void)
{
    return ota_module;
}

void hal_ota_register_module(hal_ota_module_t *module)
{
    ota_module = module;
}


hal_stat_t hal_ota_init(void)
{
    return ota_module->init(ota_module, NULL);
}

hal_stat_t hal_ota_write(hal_ota_module_t *m, volatile uint32_t* off_set, uint8_t* in_buf ,uint32_t in_buf_len)
{
    if (m == NULL) {
        m = hal_ota_get_default_module();
    }

    if (m != NULL && m->ota_write != NULL) {
        return m->ota_write(m, off_set, in_buf, in_buf_len);
    }

    return 0;
}

hal_stat_t hal_ota_read(hal_ota_module_t *m, volatile uint32_t* off_set, uint8_t* out_buf, uint32_t out_buf_len)
{
    if (m == NULL) {
        m = hal_ota_get_default_module();
    }

    if (m != NULL && m->ota_read != NULL) {
        return m->ota_read(m, off_set, out_buf, out_buf_len);
    }

    return 0;
}

hal_stat_t hal_ota_set_boot(hal_ota_module_t *m, void *something)
{
    if (m == NULL) {
        m = hal_ota_get_default_module();
    }

    if (m != NULL && m->ota_set_boot != NULL) {
        return m->ota_set_boot(m, something);
    }

    return 0;
}


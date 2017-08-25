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

#include <stddef.h>
#include <hal/hal.h>

static YOS_DLIST_HEAD(g_flash_module);

int hal_flash_conf_write(hal_flash_module_t *m, char *key,
                         unsigned char *buf, int buf_size)
{
    if (!key) {
        return -1;
    }

    if (m == NULL) {
        m = hal_flash_get_default_module();
    }

    if (m != NULL && m->write_conf != NULL) {
        return m->write_conf(m, key, buf, buf_size);
    }

    return -1;
}

int hal_flash_conf_read(hal_flash_module_t *m, char *key,
                        unsigned char *buf, int buf_size)
{
    if (!key) {
        return -1;
    }

    if (m == NULL) {
        m = hal_flash_get_default_module();
    }

    if (m != NULL && m->read_conf != NULL) {
        return m->read_conf(m, key, buf, buf_size);
    }

    return -1;
}

hal_flash_module_t *hal_flash_get_default_module(void)
{
    hal_flash_module_t *m = NULL;

    if (dlist_empty(&g_flash_module)) {
        return NULL;
    }

    m = dlist_first_entry(&g_flash_module, hal_flash_module_t, base.list);

    return m;
}

int hal_flash_conf_get_max_size(hal_flash_module_t *m)
{
    if (m == NULL) {
        m = hal_flash_get_default_module();
    }

    if (m != NULL && m->get_conf_size != NULL) {
        return m->get_conf_size(m);
    }

    return -1;
}

void hal_flash_register_module(hal_flash_module_t *module)
{
    dlist_add_tail(&module->base.list, &g_flash_module);
}

int hal_flash_init(void)
{
    int          err = 0;
    dlist_t *t;

    /* do low level init */
    dlist_for_each(t, &g_flash_module) {
        hal_flash_module_t *m = (hal_flash_module_t *)t;
        err = m->init(m, NULL);

        if (err != 0) {
            break;
        }
    }

    return err;
}


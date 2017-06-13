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

#include <string.h>
#include <yos/framework.h>

#include "utilities/configs.h"

enum {
    CONFIGS_MAGIC_NUMBER = 0xbe5cc5ec,
    CONFIGS_VERSION      = 1,
};

ur_error_t ur_configs_read(ur_configs_t *config)
{
#ifdef CONFIG_YOS_DDA
    int ret = -1;
    int len = sizeof(*config);

    if (config == NULL) {
        return UR_ERROR_FAIL;
    }

    ret = yos_kv_get("umesh", config, &len);
    if (ret < 0) {
        return UR_ERROR_FAIL;
    }
    if (config->magic_number == CONFIGS_MAGIC_NUMBER &&
        config->version == CONFIGS_VERSION) {
        return UR_ERROR_NONE;
    }
    memset(config, 0xff, sizeof(ur_configs_t));
#endif
    return UR_ERROR_FAIL;
}

ur_error_t ur_configs_write(ur_configs_t *config)
{
#ifdef CONFIG_YOS_DDA
    int ret = -1;

    if (config == NULL) {
        return UR_ERROR_FAIL;
    }

    config->magic_number = CONFIGS_MAGIC_NUMBER;
    config->version = CONFIGS_VERSION;

    ret = yos_kv_set("umesh", config, sizeof(*config), 1);
    if (ret < 0) {
        return UR_ERROR_FAIL;
    }

#endif
    return UR_ERROR_NONE;
}

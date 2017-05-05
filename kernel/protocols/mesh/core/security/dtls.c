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

#include "keys_mgr.h"
#include "dtls.h"

ur_error_t dtls_start(hal_context_t *hal, neighbor_t *nbr,
                      dtls_connected_handler_t connected_handler)
{
    // get master key from DTLS authentication or burned in the factory
    uint8_t master_key[KEY_SIZE] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    };

    set_master_key(master_key, sizeof(master_key));
    connected_handler(hal, nbr, true);
    return UR_ERROR_NONE;
}

ur_error_t dtls_stop(void)
{
    return UR_ERROR_NONE;
}

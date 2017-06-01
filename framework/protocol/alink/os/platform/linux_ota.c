/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
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
#include <assert.h>

#include "platform.h"

static FILE *fp;

#define otafilename "/tmp/alinkota.bin"
void platform_flash_program_start(void)
{
    fp = fopen(otafilename, "w");
    assert(fp);
    return;
}

int platform_flash_program_write_block(_IN_ char *buffer, _IN_ uint32_t length)
{
    unsigned int written_len = 0;
    written_len = fwrite(buffer, 1, length, fp);

    if (written_len != length)
        return -1;
    return 0;
}

int platform_flash_program_stop(void)
{
    if (fp != NULL) {
        fclose(fp);
    }

    /* check file md5, and burning it to flash ... finally reboot system */

    return 0;
}

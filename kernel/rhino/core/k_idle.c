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

#include <k_api.h>

#if (YUNOS_CONFIG_CPU_USAGE_STATS > 0)
void idle_count_set(idle_count_t value)
{
    CPSR_ALLOC();

    YUNOS_CPU_INTRPT_DISABLE();

    g_idle_count = value;

    YUNOS_CPU_INTRPT_ENABLE();
}

idle_count_t idle_count_get(void)
{
    return g_idle_count;
}
#endif

void idle_task(void *arg)
{
    CPSR_ALLOC();

    /* avoid warning */
    (void)arg;

    while (YUNOS_TRUE) {
        YUNOS_CPU_INTRPT_DISABLE();

        g_idle_count++;

        YUNOS_CPU_INTRPT_ENABLE();

#if (YUNOS_CONFIG_USER_HOOK > 0)
        yunos_idle_hook();
#endif
    }
}


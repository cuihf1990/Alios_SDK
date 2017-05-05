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

#ifndef K_SCHED_H
#define K_SCHED_H

#define KSCHED_FIFO           0u
#define KSCHED_RR             1u
#define SCHED_MAX_LOCK_COUNT  200u
#define NUM_WORDS             ((YUNOS_CONFIG_PRI_MAX + 31) / 32)

typedef struct {
    klist_t *cur_list_item[YUNOS_CONFIG_PRI_MAX];
    uint32_t task_bit_map[NUM_WORDS];
    uint8_t  highest_pri;
} runqueue_t;

/**
 * This function will disable schedule
 * @return the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_sched_disable(void);

/**
 * This function will enable schedule
 * @return the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_sched_enable(void);

#endif /* K_SCHED_H */


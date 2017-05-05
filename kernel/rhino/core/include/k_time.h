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

#ifndef K_TIME_H
#define K_TIME_H

#if (YUNOS_CONFIG_DYNTICKLESS > 0)
/**
 * This function will handle tickless routine
 * @param[in]  ticks
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
void yunos_tickless_proc(tick_t ticks);
#else
/**
 * This function will handle systick routine
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
void yunos_tick_proc(void);
#endif

/**
 * This function will get time of the system in ms
 * @return  system time
 */
sys_time_t yunos_sys_time_get(void);

/**
 * This function will get ticks of the system
 * @return  the system ticks
 */
sys_time_t yunos_sys_tick_get(void);

/**
 * This function will convert ms to ticks
 * @param[in]  ms  ms which will be converted to ticks
 * @return  the ticks of the ms
 */
tick_t     yunos_ms_to_ticks(sys_time_t ms);

/**
 * This function will convert ticks to ms
 * @param[in]  ticks  ticks which will be converted to ms
 * @return  the ms of the ticks
 */
sys_time_t yunos_ticks_to_ms(tick_t ticks);

#endif /* K_TIME_H */


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

#ifndef YOS_RTC_H
#define YOS_RTC_H

/**
 * RTC time
 */
typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hr;
    uint8_t weekday;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} hal_rtc_time_t;


/**@brief This function will initialize the on board CPU real time clock
 *
 * @note  This function should be called by MICO system when initializing clocks, so
 *        It is not needed to be called by application
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
void hal_rtc_init(void);

/**@brief This function will return the value of time read from the on board CPU real time clock. Time value must be given in the format of
 * the structure hal_rtc_time_t
 *
 * @param time        : pointer to a time structure
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int32_t hal_rtc_get_time(hal_rtc_time_t *time);

/**@brief This function will set MCU RTC time to a new value. Time value must be given in the format of
 * the structure hal_rtc_time_t
 *
 * @param time        : pointer to a time structure
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int32_t hal_rtc_set_time(hal_rtc_time_t *time);

/** @} */
/** @} */

#endif



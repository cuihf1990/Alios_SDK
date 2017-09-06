/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

/**
 * @file hal/soc/rtc.h
 * @brief RTC HAL
 * @version since 5.5.0
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
} rtc_time_t;

typedef struct {
    uint8_t      port;   /* rtc port */
    void        *priv;   /* priv data */
} rtc_dev_t;


/**@brief This function will initialize the on board CPU real time clock
 *
 * @note  This function should be called by MICO system when initializing clocks, so
 *        It is not needed to be called by application
 * @param     rtc   : rtc device
 * @return    0     : on success.
 * @return    EIO   : if an error occurred with any step
 */
void hal_rtc_init(rtc_dev_t *rtc);

/**@brief This function will return the value of time read from the on board CPU real time clock. Time value must be given in the format of
 * the structure hal_rtc_time_t
 * @param     rtc  : rtc device
 * @param     time : pointer to a time structure
 *
 * @return    0    : on success.
 * @return    EIO  : if an error occurred with any step
 */
int32_t hal_rtc_get_time(rtc_dev_t *rtc, rtc_time_t *time);

/**@brief This function will set MCU RTC time to a new value. Time value must be given in the format of
 * the structure hal_rtc_time_t
 * @param     rtc    : rtc device
 * @param     time   : pointer to a time structure
 *
 * @return    0      : on success.
 * @return    EIO    : if an error occurred with any step
 */
int32_t hal_rtc_set_time(rtc_dev_t *rtc, rtc_time_t *time);

#endif



/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

/**
 * @file hal/soc/timer.h
 * @brief PWM HAL
 * @version since 5.5.0
 */

#ifndef HAL_TIMER_H
#define HAL_TIMER_H

typedef void (*hal_timer_cb_t)(void *arg);

typedef struct {
    int8_t         ch;
    void          *priv;
    hal_timer_cb_t cb;
    void          *arg;
} hal_timer_t;

/**
 * @brief init a hardware timer
 * @param tmr timer struct
 * @param period micro seconds for repeat timer trigger
 * @param auto_reoad set to 0, if you just need oneshot timer
 * @param cb callback to be triggered after useconds
 * @ch    timer channel
 * @param arg passed to cb
 * @note  period   auto   auto   auto
 *         *-------|--------|--------|--------|
 */
void hal_timer_init(hal_timer_t *tmr, unsigned int period,
                    unsigned char auto_reload, unsigned char ch, hal_timer_cb_t cb, void *arg);

/**
 * @brief init a hardware timer
 * @param None
 * @retval 0 == success
 *        <0 == failure
 */
int32_t hal_timer_start(hal_timer_t *tmr);

/**
 * @brief stop a hardware timer
 * @param tmr timer struct
 * @param cb callback to be triggered after useconds
 * @param arg passed to cb
 */
void hal_timer_stop(hal_timer_t *tmr);

#endif

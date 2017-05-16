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

#ifndef HAL_TIMER_H
#define HAL_TIMER_H

typedef void (*hal_timer_cb_t)(void *arg);

typedef struct {
    int ch;
    void *priv;
    hal_timer_cb_t cb;
    void *arg;
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
void hal_timer_init(hal_timer_t *tmr, unsigned int period, unsigned char auto_reload, unsigned char ch, hal_timer_cb_t cb, void *arg);

/**
 * @brief init a hardware timer
 * @param None
 * @retval 0 == success
 *        <0 == failure
 */
int hal_timer_start(hal_timer_t *tmr);

/**
 * @brief stop a hardware timer
 * @param tmr timer struct
 * @param cb callback to be triggered after useconds
 * @param arg passed to cb
 */
void hal_timer_stop(hal_timer_t *tmr);

#endif

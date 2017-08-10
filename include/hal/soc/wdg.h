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

/**
 * @file hal/soc/wdg.h
 * @brief WDG HAL
 * @version since 5.5.0
 */

#ifndef YOS_WDG_H
#define YOS_WDG_H

typedef struct {
    uint32_t timeout;  /* Watchdag timeout */
} wdg_config_t;

typedef struct {
    uint8_t      port;    /* spi port */
    wdg_config_t config;  /* spi config */
    void        *priv;    /* priv data */
} wdg_dev_t;

/**
 * @biref This function will initialize the on board CPU hardware watch dog
 * @param     wdg         : the watch dog device
 * @return    kNoErr      : on success.
 * @return    kGeneralErr : if an error occurred with any step
 */
int32_t hal_wdg_init(wdg_dev_t *wdg);

/**
 * @biref Reload watchdog counter.
 * @param     wdg         : the watch dog device
 * @param     none
 * @return    none
 */
void hal_wdg_reload(wdg_dev_t *wdg);

/**
 * @biref This function performs any platform-specific cleanup needed for hardware watch dog.
 * @param     wdg           : the watch dog device
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int32_t hal_wdg_finalize(wdg_dev_t *wdg);

#endif



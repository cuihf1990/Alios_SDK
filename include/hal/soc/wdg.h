/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
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
 * @param     wdg    : the watch dog device
 * @return    0      : on success.
 * @return    EIO    : if an error occurred with any step
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
 * @param     wdg   : the watch dog device
 * @return    0     : on success.
 * @return    EIO   : if an error occurred with any step
 */
int32_t hal_wdg_finalize(wdg_dev_t *wdg);

#endif



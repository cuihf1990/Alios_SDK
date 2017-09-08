/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

/**
 * @file hal/base.h
 * @brief basic data structures for HAL
 * @version since 1.0.0
 */

#ifndef HAL_BASE_H
#define HAL_BASE_H

#include <aos/aos.h>

/**
 * @brief HAL common error code
 */
enum {
    HAL_ERR_ARG = -4096,
    HAL_ERR_CAP,
};

/**
 * @brief HAL Module define
 */
typedef struct {
    dlist_t      list;
    int          magic;
    const char  *name;
    void        *priv_dev; /* Driver may want to describe it */
} hal_module_base_t;

#endif


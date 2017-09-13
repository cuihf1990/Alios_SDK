/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef TRACE_HAL_H
#define TRACE_HAl_H

/**
 * @brief trace data transfer init.
 */
void *trace_hal_init(void);

/**
 * @brief trace data transfer send.
 *
 * @param[in]  @handle  data transfer channel object
 * @param[in]  @buf  the buffer store data
 * @param[in]  @len  the len of data
 *
 * @retval the size send success.
 */
ssize_t trace_hal_send(void *handle, void *buf, size_t len);

/**
 * @brief trace data transfer receive.
 *
 * @param[in]  @handle  data transfer channel object
 * @param[in]  @buf  the buffer to store data
 *
 * @retval the size receive success.
 */
ssize_t trace_hal_recv(void *handle, void *buf);

/**
 * @brief trace data transfer init.
 * @param[in]  @handle  data transfer channel object
 */
void trace_hal_deinit(void *handle);

#endif


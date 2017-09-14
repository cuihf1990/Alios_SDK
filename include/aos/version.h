/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef AOS_VERSION_H
#define AOS_VERSION_H

/**
 * get aos product model
 *
 * @return  model success, 0 failure
 */
const char *aos_get_product_model(void);

/**
 * get aos os version
 *
 * @return  os version success, 0 failure
 */
const char *aos_get_os_version(void);

/**
 * get aos kernel version
 *
 * @return  kernel version success, 0 failure
 */
const char *aos_get_kernel_version(void);

/**
 * get aos app version
 *
 * @return  app version success, 0 failure
 */
const char *aos_get_app_version(void);

/**
 * get aos device name
 *
 * @return  device name success, 0 failure
 */
const char *aos_get_device_name(void);

/**
 * dump sys info
 */
void dump_sys_info(void);

#endif /* AOS_VERSION_H */


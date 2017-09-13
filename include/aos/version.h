/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

/**
 * @file aos/version.h
 * @brief version API
 * @version since 1.0.0
 */

#ifndef AOS_SYS_VERSION
#define AOS_SYS_VERSION

/**
 * @brief get aos product model 
 * @retval model success
 * @retval 0 failure
 */
const char *aos_get_product_model(void);

/**
 * @brief get aos os version 
 * @retval os version success
 * @retval 0 failure
 */
const char *aos_get_os_version (void);

/**
 * @brief get aos kernel version 
 * @retval kernel version success
 * @retval 0 failure
 */
const char *aos_get_kernel_version (void);

/**
 * @brief get aos app version 
 * @retval app version success
 * @retval 0 failure
 */
const char *aos_get_app_version (void);

/**
 * @brief get aos device name 
 * @retval device name success
 * @retval 0 failure
 */
const char *aos_get_device_name(void);

/**
 * @brief dump sys info
 */
void dump_sys_info(void);

#endif /* AOS_SYS_VERSION */


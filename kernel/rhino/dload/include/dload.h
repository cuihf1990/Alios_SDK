/****************************************************************************
 *
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 ****************************************************************************/

 /**
 * @file
 * @brief      dynamic loader interfaces
 * @details
 * @author     xzf
 * @date       2016-11-16
 * @version    0.1
 */

#ifndef DLOAD_H_
#define DLOAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** dload error code definition */
typedef enum {
    E_DLD_SUCCESS   = 0u,
    E_DLD_OVERRANGE,
    E_DLD_NO_MEM,
    E_DLD_INV_PARAM,
    E_DLD_INV_VAL,
    E_DLD_INV_FMT,
    E_DLD_CRC_CHK,
    E_DLD_CMMU,
    E_DLD_NOT_EXIST,
    E_DLD_ALREADY_LOADED
} dload_err_t;

/**
 *  dynamic loader initialization
 *
 * @return       none
 * @note         must only init once at system start
 */
void ymod_init(void);

/**
 *  load specified image file
 *
 * @param[in]    image_fd   image file descriptor to be loaded
 * @return       0-success , other-error code. see definition of dload_err_t
 */
int32_t ymod_load(int32_t image_fd);

/**
 *  unload specified image file
 *
 * @param[in]    image_fd   image file descriptor to be unloaded
 * @return       0-success , other-error code. see definition of dload_err_t
 * @note         this function do not delete the image file,only stop running
 */
int32_t ymod_unload(int32_t image_fd);

/**
 *  get data size(.data&.bss section) from image file
 *
 * @param[in]    image_fd   image file descriptor
 * @return       data size in bytes. negative indicates error code, see definition of dload_err_t
 */
int32_t ymod_get_data_size(int32_t image_fd);

/**
 *  show running module information
 *
 * @param    none
 * @return   none
 */
void ymod_show_status(void);

#ifdef __cplusplus
}
#endif

#endif /*DLOAD_H_*/


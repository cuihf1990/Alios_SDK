/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef AOS_VFS_ADC_H
#define AOS_VFS_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vfs_inode.h"

/**
 * This function is used to open adc device.
 *
 * @param[in]  fp  device pointer.
 *
 * @return  0 on success, others on failure with errno set appropriately.
 */
int vfs_adc_open(inode_t *inode, file_t *fp);

/**
 * This function is used to close adc device.
 *
 * @param[in]  fp  device pointer.
 *
 * @return  0 on success, others on failure with errno set appropriately.
 */
 int vfs_adc_close(file_t *fp);

/**
 * This function is used to complete the data sample and get the sampled value.
 *
 * @param[in]   fp      device pointer.
 * @param[out]  buf     data buffer for sampled data.
 * @param[in]   nbytes  the maximum size of the user-provided buffer.
 *
 * @return  The positive non-zero number of bytes read on success, 
 * 0 on read nothing, or negative on failure with errno set appropriately.
 */
ssize_t vfs_adc_read(file_t *fp, void *buf, size_t nbytes);

/**
 * This function is used to output analog quantities.
 *
 * @param[in]   fp      device pointer.
 * @param[out]  buf     data buffer for output data.
 * @param[in]   nbytes  the maximum size of the user-provided buffer.
 *
 * @return   The positive non-zero number of bytes write on success, 
 * 0 on write nothing, or negative on failure with errno set appropriately.
 */
ssize_t vfs_adc_write(file_t *fp, const void *buf, size_t nbytes);

/**
 * This function performs device input and output operations.
 *
 * @param[in]  fp   device pointer.
 * @param[in]  cmd  command of input and output operating.
 * @param[in]  arg  argument of input and output operating. 
 *
 * @return  0 on success, negative on failure with errno set appropriately.
 */
int vfs_adc_ioctl(file_t *fp, int cmd, unsigned long arg);

#ifdef __cplusplus
}
#endif

#endif /* AOS_VFS_ADC_H */


/****************************************************************************
 *
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 ****************************************************************************/

/**
* @file
* @brief     system api table
* @details
* @author    xzf
* @date      2016-11-16
* @version   0.1
* @note
*/

#include <stdint.h>
//#include <k_api.h>
#include <dload_default_config.h>
#include "time.h"

#if (YUNOS_CONFIG_DLOAD_SUPPORT > 0)

extern int printf(const char *format, ...);

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

struct api_table_s {
    int32_t (*yunos_printk)(const char *fmt, ...);
    int     (*yunos_clock_gettime)(uint8_t clockid, struct timespec *tp);
#if 0
    /*================== misc :  ==================*/
    int32_t (*yunos_uname)(struct utsname *name);
    void    (*yunos_set_errno)(int32_t errcode);
    int32_t (*yunos_get_errno)(void);
    int32_t (*yunos_printk)(int32_t log_type, const char *fmt, ...);
    void   *(*yunos_kmalloc)(size_t size);
    void    (*yunos_kfree)(void *mem);

    /*================== vfs :  ==================*/
    int32_t (*yunos_open)(const char *path, int32_t oflag, ...);
    int32_t (*yunos_close)(int32_t fd);
    ssize_t (*yunos_read)(int32_t fd, void *buf, size_t nbytes);
    ssize_t (*yunos_write)(int32_t fd, const void *buf, size_t nbytes);
    int32_t (*yunos_ioctl)(int32_t fd, int32_t req, uint32_t arg);

    /*================== drivers :  ==================*/
    int32_t (*yunos_register_driver)(const char *path, file_ops_t *fops,
                                     void *priv);
    int32_t (*yunos_unregister_driver)(const char *path);

    /*================== semaphore Interfaces : semaphore.h==================*/

    int32_t (*yunos_sem_init)( ksem_t *sem, int32_t pshared, uint32_t value);
    int32_t (*yunos_sem_destroy)(ksem_t *sem);
    int32_t (*yunos_sem_timedwait)(ksem_t *sem, const struct timespec *timeout);
    int32_t (*yunos_sem_post)(ksem_t *sem);

    int32_t (*yunos_usleep)(useconds_t usec);
#endif
};


struct api_table_s g_api_table = {
    .yunos_printk = printf,
    .yunos_clock_gettime = clock_gettime
#if 0
    .yunos_uname = &uname,
    .yunos_set_errno = &yunos_set_errno,
    .yunos_get_errno = &yunos_get_errno,
    .yunos_printk = &syslog,
    .yunos_kmalloc = &malloc,
    .yunos_kfree = &free,
    .yunos_open = &open,
    .yunos_close = &close,
    .yunos_read = &read,
    .yunos_write = &write,
    .yunos_ioctl = &ioctl,
    .yunos_register_driver = &register_driver,
    .yunos_unregister_driver = &unregister_driver,
    .yunos_sem_init = &sem_init,
    .yunos_sem_destroy = &sem_destroy,
    .yunos_ksem_timedwait = &ksem_timedwait,
    .yunos_sem_post = &sem_post,
    .yunos_usleep = &usleep
#endif
};

#endif /*YUNOS_CONFIG_DLOAD_SUPPORT*/

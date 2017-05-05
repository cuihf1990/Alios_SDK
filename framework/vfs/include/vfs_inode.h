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

#ifndef VFS_INODE_H
#define VFS_INODE_H

#include <stdbool.h>
#include <stdint.h>
#include "vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    VFS_TYPE_NOT_INIT,
    VFS_TYPE_CHAR_DEV
};

typedef struct file_ops file_ops_t;

/* this structure represents inode for driver and fs*/
typedef struct {
    file_ops_t *ops;      /* inode operations */
    void       *i_arg;    /* per inode private data */
    char       *i_name;   /* name of inode */
    int         i_flags;  /* flags for inode*/
    uint8_t     type;     /* type for inode */
    uint8_t     refs;     /* refs for inode */
} inode_t;

#ifdef YUNOS_CONFIG_VFS_POLL_SUPPORT
#define YUNOS_VFS_FILE_OPS_FIELDS  \
    int (*open)(inode_t * node);\
    int (*close)(inode_t * node);\
    ssize_t (*read)(inode_t * node, char *buf, size_t len);\
    ssize_t (*write)(inode_t * node, const char *buf, size_t len);\
    int (*ioctl)(inode_t * node, int cmd, unsigned long arg);\
    int (*poll)(inode_t * node, bool setup, struct pollfd *pfd, void *sem);\

#else
#define YUNOS_VFS_FILE_OPS_FIELDS  \
    int (*open)(inode_t *node);\
    int (*close)(inode_t *node);\
    ssize_t (*read)(inode_t *node, char *buf, size_t len);\
    ssize_t (*write)(inode_t *node, const char *buf, size_t len);\
    int (*ioctl)(inode_t *node, int cmd, unsigned long arg);\

#endif

struct file_ops {
    YUNOS_VFS_FILE_OPS_FIELDS
};

int inode_init(void);
int inode_alloc(void);
int inode_del(int fd);
int inode_open(const char *path);
int inode_ptr_get(int fd, inode_t **node);
int inode_avail_count(void);
int inode_ref(int fd);
int inode_unref(int fd);
int inode_busy(int fd);

#ifdef __cplusplus
}
#endif

#endif /*VFS_INODE_H*/


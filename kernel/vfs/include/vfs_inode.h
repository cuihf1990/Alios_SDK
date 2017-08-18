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

typedef const struct file_ops file_ops_t;

/* this structure represents inode for driver and fs*/
typedef struct {
    file_ops_t *ops;      /* inode operations */
    void       *i_arg;    /* per inode private data */
    char       *i_name;   /* name of inode */
    int         i_flags;  /* flags for inode*/
    uint8_t     type;     /* type for inode */
    uint8_t     refs;     /* refs for inode */
} inode_t;

typedef struct {
    inode_t    *node;
    void       *f_arg;
    size_t      offset;
} file_t;

struct pollfd;
typedef void (*poll_notify_t)(struct pollfd *, void *);
struct file_ops {
    int (*open)(inode_t *, file_t *);
    int (*close)(file_t *);
    ssize_t (*read)(file_t *, void *, size_t);
    ssize_t (*write)(file_t *, const void *buf, size_t len);
    int (*ioctl)(file_t *, int cmd, unsigned long arg);
#ifdef YOS_CONFIG_VFS_POLL_SUPPORT
    int (*poll)(file_t *, bool , poll_notify_t, struct pollfd *, void *);
#endif
};

int     inode_init(void);
int     inode_alloc(void);
int     inode_del(inode_t *node);
inode_t *inode_open(const char *path);
int     inode_ptr_get(int fd, inode_t **node);
int     inode_avail_count(void);
void    inode_ref(inode_t *);
void    inode_unref(inode_t *);
int     inode_busy(inode_t *);

#ifdef __cplusplus
}
#endif

#endif /*VFS_INODE_H*/


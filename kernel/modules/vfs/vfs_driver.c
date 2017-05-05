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

#include <malloc.h>
#include <string.h>
#include <csp.h>
#include <vfs_conf.h>
#include <vfs_err.h>
#include <vfs_driver.h>

extern csp_mutex_t   g_vfs_mutex;

int yunos_register_driver(const char *path, file_ops_t *ops, void *arg)
{
    inode_t *node;
    int      ret;
    size_t   len;
    void    *mem;

    if (path == NULL || ops == NULL) {
        return E_VFS_NULL_PTR;
    }

    if (csp_mutex_lock(g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    ret = inode_open(path);

    if (ret != E_VFS_INODE_NOT_FOUND) {
        csp_mutex_unlock(g_vfs_mutex);
        return E_VFS_REGISTERED;
    }

    ret = inode_alloc();

    if (ret < 0) {
        csp_mutex_unlock(g_vfs_mutex);
        return ret;
    }

    inode_ptr_get(ret, &node);

    node->ops    = ops;
    node->i_arg  = arg;
    len = strlen(path);
    mem = malloc(len + 1);

    if (mem == NULL) {
        csp_mutex_unlock(g_vfs_mutex);
        return E_VFS_NO_MEM;
    }

    memcpy(mem, (const void *) path, len);
    node->i_name = (char *)mem;
    node->i_name[len] = '\0';
    node->type   = VFS_TYPE_CHAR_DEV;

    /* step out critical area for type is allocated */
    if (csp_mutex_unlock(g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    return VFS_SUCCESS;
}

int yunos_unregister_driver(const char *path)
{
    int fd;
    int ret;

    if (path == NULL) {
        return E_VFS_NULL_PTR;
    }

    if (csp_mutex_lock(g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    fd = inode_open(path);

    if (fd == E_VFS_INODE_NOT_FOUND) {
        csp_mutex_unlock(g_vfs_mutex);
        return E_VFS_INODE_NOT_FOUND;
    }

    ret = inode_del(fd);

    if (csp_mutex_unlock(g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    return ret;
}


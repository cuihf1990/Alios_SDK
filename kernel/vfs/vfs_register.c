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

#include <string.h>

#include <yos/kernel.h>
#include <vfs_conf.h>
#include <vfs_err.h>
#include <vfs_register.h>

extern yos_mutex_t g_vfs_mutex;

int yos_register_driver(const char *path, file_ops_t *ops, void *arg)
{
    inode_t *node = NULL;
    int      ret;

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return E_VFS_K_ERR;
    }

    ret = inode_reserve(path, &node);
    if (ret == VFS_SUCCESS) {
        /* now populate it with char specific information */
        INODE_SET_CHAR(node);

        node->ops.i_ops = ops;
        node->i_arg     = arg;
    }

    /* step out critical area for type is allocated */
    if (yos_mutex_unlock(&g_vfs_mutex) != 0) {
        if (node->i_name != NULL) {
            yos_free(node->i_name);
        }

        memset(node, 0, sizeof(inode_t));
        return E_VFS_K_ERR;
    }

    return ret;
}

int yos_unregister_driver(const char *path)
{
    int ret;

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return E_VFS_K_ERR;
    }

    ret = inode_release(path);

    if (yos_mutex_unlock(&g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    return ret;
}

int yos_register_fs(const char *path, fs_ops_t *ops, void *arg)
{
    inode_t *node = NULL;
    int      ret;

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return E_VFS_K_ERR;
    }

    ret = inode_reserve(path, &node);
    if (ret == VFS_SUCCESS) {
        INODE_SET_FS(node);

        node->ops.i_fops = ops;
        node->i_arg      = arg;
    }

    if (yos_mutex_unlock(&g_vfs_mutex) != 0) {
        if (node->i_name != NULL) {
            yos_free(node->i_name);
        }

        memset(node, 0, sizeof(inode_t));
        return E_VFS_K_ERR;
    }

    return ret;
}

int yos_unregister_fs(const char *path)
{
    int ret;

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return E_VFS_K_ERR;
    }

    ret = inode_release(path);

    if (yos_mutex_unlock(&g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    return ret;
}


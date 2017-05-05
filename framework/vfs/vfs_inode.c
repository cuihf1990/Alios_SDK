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
#include <vfs_inode.h>

static inode_t g_vfs_dev_nodes[YUNOS_CONFIG_VFS_DEV_NODES];

int inode_init()
{
    memset(g_vfs_dev_nodes, 0, sizeof(inode_t) * YUNOS_CONFIG_VFS_DEV_NODES);
    return 0;
}

int inode_alloc()
{
    int e = 0;

    for (; e < YUNOS_CONFIG_VFS_DEV_NODES; e++) {
        if (g_vfs_dev_nodes[e].type == VFS_TYPE_NOT_INIT) {
            return e + YUNOS_CONFIG_VFS_FD_OFFSET;
        }
    }

    return E_VFS_INODE_NO_AVAIL;
}

int inode_del(int fd)
{
    if (fd < YUNOS_CONFIG_VFS_FD_OFFSET || fd >= YUNOS_CONFIG_VFS_FD_OFFSET + YUNOS_CONFIG_VFS_DEV_NODES) {
        return E_VFS_FD_ILLEGAL;
    }

    fd -= YUNOS_CONFIG_VFS_FD_OFFSET;

    if (g_vfs_dev_nodes[fd].refs > 0) {
        return E_VFS_BUSY;
    }

    if (g_vfs_dev_nodes[fd].refs == 0) {
        if (g_vfs_dev_nodes[fd].i_name != NULL) {
            free(g_vfs_dev_nodes[fd].i_name);
        }

        g_vfs_dev_nodes[fd].i_name = NULL;
        g_vfs_dev_nodes[fd].i_arg = NULL;
        g_vfs_dev_nodes[fd].i_flags = 0;
        g_vfs_dev_nodes[fd].type = VFS_TYPE_NOT_INIT;
    }

    return VFS_SUCCESS;
}

int inode_open(const char *path)
{
    int e = 0;

    for (; e < YUNOS_CONFIG_VFS_DEV_NODES; e++) {
        if ((g_vfs_dev_nodes[e].i_name != NULL) && (strcmp(g_vfs_dev_nodes[e].i_name, path) == 0)) {
            return e + YUNOS_CONFIG_VFS_FD_OFFSET;
        }
    }

    return E_VFS_INODE_NOT_FOUND;
}

int inode_ptr_get(int fd, inode_t **node)
{
    if (fd < YUNOS_CONFIG_VFS_FD_OFFSET || fd >= YUNOS_CONFIG_VFS_FD_OFFSET + YUNOS_CONFIG_VFS_DEV_NODES) {
        return E_VFS_FD_ILLEGAL;
    }

    fd -= YUNOS_CONFIG_VFS_FD_OFFSET;

    *node = &g_vfs_dev_nodes[fd];

    return VFS_SUCCESS;
}

int inode_ref(int fd)
{
    if (fd < YUNOS_CONFIG_VFS_FD_OFFSET || fd >= YUNOS_CONFIG_VFS_FD_OFFSET + YUNOS_CONFIG_VFS_DEV_NODES) {
        return E_VFS_FD_ILLEGAL;
    }

    fd -= YUNOS_CONFIG_VFS_FD_OFFSET;

    g_vfs_dev_nodes[fd].refs++;
    return VFS_SUCCESS;
}

int inode_unref(int fd)
{
    if (fd < YUNOS_CONFIG_VFS_FD_OFFSET || fd >= YUNOS_CONFIG_VFS_FD_OFFSET + YUNOS_CONFIG_VFS_DEV_NODES) {
        return E_VFS_FD_ILLEGAL;
    }

    fd -= YUNOS_CONFIG_VFS_FD_OFFSET;

    if (g_vfs_dev_nodes[fd].refs > 0) {
        g_vfs_dev_nodes[fd].refs--;
    }

    return VFS_SUCCESS;
}

int inode_busy(int fd)
{
    if (fd < YUNOS_CONFIG_VFS_FD_OFFSET || fd >= YUNOS_CONFIG_VFS_FD_OFFSET + YUNOS_CONFIG_VFS_DEV_NODES) {
        return E_VFS_FD_ILLEGAL;
    }

    fd -= YUNOS_CONFIG_VFS_FD_OFFSET;

    return g_vfs_dev_nodes[fd].refs;
}

int inode_avail_count(void)
{
    int count = 0;
    int e = 0;

    for (; e < YUNOS_CONFIG_VFS_DEV_NODES; e++) {
        if (g_vfs_dev_nodes[count].type == VFS_TYPE_NOT_INIT) {
            count++;
        }
    }

    return count;
}


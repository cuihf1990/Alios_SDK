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

#include <vfs_conf.h>
#include <vfs_err.h>
#include <vfs_inode.h>

static inode_t g_vfs_dev_nodes[YOS_CONFIG_VFS_DEV_NODES];

int inode_init()
{
    memset(g_vfs_dev_nodes, 0, sizeof(inode_t) * YOS_CONFIG_VFS_DEV_NODES);
    return 0;
}

int inode_alloc()
{
    int e = 0;

    for (; e < YOS_CONFIG_VFS_DEV_NODES; e++) {
        if (g_vfs_dev_nodes[e].type == VFS_TYPE_NOT_INIT) {
            return e;
        }
    }

    return E_VFS_INODE_NO_AVAIL;
}

int inode_del(inode_t *node)
{
    if (node->refs > 0) {
        return E_VFS_BUSY;
    }

    if (node->refs == 0) {
        if (node->i_name != NULL) {
            yos_free(node->i_name);
        }

        node->i_name = NULL;
        node->i_arg = NULL;
        node->i_flags = 0;
        node->type = VFS_TYPE_NOT_INIT;
    }

    return VFS_SUCCESS;
}

inode_t *inode_open(const char *path)
{
    int e = 0;
    inode_t *node;

    for (; e < YOS_CONFIG_VFS_DEV_NODES; e++) {
        node = &g_vfs_dev_nodes[e];
        if (node == NULL) {
            continue;
        }
        if (node->i_name == NULL) {
            continue;
        }
        if (strcmp(node->i_name, path) == 0) {
            return node;
        }
    }

    return NULL;
}

int inode_ptr_get(int fd, inode_t **node)
{
    if (fd < 0 || fd >= YOS_CONFIG_VFS_DEV_NODES) {
        return E_VFS_FD_ILLEGAL;
    }

    *node = &g_vfs_dev_nodes[fd];

    return VFS_SUCCESS;
}

void inode_ref(inode_t *node)
{
    node->refs++;
}

void inode_unref(inode_t *node)
{
    if (node->refs > 0) {
        node->refs--;
    }
}

int inode_busy(inode_t *node)
{
    return node->refs > 0;
}

int inode_avail_count(void)
{
    int count = 0;
    int e = 0;

    for (; e < YOS_CONFIG_VFS_DEV_NODES; e++) {
        if (g_vfs_dev_nodes[count].type == VFS_TYPE_NOT_INIT) {
            count++;
        }
    }

    return count;
}

static int inode_set_name(const char *path, inode_t **inode)
{
    size_t len;
    void  *mem;

    len = strlen(path);
    mem = (void *)yos_malloc(len + 1);
    if (!mem)
        return E_VFS_NO_MEM;

    memcpy(mem, (const void *)path, len);
    (*inode)->i_name = (char *)mem;
    (*inode)->i_name[len] = '\0';

    return VFS_SUCCESS;
}

int inode_reserve(const char *path, inode_t **inode)
{
    int ret;
    inode_t *node;

    VFS_NULL_PARA_CHK(path != NULL && inode != NULL);
    *inode = NULL;

    /* Handle paths that are interpreted as the root directory */
    if (path[0] == '\0' || path[0] != '/')
        return E_VFS_REGISTERED;

    ret = inode_alloc();
    if (ret < 0)
        return ret;

    inode_ptr_get(ret, &node);

    ret = inode_set_name(path, &node);
    if (ret < 0)
        return ret;

    *inode = node;
    return VFS_SUCCESS;
}

int inode_release(const char *path)
{
    int ret;
    inode_t *node;

    VFS_NULL_PARA_CHK(path != NULL);

    node = inode_open(path);
    if (node == NULL)
        return E_VFS_INODE_NOT_FOUND;

    ret = inode_del(node);
    if (ret < 0)
        return ret;

    return VFS_SUCCESS;
}


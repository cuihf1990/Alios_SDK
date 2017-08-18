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

#ifndef VFS_DRIVER_H
#define VFS_DRIVER_H

#include <vfs_inode.h>

#ifdef __cplusplus
extern "C" {
#endif

int yos_register_driver(const char *path, file_ops_t *fops, void *arg);
int yos_unregister_driver(const char *path);

#ifdef __cplusplus
}
#endif

#endif    /*VFS_DRIVER_H*/


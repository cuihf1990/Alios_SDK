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

#ifndef VFS_DIRENT_H
#define VFS_DIRENT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int     d_ino;                      /* file number */
    uint8_t d_type;                     /* type of file */
    char    d_name[];                   /* file name */
} yos_dirent_t;

typedef struct {
    int         dd_vfs_fd;              /* This keeps track of the current directory position for telldir */
    int         dd_rsv;
} yos_dir_t;

#ifdef __cplusplus
}
#endif

#endif /* VFS_DIRENT_H */



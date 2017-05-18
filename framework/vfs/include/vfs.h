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

#ifndef YUNOS_VFS_H
#define YUNOS_VFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <vfs_conf.h>
#include <poll.h>

int vfs_init(void);

int vfs_device_init(void);

#ifdef __cplusplus
}
#endif

#endif

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

#ifndef YUNOS_VFS_CONFIG_H
#define YUNOS_VFS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif
#define VFS_FALSE    0u
#define VFS_TRUE     1u

#define    YUNOS_CONFIG_VFS_DEV_NODES    25//15
/*mem 1000 byte*/
#define    YUNOS_CONFIG_VFS_DEV_MEM      2000
#define    YUNOS_CONFIG_VFS_POLL_SUPPORT 1
#define    YUNOS_CONFIG_VFS_FD_OFFSET    64

#ifdef __cplusplus
}
#endif

#endif


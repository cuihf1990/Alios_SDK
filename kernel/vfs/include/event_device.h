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

#ifndef EVENT_DEVICE_H
#define EVENT_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#define IOCTL_WRITE_NORMAL 0x1
#define IOCTL_WRITE_URGENT 0x2

#define MK_CMD(c, l) ((l << 4) | (c))
#define _GET_LEN(cmd) ((cmd) >> 4)
#define _GET_CMD(cmd) ((cmd) & 0xf)

#ifdef __cplusplus
}
#endif

#endif

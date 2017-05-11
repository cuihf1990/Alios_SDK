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

#ifndef YOS_NETWORK_API_H
#define YOS_NETWORK_API_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <errno.h>
#include <fcntl.h>

/* network */
#ifndef WITH_LWIP
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/eventfd.h>
#else
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#endif

#ifdef __cplusplus
}
#endif

#endif /* YOS_NETWORK_API_H */

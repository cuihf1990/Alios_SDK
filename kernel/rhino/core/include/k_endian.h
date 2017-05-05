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
#ifndef K_ENDIAN_H
#define K_ENDIAN_H

#if (YUNOS_CONFIG_LITTLE_ENDIAN != 1)

#define yunos_htons(x) x
#define yunos_htonl(x) x
#define yunos_ntohl(x) x
#define yunos_ntohs(x) x

#else

#ifndef yunos_htons
#define yunos_htons(x) ((uint16_t)(((x) & 0x00ffU) << 8)| \
                        (((x) & 0xff00U) >> 8))
#endif

#ifndef yunos_htonl
#define yunos_htonl(x) ((uint32_t)(((x) & 0x000000ffUL) << 24) | \
                        (((x) & 0x0000ff00UL) << 8) | \
                        (((x) & 0x00ff0000UL) >> 8) | \
                        (((x) & 0xff000000UL) >> 24))
#endif

#ifndef yunos_ntohs
#define yunos_ntohs(x) ((uint16_t)((x & 0x00ffU) << 8) | \
                        ((x & 0xff00U) >> 8))
#endif

#ifndef yunos_ntohl
#define yunos_ntohl(x) ((uint32_t)(((x) & 0x000000ffUL) << 24) | \
                        (((x) & 0x0000ff00UL) << 8) | \
                        (((x) & 0x00ff0000UL) >> 8) | \
                        (((x) & 0xff000000UL) >> 24))
#endif


#endif /*YUNOS_CONFIG_LITTLE_ENDIAN*/



#endif

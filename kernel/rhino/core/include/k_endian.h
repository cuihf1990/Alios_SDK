/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
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

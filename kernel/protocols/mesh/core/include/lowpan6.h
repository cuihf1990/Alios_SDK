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

#ifndef UR_LOWPAN6_H
#define UR_LOWPAN6_H

#include "message.h"
#include "memory.h"
#include "interface_context.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FIXED_IID  "\x00\x00\x00\xff\xfe\x00"

enum {
    IP_VERSION_6       = 0x60000000,
    VERSION_MASK       = 0xf0000000,
    TRAFFIC_CLASS_MASK = 0x0ff00000,
    TC_ECN_MASK        = 0x0c000000,
    TC_DSCP_MASK       = 0x03f00000,
    FLOW_LABEL_MASK    = 0x000fffff,
};

enum {
    IPHC_DISPATCH    = 0b110,
    NHC_UDP_DISPATCH = 0b11110,
};

enum {
    TC_FL_BOTH_APEENDED         = 0b00,
    DCSP_ELEDED_ECN_FL_APPENDED = 0b01,
    TC_APENDED_FL_ELIDED        = 0b10,
    TC_FL_BOTH_ELIDED           = 0b11,
};

enum {
    NEXT_HEADER_APPENDED = 0b0,
    NEXT_HEADER_ELIDED   = 0b1,
};

enum {
    HOP_LIM_APPENDED = 0b00,
    HOP_LIM_1        = 0b01,
    HOP_LIM_64       = 0b10,
    HOP_LIM_255      = 0b11,
};

enum {
    STATELESS_COMPRESS = 0b0,
    STATEFULL_COMPRESS = 0b1,
};

enum {
    UNICAST_DESTINATION   = 0b0,
    MULTICAST_DESTINATION = 0b1,
};

enum {
    UCAST_ADDR_128BIT = 0b00,
    UCAST_ADDR_64BIT  = 0b01,
    UCAST_ADDR_16BIT  = 0b10,
    UCAST_ADDR_ELIDED = 0b11,
};

enum {
    MCAST_ADDR_128BIT = 0b00,
    MCAST_ADDR_48BIT  = 0b01,
    MCAST_ADDR_32BIT  = 0b10,
    MCAST_ADDR_8BIT   = 0b11,
};

enum {
    CHKSUM_APPENDED = 0b0,
    CHKSUM_ELIDED   = 0b1,
};

enum {
    NO_PORT_COMPRESSED   = 0b00,
    DST_PORT_COMPRESSED  = 0b01,
    SRC_PORT_COMPRESSED  = 0b10,
    BOTH_PORT_COMPRESSED = 0b11,
};

enum {
    FRAG_1_DISPATCH = 0b11000,
    FRAG_N_DISPATCH = 0b11100,
};

enum {
    REASSEMBLE_TICK_INTERVAL = 600,
};

typedef struct iphc_header_s {
    unsigned int DAM  : 2;
    unsigned int DAC  : 1;
    unsigned int M    : 1;
    unsigned int SAM  : 2;
    unsigned int SAC  : 1;
    unsigned int CID  : 1;

    unsigned int HLIM : 2;
    unsigned int NH   : 1;
    unsigned int TF   : 2;
    unsigned int DP   : 3;
} __attribute__((packed)) iphc_header_t;

typedef struct nhc_header_s {
    unsigned int P  : 2;
    unsigned int C  : 1;
    unsigned int DP : 5;
} __attribute__((packed)) nhc_header_t;

typedef struct frag_header_s {
    unsigned int size : 11;
    unsigned int dispatch : 5;
    uint16_t tag;
    uint8_t  offset;
} __attribute__((packed)) frag_header_t;

#define lowpan_mem_alloc  ur_mem_alloc
#define lowpan_mem_free   ur_mem_free

ur_error_t lp_header_compress(const uint8_t* header, uint8_t* buffer,
                              uint8_t *ip_hdr_len, uint8_t *lowpan_hdr_len);
message_t* lp_header_decompress(message_t *message, mesh_header_t *mesh_header);
ur_error_t lp_reassemble(message_t *p, mesh_header_t *mesh_header, message_t **reass_p);
void       lp_init(void);
#ifdef __cplusplus
}
#endif

#endif /* UR_LOWPAN6_H */

/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef UR_LOWPAN6_H
#define UR_LOWPAN6_H

#include "utilities/message.h"
#include "utilities/memory.h"
#include "hal/interface_context.h"

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
    IPHC_DISPATCH    = 0b011,
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
    MIN_LOWPAN_FRM_SIZE = 6,
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
    uint16_t size : 11;
    uint16_t dispatch : 5;
    uint16_t tag;
    uint8_t  offset;
} __attribute__((packed)) frag_header_t;

ur_error_t lp_header_compress(const uint8_t *header, uint8_t *buffer,
                              uint16_t *ip_hdr_len, uint16_t *lowpan_hdr_len);
ur_error_t lp_header_decompress(uint8_t *header, uint16_t *header_size,
                                uint16_t *lowpan_header_size,
                                ur_addr_t *src, ur_addr_t *dest);
ur_error_t lp_reassemble(message_t *p, message_t **reass_p);
void       lp_start(void);
void       lp_stop(void);
bool       lp_reass_queue_empty(void);
#ifdef __cplusplus
}
#endif

#endif /* UR_LOWPAN6_H */

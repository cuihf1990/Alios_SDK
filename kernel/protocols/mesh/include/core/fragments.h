/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef MESH_FRAGMENTS_H
#define MESH_FRAGMENTS_H

#include "utilities/message.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    FRAG_1_DISPATCH = 0b11000,
    FRAG_N_DISPATCH = 0b11100,
};

enum {
    REASSEMBLE_TICK_INTERVAL = 600,
};

typedef struct frag_header_s {
    uint16_t size : 11;
    uint16_t dispatch : 5;
    uint16_t tag;
    uint8_t  offset;
} __attribute__((packed)) frag_header_t;

ur_error_t lp_reassemble(message_t *p, message_t **reass_p);
void lp_start(void);
void lp_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* MESH_FRAGMENTS_H */

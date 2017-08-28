#ifndef UR_MCAST_H
#define UR_MCAST_H

#include "hal/interface_context.h"

typedef struct mcast_header_s {
    uint8_t  control;
    uint8_t subnetid;
    uint16_t sid;
    uint8_t  sequence;
} __attribute__((packed)) mcast_header_t;

ur_error_t insert_mcast_header(network_context_t *network,
                               uint8_t *message);
ur_error_t process_mcast_header(network_context_t *network,
                                uint8_t *message);

#endif  /* UR_MCAST_H */

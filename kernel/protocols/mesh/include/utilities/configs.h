#ifndef UR_CONFIGS_H
#define UR_CONFIGS_H

#include "umesh_types.h"

typedef struct prev_netinfo_s {
    uint16_t meshnetid;
    uint16_t path_cost;
} prev_netinfo_t;

typedef struct ur_configs_s {
    uint32_t magic_number;
    uint8_t  version;
    uint8_t  main_version;
    prev_netinfo_t prev_netinfo;
} ur_configs_t;

ur_error_t ur_configs_read(ur_configs_t *config);
ur_error_t ur_configs_write(ur_configs_t *config);

#endif  /* UR_CONFIGS_H */

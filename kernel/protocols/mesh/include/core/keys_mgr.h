#ifndef UR_KEYS_MANAGER_H
#define UR_KEYS_MANAGER_H

#include "umesh_types.h"

enum {
    SEC_LEVEL_0 = 0,
    SEC_LEVEL_1 = 1,
};

ur_error_t calculate_one_time_key(uint8_t *key, uint32_t timestamp,
                                  const uint8_t *mac);
ur_error_t calculate_network_key(void);

ur_error_t set_symmetric_key(uint8_t key_index, uint8_t *payload,
                             uint8_t length);
const uint8_t *get_symmetric_key(uint8_t key_index);

#endif /* UR_KEYS_MANAGER_H */

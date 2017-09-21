/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef UR_LWIP_ADAPTER_H
#define UR_LWIP_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

ur_error_t ur_adapter_interface_init(void);

ur_error_t ur_adapter_interface_up(void);
ur_error_t ur_adapter_interface_down(void);
ur_error_t ur_adapter_interface_update(void);

#ifdef __cplusplus
}
#endif

#endif  /* UR_LWIP_ADAPTER_H */

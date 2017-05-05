/****************************************************************************
 * Copyright (C) 2015 The YunOS Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/
#ifndef _BLE_HCICMDS_H_
#define _BLE_HCICMDS_H_

#include "mxd_type.h"

/* advertising type */
typedef enum _BLE_ADV_TYPE {
    BLE_ADV_IND                     = 0x00,
    BLE_ADV_DIRECT_IND_HI           = 0x01,
    BLE_ADV_SCAN_IND                = 0x02,
    BLE_ADV_NONCONN_IND             = 0x03,
    BLE_ADV_DIRECT_IND_LOW          = 0x04
} BLE_ADV_TYPE;

#define BLE_ADDR_PUBLIC         0x00
#define BLE_ADDR_RANDOM         0x01
//#define BLE_ADDR_TYPE_MASK      (BLE_ADDR_RANDOM | BLE_ADDR_PUBLIC)

/* advertising channel map */
#define BLE_ADV_CHNL_37    (0x01 << 0)
#define BLE_ADV_CHNL_38    (0x01 << 1)
#define BLE_ADV_CHNL_39    (0x01 << 2)
#define BLE_DEFAULT_ADV_CHNL_MAP   (BLE_ADV_CHNL_37| BLE_ADV_CHNL_38| BLE_ADV_CHNL_39)

/* advertising filter policy */
#define AP_SCAN_CONN_ALL           0x00 //Process scan and connection requests from all devices (i.e., the White Listis not in use)       /* default */
#define AP_SCAN_WL_CONN_ALL        0x01 //Process connection requests from all devices and only scan requests from devices that are in the White List.
#define AP_SCAN_ALL_CONN_WL        0x02 //Process scan requests from all devices and only connection requests from devices that are in the White List..
#define AP_SCAN_CONN_WL            0x03 //Process scan and connection requests only from devices in the White List.
#define AP_SCAN_CONN_POLICY_MAX    0x04


typedef enum _BLE_SCAN_TYPE
{
    BLE_SCAN_PASSIVE = 0x00,
    BLE_SCAN_ACTIVE  = 0x01
} BLE_SCAN_TYPE;

typedef enum _BLE_SFP
{
    BLE_SFP_ACCEPT_ALL = 0x00,
    BLE_SFP_USE_WL = 0x01
}  BLE_SFP;

BOOLEAN btsnd_hcic_ble_reset(void);
BOOLEAN btsnd_hcic_read_bd_addr (void);

BOOLEAN btsnd_hcic_ble_write_adv_params (uint16_t adv_int_min, uint16_t adv_int_max,
        uint8_t adv_type, uint8_t addr_type_own,
        uint8_t addr_type_dir, BD_ADDR direct_bda,
        uint8_t channel_map, uint8_t adv_filter_policy);

BOOLEAN btsnd_hcic_ble_set_scan_params (uint8_t scan_type,
                                        uint16_t scan_int, uint16_t scan_win,
                                        uint8_t addr_type_own, uint8_t scan_filter_policy);

BOOLEAN btsnd_hcic_ble_set_adv_data (uint8_t data_len, uint8_t *p_data);
BOOLEAN btsnd_hcic_ble_set_adv_enable (uint8_t adv_enable);
BOOLEAN btsnd_hcic_ble_set_scan_enable (uint8_t scan_enable, uint8_t filter_duplicate);
BOOLEAN btsnd_hcic_ble_set_random_addr (BD_ADDR random_bda);

BOOLEAN btsnd_hcic_ble_create_ll_conn (uint16_t scan_int, uint16_t scan_win,
                                       uint8_t init_filter_policy,
                                       uint8_t addr_type_peer, BD_ADDR bda_peer,
                                       uint8_t addr_type_own,
                                       uint16_t conn_int_min, uint16_t conn_int_max,
                                       uint16_t conn_latency, uint16_t conn_timeout,
                                       uint16_t min_ce_len, uint16_t max_ce_len);

#endif

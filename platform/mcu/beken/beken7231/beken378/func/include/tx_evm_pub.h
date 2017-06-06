#ifndef _TX_EVM_PUB_H_
#define _TX_EVM_PUB_H_

#include "hal_desc.h"

#define TX_LEGACY_MODE              (1)
#define TX_HT_VHT_MODE              (2)

#define TX_LEGACY_DATA_LEN_MASK              (0xFFF)
#define TX_HT_VHT_DATA_LEN_MASK              (0xFFFFF)

/*******************************************************************************
* Function Declarations
*******************************************************************************/
extern void evm_via_mac_evt(INT32 dummy);
extern void evm_via_mac_begin(void);
extern void evm_via_mac_continue(void);
extern UINT32 evm_bypass_mac_set_tx_data_length(UINT32 is_legacy_mode, UINT32 len);
extern UINT32 evm_bypass_mac_set_rate(UINT32 ppdu_rate);
extern void evm_bypass_mac_set_channel(UINT32 channel);
extern void evm_via_mac_set_channel(UINT32 channel);
extern void evm_bypass_mac_test(void);
extern void evm_via_mac_set_rate(HW_RATE_E rate, uint32_t is_2_4G);

#endif //_TX_EVM_PUB_H_
// eof


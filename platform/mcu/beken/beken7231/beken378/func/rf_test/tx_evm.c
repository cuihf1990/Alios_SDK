#include "include.h"
#include "tx_evm_pub.h"
#include "tx_evm.h"

#include "mac_phy_bypass_pub.h"
#include "uart_pub.h"

#include "mac.h"
#include "phy.h"
#include "hal_machw.h"
#include "me.h"
#include "mm.h"
#include "ke_task.h"
#include "vif_mgmt.h"

#include "drv_model_pub.h"
#include "target_util_pub.h"
#include "ke_event.h"

#if CFG_TX_EVM_TEST
UINT32 evm_mac_pkt_count = 0;
UINT32 evm_mac_pkt_max = 500000;
UINT32 evm_channel = EVM_DEFAULT_CHANNEL;
UINT32 evm_test_via_mac_flag = 0;
	
struct mac_addr const evm_mac_addr = {
								    {0x7112, 0x7111, 0x7111}
								 };

void evm_init(UINT32 channel)
{
	BOOL p2p = 0;
	UINT8 vif_idx;
	UINT8 vif_type = 2;
	struct phy_cfg_tag cfg;
	
	/*reset mm*/
	EVM_PRT("[EVM]reset_mm\r\n");
	hal_machw_stop();
	phy_stop();
	me_init();
	mm_init();
	ke_state_set(TASK_MM, MM_IDLE);
	
	/*config me*/
	EVM_PRT("[EVM]config_me\r\n");
	
	/*config me channel*/
	EVM_PRT("[EVM]config_me_channel\r\n");
	
	/*start mm*/
	EVM_PRT("[EVM]start_mm\r\n");
	cfg.parameters[0] = 1;
	cfg.parameters[1] = 0;
    phy_init(&cfg);
	
    phy_set_channel(PHY_BAND_2G4, PHY_CHNL_BW_20, channel, channel, 0, PHY_PRIM);
	
	/*add mm interface*/
	EVM_PRT("[EVM]add_mm_interface\r\n");
	vif_mgmt_register(&evm_mac_addr, vif_type, p2p, &vif_idx);

	/* Put the HW in active state*/
	mm_active();

	/*disable rx*/
	nxmac_rx_cntrl_set(0);
}

UINT32 evm_bypass_mac_set_tx_data_length(UINT32 is_legacy_mode, UINT32 len)
{	
	UINT32 ret;
	UINT32 param;

	if(is_legacy_mode)
	{
		param = len & TX_LEGACY_DATA_LEN_MASK;
		ret = sddev_control(MPB_DEV_NAME, MCMD_TX_LEGACY_SET_LEN, &param);
	}
	else
	{
		param = len & TX_HT_VHT_DATA_LEN_MASK;
		ret = sddev_control(MPB_DEV_NAME, MCMD_TX_HT_VHT_SET_LEN, &param);
	}
	
	EVM_PRT("[EVM]tx_mode_bypass_mac_set_length\r\n");

	return ret;
}
	
UINT32 evm_bypass_mac_set_rate(UINT32 ppdu_rate)
{
	UINT32 ret;
	UINT32 param;

	param = ppdu_rate;
	ret = sddev_control(MPB_DEV_NAME, MCMD_TX_BYPASS_MAC_RATE, &param);
	
	EVM_PRT("[EVM]tx_mode_bypass_mac_set_rate\r\n");

	return ret;
}

void evm_bypass_mac_set_channel(UINT32 channel)
{
	evm_channel = channel;
}

void evm_bypass_mac(void)
{
	sddev_control(MPB_DEV_NAME, MCMD_TX_MODE_BYPASS_MAC, 0);
	EVM_PRT("[EVM]tx_mode_bypass_mac\r\n");
}

void evm_bypass_mac_test(void)
{
	evm_init(evm_channel);
	
	evm_bypass_mac();
	EVM_PRT("[EVM]test_bypass_mac\r\n");
}

void evm_via_mac_evt(INT32 dummy)
{
	evm_req_tx(&evm_mac_addr);
	evm_mac_pkt_count ++;
}

uint32_t evm_via_mac_is_start(void)
{
	return evm_test_via_mac_flag;
}

void evm_via_mac_begin(void)
{
	evm_init(evm_channel);
	
	evm_test_via_mac_flag = 1;

	evm_via_mac_evt(0);
}

void evm_via_mac_continue(void)
{
	if(0 == evm_test_via_mac_flag)
	{
		return;
	}
	
	if(evm_mac_pkt_count < evm_mac_pkt_max)
	{
		ke_evt_set(KE_EVT_EVM_MAC_BIT);
	}
	else
	{
		ke_evt_clear(KE_EVT_EVM_MAC_BIT);
	}
}

void evm_via_mac_set_rate(HW_RATE_E rate, uint32_t is_2_4G)
{
	txl_evm_set_hw_rate(rate, is_2_4G);
}

void evm_via_mac_set_channel(UINT32 channel)
{
	evm_channel = channel;
}
#endif // CFG_TX_EVM_TEST
// eof


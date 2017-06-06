#include "include.h"
#include "arm_arch.h"

#include "mac_phy_bypass_pub.h"
#include "mac_phy_bypass.h"

#include "drv_model_pub.h"

UINT32 reg_134 = 0x00;
UINT32 reg_135 = 0xc4;
UINT32 reg_138 = 0x00;
UINT32 reg_139 = 0x10;
UINT32 reg_140 = 0x00;

#if CFG_MAC_PHY_BAPASS
struct MPB_TypeDef mpb_regs =
{
    (volatile MPB_REG0x0_TypeDef  *)(MPB_ADDR_BASE + 0 * 4),
    (volatile MPB_REG0x1_TypeDef  *)(MPB_ADDR_BASE + 1 * 4),
    (volatile MPB_REG0x2_TypeDef  *)(MPB_ADDR_BASE + 2 * 4),
    (volatile MPB_REG0x3_TypeDef  *)(MPB_ADDR_BASE + 3 * 4),
    (volatile MPB_REG0x4_TypeDef  *)(MPB_ADDR_BASE + 4 * 4),
    (volatile MPB_REG0x8_TypeDef  *)(MPB_ADDR_BASE + 8 * 4),
    (volatile MPB_REG0x9_TypeDef  *)(MPB_ADDR_BASE + 9 * 4),
    (volatile MPB_REG0xA_TypeDef  *)(MPB_ADDR_BASE + 10 * 4),    
    (volatile MPB_REG0xB_TypeDef  *)(MPB_ADDR_BASE + 11 * 4),
    
    (volatile MPB_REG0x80_TypeDef  *)(MPB_ADDR_BASE + 128 * 4),
    (volatile MPB_REG0x81_TypeDef  *)(MPB_ADDR_BASE + 129 * 4),
    (volatile MPB_REG0x82_TypeDef  *)(MPB_ADDR_BASE + 130 * 4),
    (volatile MPB_REG0x83_TypeDef  *)(MPB_ADDR_BASE + 131 * 4),
    (volatile MPB_REG0x84_TypeDef  *)(MPB_ADDR_BASE + 132 * 4),
    (volatile MPB_REG0x85_TypeDef  *)(MPB_ADDR_BASE + 133 * 4),
    (volatile MPB_REG0x86_TypeDef  *)(MPB_ADDR_BASE + 134 * 4),
    (volatile MPB_REG0x87_TypeDef  *)(MPB_ADDR_BASE + 135 * 4),
    (volatile MPB_REG0x88_TypeDef  *)(MPB_ADDR_BASE + 136 * 4),
    (volatile MPB_REG0x89_TypeDef  *)(MPB_ADDR_BASE + 137 * 4),
    (volatile MPB_REG0x8A_TypeDef  *)(MPB_ADDR_BASE + 138 * 4),
    (volatile MPB_REG0x8B_TypeDef  *)(MPB_ADDR_BASE + 139 * 4),
    (volatile MPB_REG0x8C_TypeDef  *)(MPB_ADDR_BASE + 140 * 4),
    (volatile MPB_REG0x8D_TypeDef  *)(MPB_ADDR_BASE + 141 * 4),
    (volatile MPB_REG0x8E_TypeDef  *)(MPB_ADDR_BASE + 142 * 4),
    (volatile MPB_REG0x8F_TypeDef  *)(MPB_ADDR_BASE + 143 * 4),
};

static SDD_OPERATIONS mpb_op = {
            mpb_ctrl
};

void mpb_init(void)
{
	sddev_register_dev(MPB_DEV_NAME, &mpb_op);
}

void mpb_exit(void)
{
	sddev_unregister_dev(MPB_DEV_NAME);
}

void mpb_tx_mode(void)
{
    mpb_regs.r0->value   = 0x00;
    mpb_regs.r128->value = 0x34;
    mpb_regs.r129->value = 0x00;
    mpb_regs.r130->value = 0x00;
    mpb_regs.r131->value = 0x00;
    mpb_regs.r132->value = 0x81;
    mpb_regs.r133->value = 0x00;
    mpb_regs.r134->value = reg_134;
    mpb_regs.r135->value = reg_135;
    mpb_regs.r136->value = 0x00;
    mpb_regs.r137->value = 0x00;
    mpb_regs.r138->value = reg_138;
    mpb_regs.r139->value = reg_139;
    mpb_regs.r140->value = reg_140;
    mpb_regs.r141->value = 0x00;
    mpb_regs.r142->value = 0x00;
    mpb_regs.r143->value = 0xff;
    mpb_regs.r3->value   = 0x177;
    mpb_regs.r2->value   = 0x10;
    mpb_regs.r0->value   = 0x01;
}

void mpb_rx_mode(void)
{
	mpb_regs.r0->value = 0x00;
	mpb_regs.r1->value = 0x05;
	mpb_regs.r0->value = 0x21;
}

static UINT32 mpb_select_tx_rate(UINT32 rate)
{
    UINT32 param;
    
    switch(rate)
    {
        case 1 :	param = 0x0;	break;  // 1Mbps
        case 2 :	param = 0x1;	break;  // 2Mbps
        case 5 :	param = 0x2;	break;	// 5.5Mbps
        case 11:	param = 0x3;	break;	// 11Mbps
        case 6 :	param = 0xb;	break;	// 6Mbps
        case 9 :	param = 0xf;	break;	// 9Mbps
        case 12:	param = 0xa;	break;	// 12Mbps
        case 18:	param = 0xe;	break;	// 18Mbps
        case 24:	param = 0x9;	break;	// 24Mbps
        case 36:	param = 0xd;	break;	// 36Mbps
        case 48:	param = 0x8;	break;	// 48Mbps
        case 54:	param = 0xc;	break;	// 54Mbps
        default:    param = 0xc;    break;	// 54Mbps
    }  

    return param;
}

UINT32 mpb_ctrl(UINT32 cmd, void *param)
{
	UINT32 len;
	UINT32 reg;
	
	switch(cmd)
	{
		case MCMD_TX_BYPASS_MAC_RATE:
            len = mpb_select_tx_rate((*(UINT32*)param));
            reg = reg_135 & (~(PPDU_RATE_MASK << PPDU_RATE_POSI));
			reg |= (len  & PPDU_RATE_MASK) << PPDU_RATE_POSI;
			reg_135 = reg;
			break;

		case MCMD_TX_LEGACY_SET_LEN:
			len = (*(UINT32*)param);
            reg_134 &= ~(0xff);
            reg_135 &= ~(0xf);
			reg_134 |= len & 0xff;
			reg_135 |= (len >> 8) & 0xf;
			break;

		case MCMD_TX_HT_VHT_SET_LEN:
			len = (*(UINT32*)param);
            reg_138 &= ~(0xff);
            reg_139 &= ~(0xff);
            reg_140 &= ~(0xf);
			reg_138 = len & 0xff;
			reg_139 = (len >> 8) & 0xff;
			reg_140 = (len >> 16) & 0xf;
			break;
			
		case MCMD_TX_MODE_BYPASS_MAC:
			mpb_tx_mode();
			break;
			
		case MCMD_RX_MODE_BYPASS_MAC:
			mpb_rx_mode();
			break;
			
		default:
			break;
	}
	
	return 0;
}

#endif
// eof

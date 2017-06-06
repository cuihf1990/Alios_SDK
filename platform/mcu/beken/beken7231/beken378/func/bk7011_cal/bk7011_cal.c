#include "include.h"
#include "arm_arch.h"

#include "target_util_pub.h"
#include "mem_pub.h"

#include "drv_model_pub.h"
#include "sys_ctrl_pub.h"
#include "phy.h"

#include "bk7011_cal_pub.h"
#include "bk7011_cal.h"

#if CFG_SUPPORT_CALIBRATION

#define BK7211_68PIN_BOARD      0    // default:0  PIN48

#include "uart_pub.h"

static INT32 gtx_dc_n = 3;
static UINT32 gst_rx_adc = CAL_DELAY100US;
static UINT32 gst_sar_adc = CAL_DELAY05US;
static UINT8 pwr_nstep = 0;

#define BK7231_56PIN_BOARD  1
#define BK7231_32PIN_BOARD  2
#define BK7231_32PIN_TZH_BOARD  3

#define BOARD  BK7231_32PIN_TZH_BOARD

#if (BOARD == BK7231_56PIN_BOARD)
#define TRX_REG_0XA_VAL         0x83703274
#define TRX_REG_0XB_VAL         0x20240077   
#define TRX_REG_0XC_VAL         0x01A267EE
#define TSSI_POUT_TH            0x50   //0x50:RF components for best power,0x43:for best sens
#else
    #if (BOARD == BK7231_32PIN_BOARD)
	    #define TRX_REG_0XA_VAL         0x83703274
	    #define TRX_REG_0XB_VAL         0x20246077   
	    #define TRX_REG_0XC_VAL         0x01A287EE
	    #define TSSI_POUT_TH            0x80
    #else
        #if (BOARD == BK7231_32PIN_TZH_BOARD)
        #define TRX_REG_0XA_VAL         0x83703274
        #define TRX_REG_0XB_VAL         0x20242077   
        #define TRX_REG_0XC_VAL         0x01A287EE
        #define TSSI_POUT_TH            0x75
        #endif
    #endif
#endif

#define No_TXPOWERCAL   0
#define TXIQ_TSSI_TH             0x40
#define TRX_REG_0XD_TX_IQ_VAL    0xF9FF0338   //0214 
#define TRX_REG_0XD_TX_LOOPBACK_IQ_VAL    0xF9FE7FF1   //0214 

#define TRX_REG_0XC_TXLO_LEAKAGE_VAL     0x1A28188

#define TRX_REG_0XC_RXIQ_VAL    0x1A245ED  // 1A244ED    //0215 increasse gain due to load antenna   01A244DD
#define TRX_REG_0XE_RXIQ_VAL    0xF9F87FF1   //0214  FDF87FF1


#define CALIBRATE_TIMES     200
//#undef  CALIBRATE_TIMES         // by gwf

#ifdef CALIBRATE_TIMES
int calibrate_time = 0;

#define gi_dc_tx_pa_dgainPA20  2
#define gi_dc_tx_pa_dgainbuf20  6
#define gi_gain_tx_pa_dgainPA20  2
#define gi_gain_tx_pa_dgainbuf20  4

#define gi_dc_tx_loopback_pa_dgainPA20  5
#define gi_dc_tx_loopback_pa_dgainbuf20  7
#define gi_gain_tx_loopback_pa_dgainPA20  6
#define gi_gain_tx_loopback_pa_dgainbuf20  5

int *p_gbias_after_cal_array = NULL;
int *p_gav_tssi_array = NULL;
int *p_gtx_ifilter_corner_array = NULL;
int *p_gtx_qfilter_corner_array = NULL;
int *p_rx_amp_err_rd_array = NULL;
int *p_rx_phase_err_rd_array = NULL;

int *p_gtx_i_dc_comp_array = NULL;
int *p_gtx_q_dc_comp_array = NULL;
int *p_gtx_i_gain_comp_array = NULL;
int *p_gtx_q_gain_comp_array = NULL;
int *p_gtx_phase_comp_array = NULL;

int *p_gtx_i_dc_comp_temp_array = NULL;
int *p_gtx_q_dc_comp_temp_array = NULL;
int *p_gtx_i_gain_comp_temp_array = NULL;
int *p_gtx_q_gain_comp_temp_array = NULL;
int *p_gtx_phase_comp_temp_array = NULL;

int *p_g_rx_dc_gain_tab_array = NULL;
#endif

static INT32 bk7011_trx_val[32] =
{
    0x0811EF5E,    // 0
    0x01000000,
    0x00000000,
    0x00000000,
    0x38025E88,
    0x2555EC7A,    // 5
    0x5FA44104,//0x5FA4410C,
    0x207C48F5,
    0x076CA9CA,
    0x000003AF,    // 9,from 3AF to 3BF ,add fixed 6dB gain
    TRX_REG_0XA_VAL,//0x83373330, 
    TRX_REG_0XB_VAL,//0x4A444B24,
    TRX_REG_0XC_VAL,//0x01A187EE,   // 0x01A147FE is use for high temp calib
    0xDDF90339,// 0xD
    0xDA01BCF0,// 0xE
    0x00018000,// 0xF ADC sample rate clk 40M    
    0xD0000000,
    0x00000000,
    0xF0090481,//0x12,from 0xF0090501 to 0xF0090481 ad/da LDO voltage reduced 50mv  
    0x7B305ECC,
    0x827C827C,    // 0x14
    0x86788678,
    0x8C748C74,
    0xA45F9868,
    0xA45FA45F,
    0xA55EA45F,    // 0x19
    0xA55DA55E,
    0xA55DA55D,
    0x20000000     // 0x1C
};

static INT32 bk7011_rc_val[32] =
{
    0x00000009,//00 REG_0x0,  0x0000;
    0xF0000000,//01 REG_0x1,  0x0004;
    0x00000030,//02 REG_0x5,  0x0014;
    0x00010001,//03 REG_0x8,  0x0020;
    0x000100e0,//04 REG_0xB,  0x002C;
    0x00010070,//05 REG_0xE,  0x0038;
    0x00010001,//06 REG_0x11, 0x0044;
    0x00010005,//07 REG_0x19, 0x0064;
    0x00000002,//08 REG_0x1c, 0x0070;
    0x0000012c,//09 REG_0x1E, 0x0078;
    0x1002DF4B,//10 REG_0x3C, 0x00F0;
    0x00000000,//11 REG_0x3E, 0x00F8;   // disable comp bit
    0x03E803CB,//12 REG_0x3F, 0x00FC;
    0x00000001,//13 REG_0x40, 0x0100;
    0x00000000,//14 REG_0x41, 0x0104;
    0x02000041,//15 REG_0x42, 0x0108;
    0x218B018B,//16 REG_0x4C, 0x0130;
    0x2CC02000,//17 REG_0x4D, 0x0134;
    0x020201F1,//18 REG_0x4F, 0x013C;
    0x03FF03F3,//19 REG_0x50, 0x0140;
    0x01F80200,//20 REG_0x51, 0x0144;
#if BK7211_68PIN_BOARD
    0x48A79D40,//21 REG_0x52, 0x0148;
    0x00025640,//0x0002567A;//22 REG_0x54, 0x0150;
#else
    0x50079D40,//21 REG_0x52, 0x0148;  0x4A443B24   0x53479D40
    (0x0002e600 | ((TSSI_POUT_TH)<<1)),//22 REG_0x54, 0x0150;  0x00025646-1us  2E600
#endif
    0x00000000,//23 REG_0x55, 0x0154;
    0x80000064,//24 REG_0x5C, 0x0170;
};

UINT32 tx_pwr_rate[12][3] =
{
    0
};

static UINT8 gstat_cal = 1;  // 1: calibration not done, 0: done

INT32 gconst_iqcal_p = 117;
INT32 gconst_pout = 0x210;
INT32 gav_tssi = 0;
INT32 grc_reg_map[32];
UINT8 gbias_after_cal = 0;
INT32 gtx_dcorMod = 0x4;
INT32 gtx_dcorPA = 0x8;
INT32 gtx_pre_gain = 0x7;

INT32 gtx_i_dc_comp = 0x202;
INT32 gtx_q_dc_comp = 0x1ff;

INT32 gtx_i_gain_comp = 0x3ff;
INT32 gtx_q_gain_comp = 0x3f3;

INT32 gtx_ifilter_corner = 0x10;
INT32 gtx_qfilter_corner = 0x10;
INT32 gtx_phase_comp = 0x200;
INT32 gtx_phase_ty2 = 0x200;

INT32 gtxoutput = 0;

INT32 g_rx_dc_gain_tab[8] =
{
    0x827C827C,
    0x86788678,
    0x8C748C74,
    0xA45F9868,
    0xA45FA45F,
    0xA55EA45F,
    0xA55DA55E,
    0xA55DA55D
};

INT32 grx_amp_err_wr = 0x200;
INT32 grx_phase_err_wr = 0x041;

struct BK7011RCBEKEN_TypeDef BK7011RCBEKEN =
{
    (volatile BK7011_RC_BEKEN_REG0x0_TypeDef *) (RC_BEKEN_BASE + 0 * 4),
    (volatile BK7011_RC_BEKEN_REG0x1_TypeDef *) (RC_BEKEN_BASE + 1 * 4),
    (volatile BK7011_RC_BEKEN_REG0x5_TypeDef *) (RC_BEKEN_BASE + 5 * 4),
    (volatile BK7011_RC_BEKEN_REG0x8_TypeDef *) (RC_BEKEN_BASE + 8 * 4),
    (volatile BK7011_RC_BEKEN_REG0xB_TypeDef *) (RC_BEKEN_BASE + 11 * 4),
    (volatile BK7011_RC_BEKEN_REG0xE_TypeDef *) (RC_BEKEN_BASE + 14 * 4),
    (volatile BK7011_RC_BEKEN_REG0x11_TypeDef *)(RC_BEKEN_BASE + 17 * 4),
    (volatile BK7011_RC_BEKEN_REG0x19_TypeDef *)(RC_BEKEN_BASE + 25 * 4),
    (volatile BK7011_RC_BEKEN_REG0x1C_TypeDef *)(RC_BEKEN_BASE + 28 * 4),
    (volatile BK7011_RC_BEKEN_REG0x1E_TypeDef *)(RC_BEKEN_BASE + 30 * 4),
    (volatile BK7011_RC_BEKEN_REG0x3C_TypeDef *)(RC_BEKEN_BASE + 60 * 4),
    (volatile BK7011_RC_BEKEN_REG0x3E_TypeDef *)(RC_BEKEN_BASE + 62 * 4),
    (volatile BK7011_RC_BEKEN_REG0x3F_TypeDef *)(RC_BEKEN_BASE + 63 * 4),
    (volatile BK7011_RC_BEKEN_REG0x40_TypeDef *)(RC_BEKEN_BASE + 64 * 4),
    (volatile BK7011_RC_BEKEN_REG0x41_TypeDef *)(RC_BEKEN_BASE + 65 * 4),
    (volatile BK7011_RC_BEKEN_REG0x42_TypeDef *)(RC_BEKEN_BASE + 66 * 4),
    (volatile BK7011_RC_BEKEN_REG0x4C_TypeDef *)(RC_BEKEN_BASE + 76 * 4),
    (volatile BK7011_RC_BEKEN_REG0x4D_TypeDef *)(RC_BEKEN_BASE + 77 * 4),
    (volatile BK7011_RC_BEKEN_REG0x4F_TypeDef *)(RC_BEKEN_BASE + 79 * 4),
    (volatile BK7011_RC_BEKEN_REG0x50_TypeDef *)(RC_BEKEN_BASE + 80 * 4),
    (volatile BK7011_RC_BEKEN_REG0x51_TypeDef *)(RC_BEKEN_BASE + 81 * 4),
    (volatile BK7011_RC_BEKEN_REG0x52_TypeDef *)(RC_BEKEN_BASE + 82 * 4),
    (volatile BK7011_RC_BEKEN_REG0x54_TypeDef *)(RC_BEKEN_BASE + 84 * 4),
    (volatile BK7011_RC_BEKEN_REG0x55_TypeDef *)(RC_BEKEN_BASE + 85 * 4),
    (volatile BK7011_RC_BEKEN_REG0x5C_TypeDef *)(RC_BEKEN_BASE + 92 * 4),
    (volatile BK7011_RC_BEKEN_REG0x6A_TypeDef *)(RC_BEKEN_BASE + 106 * 4),
};

struct BK7011TRxV2A_TypeDef BK7011TRX =
{
    (BK7011_TRxV2A_REG0x0_TypeDef *)(&grc_reg_map[0]),
    (BK7011_TRxV2A_REG0x1_TypeDef *)(&grc_reg_map[1]),
    (BK7011_TRxV2A_REG0x2_TypeDef *)(&grc_reg_map[2]),
    (BK7011_TRxV2A_REG0x3_TypeDef *)(&grc_reg_map[3]),
    (BK7011_TRxV2A_REG0x4_TypeDef *)(&grc_reg_map[4]),
    (BK7011_TRxV2A_REG0x5_TypeDef *)(&grc_reg_map[5]),
    (BK7011_TRxV2A_REG0x6_TypeDef *)(&grc_reg_map[6]),
    (BK7011_TRxV2A_REG0x7_TypeDef *)(&grc_reg_map[7]),
    (BK7011_TRxV2A_REG0x8_TypeDef *)(&grc_reg_map[8]),
    (BK7011_TRxV2A_REG0x9_TypeDef *)(&grc_reg_map[9]),
    (BK7011_TRxV2A_REG0xA_TypeDef *)(&grc_reg_map[10]),
    (BK7011_TRxV2A_REG0xB_TypeDef *)(&grc_reg_map[11]),
    (BK7011_TRxV2A_REG0xC_TypeDef *)(&grc_reg_map[12]),
    (BK7011_TRxV2A_REG0xD_TypeDef *)(&grc_reg_map[13]),
    (BK7011_TRxV2A_REG0xE_TypeDef *)(&grc_reg_map[14]),
    (BK7011_TRxV2A_REG0xF_TypeDef *)(&grc_reg_map[15]),
    (BK7011_TRxV2A_REG0x10_TypeDef *)(&grc_reg_map[16]),
    (BK7011_TRxV2A_REG0x11_TypeDef *)(&grc_reg_map[17]),
    (BK7011_TRxV2A_REG0x12_TypeDef *)(&grc_reg_map[18]),
    (BK7011_TRxV2A_REG0x13_TypeDef *)(&grc_reg_map[19]),
    (BK7011_TRxV2A_REG0x14_TypeDef *)(&grc_reg_map[20]),
    (BK7011_TRxV2A_REG0x15_TypeDef *)(&grc_reg_map[21]),
    (BK7011_TRxV2A_REG0x16_TypeDef *)(&grc_reg_map[22]),
    (BK7011_TRxV2A_REG0x17_TypeDef *)(&grc_reg_map[23]),
    (BK7011_TRxV2A_REG0x18_TypeDef *)(&grc_reg_map[24]),
    (BK7011_TRxV2A_REG0x19_TypeDef *)(&grc_reg_map[25]),
    (BK7011_TRxV2A_REG0x1A_TypeDef *)(&grc_reg_map[26]),
    (BK7011_TRxV2A_REG0x1B_TypeDef *)(&grc_reg_map[27]),
    (BK7011_TRxV2A_REG0x1C_TypeDef *)(&grc_reg_map[28]),
};

struct BK7011TRxV2A_TypeDef BK7011TRXONLY =
{
    (volatile BK7011_TRxV2A_REG0x0_TypeDef *) (TRX_BEKEN_BASE + 0 * 4),
    (volatile BK7011_TRxV2A_REG0x1_TypeDef *) (TRX_BEKEN_BASE + 1 * 4),
    (volatile BK7011_TRxV2A_REG0x2_TypeDef *) (TRX_BEKEN_BASE + 2 * 4),
    (volatile BK7011_TRxV2A_REG0x3_TypeDef *) (TRX_BEKEN_BASE + 3 * 4),
    (volatile BK7011_TRxV2A_REG0x4_TypeDef *) (TRX_BEKEN_BASE + 4 * 4),
    (volatile BK7011_TRxV2A_REG0x5_TypeDef *) (TRX_BEKEN_BASE + 5 * 4),
    (volatile BK7011_TRxV2A_REG0x6_TypeDef *) (TRX_BEKEN_BASE + 6 * 4),
    (volatile BK7011_TRxV2A_REG0x7_TypeDef *) (TRX_BEKEN_BASE + 7 * 4),
    (volatile BK7011_TRxV2A_REG0x8_TypeDef *) (TRX_BEKEN_BASE + 8 * 4),
    (volatile BK7011_TRxV2A_REG0x9_TypeDef *) (TRX_BEKEN_BASE + 9 * 4),
    (volatile BK7011_TRxV2A_REG0xA_TypeDef *) (TRX_BEKEN_BASE + 10 * 4),
    (volatile BK7011_TRxV2A_REG0xB_TypeDef *) (TRX_BEKEN_BASE + 11 * 4),
    (volatile BK7011_TRxV2A_REG0xC_TypeDef *) (TRX_BEKEN_BASE + 12 * 4),
    (volatile BK7011_TRxV2A_REG0xD_TypeDef *) (TRX_BEKEN_BASE + 13 * 4),
    (volatile BK7011_TRxV2A_REG0xE_TypeDef *) (TRX_BEKEN_BASE + 14 * 4),
    (volatile BK7011_TRxV2A_REG0xF_TypeDef *) (TRX_BEKEN_BASE + 15 * 4),
    (volatile BK7011_TRxV2A_REG0x10_TypeDef *)(TRX_BEKEN_BASE + 16 * 4),
    (volatile BK7011_TRxV2A_REG0x11_TypeDef *)(TRX_BEKEN_BASE + 17 * 4),
    (volatile BK7011_TRxV2A_REG0x12_TypeDef *)(TRX_BEKEN_BASE + 18 * 4),
    (volatile BK7011_TRxV2A_REG0x13_TypeDef *)(TRX_BEKEN_BASE + 19 * 4),
    (volatile BK7011_TRxV2A_REG0x14_TypeDef *)(TRX_BEKEN_BASE + 20 * 4),
    (volatile BK7011_TRxV2A_REG0x15_TypeDef *)(TRX_BEKEN_BASE + 21 * 4),
    (volatile BK7011_TRxV2A_REG0x16_TypeDef *)(TRX_BEKEN_BASE + 22 * 4),
    (volatile BK7011_TRxV2A_REG0x17_TypeDef *)(TRX_BEKEN_BASE + 23 * 4),
    (volatile BK7011_TRxV2A_REG0x18_TypeDef *)(TRX_BEKEN_BASE + 24 * 4),
    (volatile BK7011_TRxV2A_REG0x19_TypeDef *)(TRX_BEKEN_BASE + 25 * 4),
    (volatile BK7011_TRxV2A_REG0x1A_TypeDef *)(TRX_BEKEN_BASE + 26 * 4),
    (volatile BK7011_TRxV2A_REG0x1B_TypeDef *)(TRX_BEKEN_BASE + 27 * 4),
    (volatile BK7011_TRxV2A_REG0x1C_TypeDef *)(TRX_BEKEN_BASE + 28 * 4),
};

void delay05us(INT32 num)
{
    volatile INT32 i, j;

    for(i = 0; i < num; i ++)
    {
        for(j = 0; j < 5; j ++)
            ;
    }
}

void delay100us(INT32 num)
{
    volatile INT32 i, j;

    for(i = 0; i < num; i ++)
    {
        for(j = 0; j < 1050; j ++)
            ;
    }
}


#define CAL_WR_TRXREGS(reg)    do{\
                                    while(BK7011RCBEKEN.REG0x1->value & (0x1 << reg));\
                                    BK7011TRXONLY.REG##reg->value = BK7011TRX.REG##reg->value;\
                                    cal_delay(6);\
                                    while(BK7011RCBEKEN.REG0x1->value & (0x1 << reg));\
                                }while(0)


void rwnx_cal_load_default_result(void)
{
    gtx_dcorMod = (bk7011_trx_val[11] >> 12) & 0xf;
    gtx_dcorPA = (bk7011_trx_val[12] >> 12) & 0xf;
    gtx_pre_gain = (bk7011_rc_val[21] >> 16) & 0x1f;

    gtx_i_dc_comp = (bk7011_rc_val[18] >> 16) & 0x3ff;
    gtx_q_dc_comp = bk7011_rc_val[18] & 0x3ff;

    gtx_i_gain_comp = (bk7011_rc_val[19] >> 16) & 0x3ff;
    gtx_q_gain_comp = bk7011_rc_val[19] & 0x3ff;

    gtx_ifilter_corner = (bk7011_trx_val[6] >> 10) & 0x3f;
    gtx_qfilter_corner = (bk7011_trx_val[6] >> 4) & 0x3f;
    gtx_phase_comp = (bk7011_rc_val[20] >> 16) & 0x3ff;
    gtx_phase_ty2 = bk7011_rc_val[20] & 0x3ff;

    g_rx_dc_gain_tab[0] = bk7011_trx_val[20];
    g_rx_dc_gain_tab[1] = bk7011_trx_val[21];
    g_rx_dc_gain_tab[2] = bk7011_trx_val[22];
    g_rx_dc_gain_tab[3] = bk7011_trx_val[23];
    g_rx_dc_gain_tab[4] = bk7011_trx_val[24];
    g_rx_dc_gain_tab[5] = bk7011_trx_val[25];
    g_rx_dc_gain_tab[6] = bk7011_trx_val[26];
    g_rx_dc_gain_tab[7] = bk7011_trx_val[27];

    grx_amp_err_wr = (bk7011_rc_val[15] >> 16) & 0x3ff;
    grx_phase_err_wr = bk7011_rc_val[15] & 0x3ff;

    gstat_cal = (bk7011_rc_val[16] >> 29) & 0x1;
}

#ifdef CALIBRATE_TIMES
void calibration_result_print(void)
{
    int i, j;
    int max, min;

    max = -1100, min = 1100;

    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_gbias_after_cal_array[i])
        {
            max = p_gbias_after_cal_array[i];
        }
        if (min > p_gbias_after_cal_array[i])
        {
            min = p_gbias_after_cal_array[i];
        }
    }
    CAL_PRT("gbias_after_cal: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);

    max = -1100, min = 1100;

    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_gav_tssi_array[i])
        {
            max = p_gav_tssi_array[i];
        }
        if (min > p_gav_tssi_array[i])
        {
            min = p_gav_tssi_array[i];
        }
    }
    CAL_PRT("gav_tssi: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);

    CAL_PRT("\r\n");

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_gtx_ifilter_corner_array[i])
        {
            max = p_gtx_ifilter_corner_array[i];
        }
        if (min > p_gtx_ifilter_corner_array[i])
        {
            min = p_gtx_ifilter_corner_array[i];
        }
    }
    CAL_PRT("gtx_ifilter_corner: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_gtx_qfilter_corner_array[i])
        {
            max = p_gtx_qfilter_corner_array[i];
        }
        if (min > p_gtx_qfilter_corner_array[i])
        {
            min = p_gtx_qfilter_corner_array[i];
        }
    }
    CAL_PRT("gtx_qfilter_corner: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);

    CAL_PRT("\r\n");

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_gtx_i_dc_comp_array[i])
        {
            max = p_gtx_i_dc_comp_array[i];
        }
        if (min > p_gtx_i_dc_comp_array[i])
        {
            min = p_gtx_i_dc_comp_array[i];
        }
    }
    CAL_PRT("tx_i_dc_comp: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_gtx_q_dc_comp_array[i])
        {
            max = p_gtx_q_dc_comp_array[i];
        }
        if (min > p_gtx_q_dc_comp_array[i])
        {
            min = p_gtx_q_dc_comp_array[i];
        }
    }
    CAL_PRT("tx_q_dc_comp: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_gtx_i_gain_comp_array[i])
        {
            max = p_gtx_i_gain_comp_array[i];
        }
        if (min > p_gtx_i_gain_comp_array[i])
        {
            min = p_gtx_i_gain_comp_array[i];
        }
    }
    CAL_PRT("tx_i_gain_comp: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_gtx_q_gain_comp_array[i])
        {
            max = p_gtx_q_gain_comp_array[i];
        }
        if (min > p_gtx_q_gain_comp_array[i])
        {
            min = p_gtx_q_gain_comp_array[i];
        }
    }
    CAL_PRT("tx_q_gain_comp: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_gtx_phase_comp_array[i])
        {
            max = p_gtx_phase_comp_array[i];
        }
        if (min > p_gtx_phase_comp_array[i])
        {
            min = p_gtx_phase_comp_array[i];
        }
    }
    CAL_PRT("tx_phase_comp: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);

    CAL_PRT("\r\n");
	
    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_gtx_i_dc_comp_temp_array[i])
        {
            max = p_gtx_i_dc_comp_temp_array[i];
        }
        if (min > p_gtx_i_dc_comp_temp_array[i])
        {
            min = p_gtx_i_dc_comp_temp_array[i];
        }
    }
    CAL_PRT("tx_i_dc_comp_temp: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_gtx_q_dc_comp_temp_array[i])
        {
            max = p_gtx_q_dc_comp_temp_array[i];
        }
        if (min > p_gtx_q_dc_comp_temp_array[i])
        {
            min = p_gtx_q_dc_comp_temp_array[i];
        }
    }
    CAL_PRT("tx_q_dc_comp_temp: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);


    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_gtx_i_gain_comp_temp_array[i])
        {
            max = p_gtx_i_gain_comp_temp_array[i];
        }
        if (min > p_gtx_i_gain_comp_temp_array[i])
        {
            min = p_gtx_i_gain_comp_temp_array[i];
        }
    }
    CAL_PRT("tx_i_gain_comp_temp: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_gtx_q_gain_comp_temp_array[i])
        {
            max = p_gtx_q_gain_comp_temp_array[i];
        }
        if (min > p_gtx_q_gain_comp_temp_array[i])
        {
            min = p_gtx_q_gain_comp_temp_array[i];
        }
    }
    CAL_PRT("tx_q_gain_comp_temp: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_gtx_phase_comp_temp_array[i])
        {
            max = p_gtx_phase_comp_temp_array[i];
        }
        if (min > p_gtx_phase_comp_temp_array[i])
        {
            min = p_gtx_phase_comp_temp_array[i];
        }
    }
    CAL_PRT("tx_phase_comp_temp: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);

    CAL_PRT("\r\n");

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_rx_amp_err_rd_array[i])
        {
            max = p_rx_amp_err_rd_array[i];
        }
        if (min > p_rx_amp_err_rd_array[i])
        {
            min = p_rx_amp_err_rd_array[i];
        }
    }
    CAL_PRT("rx_amp_err_rd: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        if (max < p_rx_phase_err_rd_array[i])
        {
            max = p_rx_phase_err_rd_array[i];
        }
        if (min > p_rx_phase_err_rd_array[i])
        {
            min = p_rx_phase_err_rd_array[i];
        }
    }
    CAL_PRT("rx_phase_err_rd: min = %d, max = %d, max-min = %d\r\n", min, max, max-min);

for (j=0; j<8; j++)
{
	int iTemp;
    CAL_PRT("\r\n");

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        iTemp = p_g_rx_dc_gain_tab_array[j*CALIBRATE_TIMES + i];
        if (max < (iTemp & 0x00FF))
        {
            max = iTemp & 0x00FF;
        }
        if (min > (iTemp & 0x00FF))
        {
            min = iTemp & 0x00FF;
        }
    }
    CAL_PRT("g_rx_dc_gain_tab[%d].i_%ddb: min = %d, max = %d, max-min = %d\r\n", j, j*6, min, max, max-min);

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        iTemp = p_g_rx_dc_gain_tab_array[j*CALIBRATE_TIMES + i]>>8;
        if (max < (iTemp & 0x00FF))
        {
            max = iTemp & 0x00FF;
        }
        if (min > (iTemp & 0x00FF))
        {
            min = iTemp & 0x00FF;
        }
    }
    CAL_PRT("g_rx_dc_gain_tab[%d].q_%ddb: min = %d, max = %d, max-min = %d\r\n", j, j*6, min, max, max-min);

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        iTemp = p_g_rx_dc_gain_tab_array[j*CALIBRATE_TIMES + i]>>16;
        if (max < (iTemp & 0x00FF))
        {
            max = iTemp & 0x00FF;
        }
        if (min > (iTemp & 0x00FF))
        {
            min = iTemp & 0x00FF;
        }
    }
    CAL_PRT("g_rx_dc_gain_tab[%d].i_%ddb: min = %d, max = %d, max-min = %d\r\n", j, j*6+3, min, max, max-min);

    max = -1100, min = 1100;
    for (i=0; i<calibrate_time; i++)
    {
        iTemp = p_g_rx_dc_gain_tab_array[j*CALIBRATE_TIMES + i]>>24;
        if (max < (iTemp & 0x00FF))
        {
            max = iTemp & 0x00FF;
        }
        if (min > (iTemp & 0x00FF))
        {
            min = iTemp & 0x00FF;
        }
    }
    CAL_PRT("g_rx_dc_gain_tab[%d].q_%ddb: min = %d, max = %d, max-min = %d\r\n", j, j*6+3, min, max, max-min);
}
}

void calibration_auto_test(void)
{
    int gbias_after_cal_array[CALIBRATE_TIMES];
    int gav_tssi_array[CALIBRATE_TIMES];
    int rx_amp_err_rd_array[CALIBRATE_TIMES];
    int rx_phase_err_rd_array[CALIBRATE_TIMES];
    int gtx_ifilter_corner_array[CALIBRATE_TIMES];
    int gtx_qfilter_corner_array[CALIBRATE_TIMES];
	
    int gtx_i_dc_comp_array[CALIBRATE_TIMES];
    int gtx_q_dc_comp_array[CALIBRATE_TIMES];
    int gtx_i_gain_comp_array[CALIBRATE_TIMES];
    int gtx_q_gain_comp_array[CALIBRATE_TIMES];
    int gtx_phase_comp_array[CALIBRATE_TIMES];
	
    int gtx_i_dc_comp_temp_array[CALIBRATE_TIMES];
    int gtx_q_dc_comp_temp_array[CALIBRATE_TIMES];
    int gtx_i_gain_comp_temp_array[CALIBRATE_TIMES];
    int gtx_q_gain_comp_temp_array[CALIBRATE_TIMES];
    int gtx_phase_comp_temp_array[CALIBRATE_TIMES];
	
    int g_rx_dc_gain_tab_array[8][CALIBRATE_TIMES];

    p_gbias_after_cal_array = gbias_after_cal_array;
    p_gav_tssi_array = gav_tssi_array;
    p_rx_amp_err_rd_array = rx_amp_err_rd_array;
    p_rx_phase_err_rd_array = rx_phase_err_rd_array;
    p_gtx_ifilter_corner_array = gtx_ifilter_corner_array;
    p_gtx_qfilter_corner_array = gtx_qfilter_corner_array;
    p_gtx_i_dc_comp_array = gtx_i_dc_comp_array;
    p_gtx_q_dc_comp_array = gtx_q_dc_comp_array;
    p_gtx_i_gain_comp_array = gtx_i_gain_comp_array;
    p_gtx_q_gain_comp_array = gtx_q_gain_comp_array;
    p_gtx_phase_comp_array = gtx_phase_comp_array;
    p_gtx_i_dc_comp_temp_array = gtx_i_dc_comp_temp_array;
    p_gtx_q_dc_comp_temp_array = gtx_q_dc_comp_temp_array;
    p_gtx_i_gain_comp_temp_array = gtx_i_gain_comp_temp_array;
    p_gtx_q_gain_comp_temp_array = gtx_q_gain_comp_temp_array;
    p_gtx_phase_comp_temp_array = gtx_phase_comp_temp_array;
    p_g_rx_dc_gain_tab_array = &(g_rx_dc_gain_tab_array[0][0]);

    p_gbias_after_cal_array = NULL;
    p_gav_tssi_array = NULL;
    p_rx_amp_err_rd_array = NULL;
    p_rx_phase_err_rd_array = NULL;
    p_gtx_i_dc_comp_array = NULL;
    p_gtx_q_dc_comp_array = NULL;
    p_gtx_i_gain_comp_array = NULL;
    p_gtx_q_gain_comp_array = NULL;
    p_gtx_phase_comp_array = NULL;
    p_gtx_ifilter_corner_array = NULL;
    p_gtx_qfilter_corner_array = NULL;
    p_gtx_i_dc_comp_temp_array = NULL;
    p_gtx_q_dc_comp_temp_array = NULL;
    p_gtx_i_gain_comp_temp_array = NULL;
    p_gtx_q_gain_comp_temp_array = NULL;
    p_gtx_phase_comp_temp_array = NULL;
    p_g_rx_dc_gain_tab_array = NULL;
}
#endif

void rwnx_cal_read_current_cal_result(void)
{
#ifdef CALIBRATE_TIMES
    if (p_gtx_phase_comp_array != NULL)
    {
        p_gbias_after_cal_array[calibrate_time] = gbias_after_cal;
        p_gav_tssi_array[calibrate_time] = gav_tssi;
        p_gtx_phase_comp_array[calibrate_time] = gtx_phase_comp;
        p_gtx_i_dc_comp_array[calibrate_time] = gtx_i_dc_comp;
        p_gtx_q_dc_comp_array[calibrate_time] = gtx_q_dc_comp;
        p_gtx_i_gain_comp_array[calibrate_time] = gtx_i_gain_comp;
        p_gtx_q_gain_comp_array[calibrate_time] = gtx_q_gain_comp;
        p_gtx_ifilter_corner_array[calibrate_time] = gtx_ifilter_corner;
        p_gtx_qfilter_corner_array[calibrate_time] = gtx_qfilter_corner;
		
		p_g_rx_dc_gain_tab_array[0*CALIBRATE_TIMES + calibrate_time] = g_rx_dc_gain_tab[0];
		p_g_rx_dc_gain_tab_array[1*CALIBRATE_TIMES + calibrate_time] = g_rx_dc_gain_tab[1];
		p_g_rx_dc_gain_tab_array[2*CALIBRATE_TIMES + calibrate_time] = g_rx_dc_gain_tab[2];
		p_g_rx_dc_gain_tab_array[3*CALIBRATE_TIMES + calibrate_time] = g_rx_dc_gain_tab[3];
		p_g_rx_dc_gain_tab_array[4*CALIBRATE_TIMES + calibrate_time] = g_rx_dc_gain_tab[4];
		p_g_rx_dc_gain_tab_array[5*CALIBRATE_TIMES + calibrate_time] = g_rx_dc_gain_tab[5];
		p_g_rx_dc_gain_tab_array[6*CALIBRATE_TIMES + calibrate_time] = g_rx_dc_gain_tab[6];
		p_g_rx_dc_gain_tab_array[7*CALIBRATE_TIMES + calibrate_time] = g_rx_dc_gain_tab[7];
        calibrate_time ++;
    }
    else
#endif
    {
    CAL_FATAL("*********** finally result **********\r\n");
    CAL_FATAL("gbias_after_cal: 0x%x\r\n", gbias_after_cal);
    CAL_FATAL("gav_tssi: 0x%x\r\n", gav_tssi);
    CAL_FATAL("gtx_q_dc_comp:0x%x\r\n", gtx_q_dc_comp);
    CAL_FATAL("gtx_i_dc_comp:0x%x\r\n", gtx_i_dc_comp);
    CAL_FATAL("gtx_i_gain_comp:%d\r\n", gtx_i_gain_comp);
    CAL_FATAL("gtx_q_gain_comp:%d\r\n", gtx_q_gain_comp);
    CAL_FATAL("gtx_phase_comp:%d\r\n", gtx_phase_comp);
    CAL_FATAL("gtx_phase_ty2:%d\r\n", gtx_phase_ty2);
    CAL_FATAL("gtx_ifilter_corner over: 0x%x\r\n", gtx_ifilter_corner);
    CAL_FATAL("gtx_qfilter_corner over: 0x%x\r\n", gtx_qfilter_corner);
    CAL_FATAL("gtx_dcorMod:0x%x, gtx_dcorPA:0x%x\r\n", gtx_dcorMod, gtx_dcorPA);
    CAL_FATAL("gtx_pre_gain:0x%x\r\n", gtx_pre_gain);
    CAL_FATAL("g_rx_dc_gain_tab 0 over: 0x%x\r\n", g_rx_dc_gain_tab[0]);
    CAL_FATAL("g_rx_dc_gain_tab 1 over: 0x%x\r\n", g_rx_dc_gain_tab[1]);
    CAL_FATAL("g_rx_dc_gain_tab 2 over: 0x%x\r\n", g_rx_dc_gain_tab[2]);
    CAL_FATAL("g_rx_dc_gain_tab 3 over: 0x%x\r\n", g_rx_dc_gain_tab[3]);
    CAL_FATAL("g_rx_dc_gain_tab 4 over: 0x%x\r\n", g_rx_dc_gain_tab[4]);
    CAL_FATAL("g_rx_dc_gain_tab 5 over: 0x%x\r\n", g_rx_dc_gain_tab[5]);
    CAL_FATAL("g_rx_dc_gain_tab 6 over: 0x%x\r\n", g_rx_dc_gain_tab[6]);
    CAL_FATAL("g_rx_dc_gain_tab 7 over: 0x%x\r\n", g_rx_dc_gain_tab[7]);

    CAL_FATAL("grx_amp_err_wr:0x%03x\r\n", grx_amp_err_wr);
    CAL_FATAL("grx_phase_err_wr:0x%03x\r\n", grx_phase_err_wr);
    CAL_FATAL("**************************************\r\n");
    }
}


const UINT32 cfg_tab[12][4] = 
{
    // REGB<31:28>	REGC<6:4>	  REGC<2:0>	  REGA<11:8>
        {     5,           7,          7,          7     },  // 0:  1M  
        {     5,           7,          7,          7     },  // 1:  2M  
        {     5,           7,          7,          7     },  // 2:  5.5M
        {     3,           6,          7,          7     },  // 3:  11M 
        
        {     5,           7,          7,          7     },  // 4:  6M  
        {     3,           7,          7,          7     },  // 5:  9M
        {     3,           6,          7,          7     },  // 6:  12M 
        {     3,           6,          6,          7     },  // 7:  18M 
        {     2,           7,          7,          2     },  // 8:  24M 
        {     2,           7,          6,          4     },  // 9:  36M 
        {     2,           7,          6,          2     },  // 10: 48M
        {     2,           6,          6,          2     },  // 11: 54M 
};


void rwnx_cal_generate_txpwr_tabe(void)
{
    UINT32 txpwr_val_A, txpwr_val_B, txpwr_val_C;
    UINT32 i = 0;

    for(i=0; i<12; i++) {
        txpwr_val_A = (TRX_REG_0XA_VAL & (~(0xfu<<8))) | (cfg_tab[i][3]<<8);
        txpwr_val_B = (TRX_REG_0XB_VAL & (~(0xfu<<28))) | (cfg_tab[i][0]<<28);
        txpwr_val_C = (TRX_REG_0XC_VAL & (~(0x7u<<4))) | (cfg_tab[i][1]<<4);
        txpwr_val_C = (txpwr_val_C & (~(0x7u<<0))) | (cfg_tab[i][2]<<0);

        tx_pwr_rate[i][0] = txpwr_val_A;
        tx_pwr_rate[i][1] = txpwr_val_B;
        tx_pwr_rate[i][2] = txpwr_val_C;  

#ifdef CALIBRATE_TIMES
        if (p_gtx_phase_comp_array != NULL)
        {
        }
        else
#endif
        {
            CAL_FATAL("0x%0x, 0x%0x, 0x%0x\r\n", txpwr_val_A, txpwr_val_B, txpwr_val_C);
        }
    }
}

void rwnx_cal_set_txpwr_by_rate(INT32 rate)
{
    UINT32 txpwr_val_A, txpwr_val_B, txpwr_val_C;

    if(rate >= 12)
        return;
		
    txpwr_val_A = tx_pwr_rate[rate][0];
    txpwr_val_B = (tx_pwr_rate[rate][1] & (~(0xf<<12))) | (gtx_dcorMod<<12);
    txpwr_val_C = (tx_pwr_rate[rate][2] & (~(0xf<<12))) | (gtx_dcorPA<<12);  

    BK7011TRX.REG0xA->value = txpwr_val_A;
    CAL_WR_TRXREGS(0xA);
    BK7011TRX.REG0xB->value = txpwr_val_B;
    CAL_WR_TRXREGS(0xB);
    BK7011TRX.REG0xC->value = txpwr_val_C;
    CAL_WR_TRXREGS(0xC);
}

void rwnx_cal_save_cal_result(void)
{	
#if 1
    gtx_dcorMod = 0x6;
    gtx_dcorPA = 0x8;
#endif	

    bk7011_trx_val[11] = (bk7011_trx_val[11] & (~(0xf << 12))) | (((0xf)&gtx_dcorMod) << 12);
    bk7011_trx_val[12] = (bk7011_trx_val[12] & (~(0xf << 12))) | (((0xf)&gtx_dcorPA) << 12);
    bk7011_rc_val[21] = (bk7011_rc_val[21] & (~(0x1f << 16))) | (((0x1f)&gtx_pre_gain) << 16);

#if BK7211_68PIN_BOARD
    //160414,max power for 11b 11M
#ifdef _11MBPS_MAX_POWER
    //REGA[19:16]=0xf
    bk7011_trx_val[0xA] = bk7011_trx_val[0xA] | (0xf << 16);

    //REGB[31:28]=1
    bk7011_trx_val[0xB] = (bk7011_trx_val[0xB] & (~(0xfu << 28))) | (0x1u << 28);
    //REGB[15:12]=0xf
    bk7011_trx_val[0xB] = bk7011_trx_val[0xB] | (0xf << 12);
    //REGB[7:4]=0xf
    bk7011_trx_val[0xB] = bk7011_trx_val[0xB] | (0xf << 4);
    //REGB[2:0]=0xf
    bk7011_trx_val[0xB] = bk7011_trx_val[0xB] | (0x7 << 0);

    //REGC[15:12]=0xf
    bk7011_trx_val[0xC] = bk7011_trx_val[0xC] | (0xf << 12);
    //REGC[6:4]=0x7
    bk7011_trx_val[0xC] = bk7011_trx_val[0xC] | (0x7 << 4);
    //REGC[2:0]=0x7
    bk7011_trx_val[0xC] = bk7011_trx_val[0xC] | (0x7 << 0);
#endif
#endif

    bk7011_rc_val[18] = (bk7011_rc_val[18] & (~(0x3ff << 16))) | (((0x3ff)&gtx_i_dc_comp) << 16);
    bk7011_rc_val[18] = (bk7011_rc_val[18] & (~0x3ff)) | ((0x3ff)&gtx_q_dc_comp);

    bk7011_rc_val[19] = (bk7011_rc_val[19] & (~(0x3ff << 16))) | (((0x3ff)&gtx_i_gain_comp) << 16);
    bk7011_rc_val[19] = (bk7011_rc_val[19] & (~0x3ff)) | ((0x3ff)&gtx_q_gain_comp);

    bk7011_trx_val[6] = (bk7011_trx_val[6] & (~(0x3f << 10))) | (((0x3f)&gtx_ifilter_corner) << 10);
    bk7011_trx_val[6] = (bk7011_trx_val[6] & (~(0x3f << 4))) | (((0x3f)&gtx_qfilter_corner) << 4);

    bk7011_rc_val[20] = (bk7011_rc_val[20] & (~(0x3ff << 16))) | (((0x3ff)&gtx_phase_comp) << 16);
    bk7011_rc_val[20] = (bk7011_rc_val[20] & (~0x3ff)) | ((0x3ff)&gtx_phase_ty2);

    bk7011_trx_val[20] = g_rx_dc_gain_tab[0];
    bk7011_trx_val[21] = g_rx_dc_gain_tab[1];
    bk7011_trx_val[22] = g_rx_dc_gain_tab[2];
    bk7011_trx_val[23] = g_rx_dc_gain_tab[3];
    bk7011_trx_val[24] = g_rx_dc_gain_tab[4];
    bk7011_trx_val[25] = g_rx_dc_gain_tab[5];
    bk7011_trx_val[26] = g_rx_dc_gain_tab[6];
    bk7011_trx_val[27] = g_rx_dc_gain_tab[7];

    bk7011_rc_val[15] = (bk7011_rc_val[15] & (~(0x3ff << 16))) | (((0x3ff)&grx_amp_err_wr) << 16);
    bk7011_rc_val[15] = (bk7011_rc_val[15] & (~0x3ff)) | ((0x3ff)&grx_phase_err_wr);

    if(gstat_cal)
        bk7011_rc_val[16] = bk7011_rc_val[16] | (1 << 29);
    else
        bk7011_rc_val[16] = bk7011_rc_val[16] & (~(1 << 29));
}

/*******************************************************************************
* Function Implemantation
*******************************************************************************/
void bk7011_read_cal_param(void)
{
    gtx_dc_n = (BK7011RCBEKEN.REG0x54->bits.TXDCN & 0x03) + 2;
    gst_sar_adc = ((BK7011RCBEKEN.REG0x54->bits.STSARADC & 0x03) + 1) * CAL_DELAY05US;
    gst_rx_adc = ((BK7011RCBEKEN.REG0x54->bits.STRXADC & 0x03) + 1) *  CAL_DELAY100US;
    gconst_iqcal_p = BK7011RCBEKEN.REG0x52->bits.IQCONSTANTIQCALP - 512;
    gconst_iqcal_p =  abs(gconst_iqcal_p);
    gconst_pout = BK7011RCBEKEN.REG0x52->bits.IQCONSTANTPOUT;

    return;
}

INT32 rwnx_cal_load_trx_rcbekn_reg_val(void)
{
#if (CFG_RUNNING_PLATFORM == FPGA_PLATFORM)
#else
    BK7011RCBEKEN.REG0x0->value  = bk7011_rc_val[0];
    BK7011RCBEKEN.REG0x1->value  = bk7011_rc_val[1];
    BK7011RCBEKEN.REG0x5->value  = bk7011_rc_val[2];
    BK7011RCBEKEN.REG0x8->value  = bk7011_rc_val[3];
    BK7011RCBEKEN.REG0xB->value  = bk7011_rc_val[4];
    BK7011RCBEKEN.REG0xE->value  = bk7011_rc_val[5];
    BK7011RCBEKEN.REG0x11->value = bk7011_rc_val[6];
    BK7011RCBEKEN.REG0x19->value = bk7011_rc_val[7];
    BK7011RCBEKEN.REG0x1C->value = bk7011_rc_val[8];
    BK7011RCBEKEN.REG0x0->value  = bk7011_rc_val[0];
    BK7011RCBEKEN.REG0x1E->value = bk7011_rc_val[9];

    /**********NEW ADDED************/
    BK7011RCBEKEN.REG0x3C->value = bk7011_rc_val[10];
    BK7011RCBEKEN.REG0x3E->value = bk7011_rc_val[11];
    BK7011RCBEKEN.REG0x3F->value = bk7011_rc_val[12];
    BK7011RCBEKEN.REG0x40->value = bk7011_rc_val[13];
    BK7011RCBEKEN.REG0x41->value = bk7011_rc_val[14];
    BK7011RCBEKEN.REG0x42->value = bk7011_rc_val[15];
    BK7011RCBEKEN.REG0x4C->value = bk7011_rc_val[16];
    BK7011RCBEKEN.REG0x4D->value = bk7011_rc_val[17];
    BK7011RCBEKEN.REG0x4F->value = bk7011_rc_val[18];
    BK7011RCBEKEN.REG0x50->value = bk7011_rc_val[19];
    BK7011RCBEKEN.REG0x51->value = bk7011_rc_val[20];
    BK7011RCBEKEN.REG0x52->value = bk7011_rc_val[21];
    BK7011RCBEKEN.REG0x54->value = bk7011_rc_val[22];
    BK7011RCBEKEN.REG0x55->value = bk7011_rc_val[23];
    BK7011RCBEKEN.REG0x5C->value = bk7011_rc_val[24];

    //BK7011RCBEKEN.REG0x3C->bits.RXIQSWAP = 1; /* I/Q SWAP*/

#endif

    os_memcpy(grc_reg_map, bk7011_trx_val, sizeof(INT32) * 29);
    while(BK7011RCBEKEN.REG0x1->value & 0x0FFFFFFF)
    {
        cpu_delay(1);
    }

    BK7011TRXONLY.REG0x0->value = bk7011_trx_val[0];
    BK7011TRXONLY.REG0x1->value = bk7011_trx_val[1];
    BK7011TRXONLY.REG0x2->value = bk7011_trx_val[2];
    BK7011TRXONLY.REG0x3->value = bk7011_trx_val[3];
    BK7011TRXONLY.REG0x4->value = bk7011_trx_val[4];
    BK7011TRXONLY.REG0x5->value = bk7011_trx_val[5];
    BK7011TRXONLY.REG0x6->value = bk7011_trx_val[6];
    BK7011TRXONLY.REG0x7->value = bk7011_trx_val[7];
    BK7011TRXONLY.REG0x8->value = bk7011_trx_val[8];
    BK7011TRXONLY.REG0x9->value = bk7011_trx_val[9];
    BK7011TRXONLY.REG0xA->value = bk7011_trx_val[10];
    BK7011TRXONLY.REG0xB->value = bk7011_trx_val[11];
    BK7011TRXONLY.REG0xC->value = bk7011_trx_val[12];
    BK7011TRXONLY.REG0xD->value = bk7011_trx_val[13];
    BK7011TRXONLY.REG0xE->value = bk7011_trx_val[14];
    BK7011TRXONLY.REG0xF->value = bk7011_trx_val[15];
    BK7011TRXONLY.REG0x10->value = bk7011_trx_val[16];
    BK7011TRXONLY.REG0x11->value = bk7011_trx_val[17];
    BK7011TRXONLY.REG0x12->value = bk7011_trx_val[18];
    BK7011TRXONLY.REG0x13->value = bk7011_trx_val[19];
    BK7011TRXONLY.REG0x14->value = bk7011_trx_val[20];
    BK7011TRXONLY.REG0x15->value = bk7011_trx_val[21];
    BK7011TRXONLY.REG0x16->value = bk7011_trx_val[22];
    BK7011TRXONLY.REG0x17->value = bk7011_trx_val[23];
    BK7011TRXONLY.REG0x18->value = bk7011_trx_val[24];
    BK7011TRXONLY.REG0x19->value = bk7011_trx_val[25];
    BK7011TRXONLY.REG0x1A->value = bk7011_trx_val[26];
    BK7011TRXONLY.REG0x1B->value = bk7011_trx_val[27];

    while(BK7011RCBEKEN.REG0x1->value & 0x0FFFFFFF)
    {
        cpu_delay(1);
    }

#if No_TXPOWERCAL
    BK7011TRX.REG0xB->value = TRX_REG_0XB_VAL;
    CAL_WR_TRXREGS(0xB);
    BK7011TRX.REG0xC->value = TRX_REG_0XC_VAL;
    CAL_WR_TRXREGS(0xC);

    BK7011RCBEKEN.REG0x52->bits.TXPREGAIN = 7;
#endif
    return 0;
}

INT32 rwnx_cal_save_trx_rcbekn_reg_val(void)
{
#if (CFG_RUNNING_PLATFORM == FPGA_PLATFORM)
#else
    bk7011_rc_val[0] = BK7011RCBEKEN.REG0x0->value ;
    bk7011_rc_val[1] = BK7011RCBEKEN.REG0x1->value ;
    bk7011_rc_val[2] = BK7011RCBEKEN.REG0x5->value ;
    bk7011_rc_val[3] = BK7011RCBEKEN.REG0x8->value ;
    bk7011_rc_val[4] = BK7011RCBEKEN.REG0xB->value ;
    bk7011_rc_val[5] = BK7011RCBEKEN.REG0xE->value ;
    bk7011_rc_val[6] = BK7011RCBEKEN.REG0x11->value;
    bk7011_rc_val[7] = BK7011RCBEKEN.REG0x19->value;
    bk7011_rc_val[8] = BK7011RCBEKEN.REG0x1C->value;
    bk7011_rc_val[0] = BK7011RCBEKEN.REG0x0->value ;
    bk7011_rc_val[9] = BK7011RCBEKEN.REG0x1E->value;

    /**********NEW ADDED************/
    bk7011_rc_val[10] = BK7011RCBEKEN.REG0x3C->value;
    bk7011_rc_val[11] = BK7011RCBEKEN.REG0x3E->value;
    bk7011_rc_val[12] = BK7011RCBEKEN.REG0x3F->value;
    bk7011_rc_val[13] = BK7011RCBEKEN.REG0x40->value;
    bk7011_rc_val[14] = BK7011RCBEKEN.REG0x41->value;
    bk7011_rc_val[15] = BK7011RCBEKEN.REG0x42->value;
    bk7011_rc_val[16] = BK7011RCBEKEN.REG0x4C->value;
    bk7011_rc_val[17] = BK7011RCBEKEN.REG0x4D->value;
    bk7011_rc_val[18] = BK7011RCBEKEN.REG0x4F->value;
    bk7011_rc_val[19] = BK7011RCBEKEN.REG0x50->value;
    bk7011_rc_val[20] = BK7011RCBEKEN.REG0x51->value;
    bk7011_rc_val[21] = BK7011RCBEKEN.REG0x52->value;
    bk7011_rc_val[22] = BK7011RCBEKEN.REG0x54->value;
    bk7011_rc_val[23] = BK7011RCBEKEN.REG0x55->value;
    bk7011_rc_val[24] = BK7011RCBEKEN.REG0x5C->value;

    //BK7011RCBEKEN.REG0x3C->bits.RXIQSWAP = 1; /* I/Q SWAP*/

#endif

    bk7011_trx_val[0]  = BK7011TRXONLY.REG0x0->value ;
    bk7011_trx_val[1]  = BK7011TRXONLY.REG0x1->value ;
    bk7011_trx_val[2]  = BK7011TRXONLY.REG0x2->value ;
    bk7011_trx_val[3]  = BK7011TRXONLY.REG0x3->value ;
    bk7011_trx_val[4]  = BK7011TRXONLY.REG0x4->value ;
    bk7011_trx_val[5]  = BK7011TRXONLY.REG0x5->value ;
    bk7011_trx_val[6]  = BK7011TRXONLY.REG0x6->value ;
    bk7011_trx_val[7]  = BK7011TRXONLY.REG0x7->value ;
    bk7011_trx_val[8]  = BK7011TRXONLY.REG0x8->value ;
    bk7011_trx_val[9]  = BK7011TRXONLY.REG0x9->value ;
    bk7011_trx_val[10] = BK7011TRXONLY.REG0xA->value ;
    bk7011_trx_val[11] = BK7011TRXONLY.REG0xB->value ;
    bk7011_trx_val[12] = BK7011TRXONLY.REG0xC->value ;
    bk7011_trx_val[13] = BK7011TRXONLY.REG0xD->value ;
    bk7011_trx_val[14] = BK7011TRXONLY.REG0xE->value ;
    bk7011_trx_val[15] = BK7011TRXONLY.REG0xF->value ;
    bk7011_trx_val[16] = BK7011TRXONLY.REG0x10->value;
    bk7011_trx_val[17] = BK7011TRXONLY.REG0x11->value;
    bk7011_trx_val[18] = BK7011TRXONLY.REG0x12->value;
    bk7011_trx_val[19] = BK7011TRXONLY.REG0x13->value;
    bk7011_trx_val[20] = BK7011TRXONLY.REG0x14->value;
    bk7011_trx_val[21] = BK7011TRXONLY.REG0x15->value;
    bk7011_trx_val[22] = BK7011TRXONLY.REG0x16->value;
    bk7011_trx_val[23] = BK7011TRXONLY.REG0x17->value;
    bk7011_trx_val[24] = BK7011TRXONLY.REG0x18->value;
    bk7011_trx_val[25] = BK7011TRXONLY.REG0x19->value;
    bk7011_trx_val[26] = BK7011TRXONLY.REG0x1A->value;
    bk7011_trx_val[27] = BK7011TRXONLY.REG0x1B->value;

    os_memcpy(grc_reg_map, bk7011_trx_val, sizeof(INT32) * 29);
    while(BK7011RCBEKEN.REG0x1->value & 0x0FFFFFFF)
    {
        cpu_delay(1);
    }

    return 0;
}

void bk7011_cal_ready(void)
{
    rwnx_cal_generate_txpwr_tabe();

    /*step 1: 上电完成*/
    /*step 2/3/4.1: 初始化TRX/ADDA/RC_BEKEN */
    rwnx_cal_load_trx_rcbekn_reg_val();

    bk7011_read_cal_param();
    rwnx_cal_load_default_result();

    cpu_delay(1000);
    /*step 4.2*/
    BK7011RCBEKEN.REG0x4C->bits.TXCOMPDIS = 0;
    gstat_cal = 0; // calibration start

    return;

}

void bk7011_cal_dpll(void)
{
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_CALI_DPLL, NULL);
}

#define BIAS_DIFF_VAL1       (6u)
#define BIAS_DIFF_VAL2       (2u)
void bk7011_cal_bias(void)
{
    UINT32 param, param2;

    param = PARAM_BIAS_CAL_MANUAL_BIT;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BIAS_REG_CLEAN, &param);

    param = PARAM_BIAS_CAL_TRIGGER_BIT;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BIAS_REG_CLEAN, &param);

    cpu_delay(100);

    param = PARAM_BIAS_CAL_TRIGGER_BIT;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BIAS_REG_SET, &param);

    cpu_delay(DELAY1US * 40);//40us = 30 + 10;

    //Read SYS_CTRL.REG0x4C->bias_cal_out
    param = sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BIAS_REG_READ, &param);
    param = (param >> PARAM_BIAS_CAL_OUT_POSI) & PARAM_BIAS_CAL_OUT_MASK;

    //First, Write SYS_CTRL.REG0x4C->ldo_val_man = bias_cal_out + BIAS_DIFF_VAL1
    param += BIAS_DIFF_VAL1;
    param2 = param;
    if (param > 0x1f) param = 0x1f;
    param = ((param & PARAM_BIAS_CAL_OUT_MASK) << PARAM_LDO_VAL_MANUAL_POSI)
            | PARAM_BIAS_CAL_MANUAL_BIT;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BIAS_REG_WRITE, &param);

    //Second, Write SYS_CTRL.REG0x4C->ldo_val_man = ldo_val_man - BIAS_DIFF_VAL2
    param = param2 - BIAS_DIFF_VAL2;
    if (param > 0x1f) param = 0x1f;
    gbias_after_cal = param;
    param = ((param & PARAM_BIAS_CAL_OUT_MASK) << PARAM_LDO_VAL_MANUAL_POSI)
            | PARAM_BIAS_CAL_MANUAL_BIT;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BIAS_REG_WRITE, &param);
    return;
}

void bk7011_cal_pll(void)
{
#if 1
    uint32_t loop = 0, val;

    do
    {
#if 1
        /*reg0x10 enrfpll = 1*/
        BK7011TRX.REG0x10->bits.enrfpll = 1;
        BK7011TRX.REG0x10->bits.endpll = 1;
        CAL_WR_TRXREGS(0x10);

        /*reg0x00 spitrig = 0->1->0*/
        BK7011TRX.REG0x0->bits.spitrig = 0;
        CAL_WR_TRXREGS(0x0);
        BK7011TRX.REG0x0->bits.spitrig = 1;
        CAL_WR_TRXREGS(0x0);
        BK7011TRX.REG0x0->bits.spitrig = 0;
        CAL_WR_TRXREGS(0x0);

        /*reg0x05 spitrigger = 0->1->0*/
        BK7011TRX.REG0x5->bits.spitrigger = 0;
        BK7011TRX.REG0x5->bits.errdetspien = 0;
        CAL_WR_TRXREGS(0x5);
        BK7011TRX.REG0x5->bits.spitrigger = 1;
        CAL_WR_TRXREGS(0x5);
        BK7011TRX.REG0x5->bits.spitrigger = 0;
        CAL_WR_TRXREGS(0x5);
        BK7011TRX.REG0x5->bits.errdetspien = 1;
        CAL_WR_TRXREGS(0x5);
#endif

        cpu_delay(DELAY1US * 10);
        val = BK7011RCBEKEN.REG0x0->bits.ch0ld;
        loop++;
        if(loop >= 10)
            while(1);
        break;
    }
    while(val == 1);

    bk7011_trx_val[5] = (bk7011_trx_val[5] & (~(0x3))) | (0x1);
#endif
}

void bk7011_tx_cal_en(void)
{
    BK7011RCBEKEN.REG0x0->bits.forceenable = 1;
    cpu_delay(1);
    BK7011RCBEKEN.REG0x19->bits.FCH0EN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0SHDN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0RXEN = 0;
    BK7011RCBEKEN.REG0x19->bits.FCH0TXEN = 1;
    BK7011RCBEKEN.REG0x1C->bits.FRXON = 0;
    BK7011RCBEKEN.REG0x1C->bits.FTXON = 1;

    return;
}
static INT32 bk7011_get_tx_output_power(void)
{
#define TSSIRD_COUNT           50
#define TSSIRD_DELAY_INTVAL    2
    INT32 tssioutpower;

    cal_delay(1 * gst_sar_adc);

    if(gtxoutput == 1) // first time for DC & IQ
        tssioutpower = BK7011RCBEKEN.REG0x54->bits.TSSIRD - TXIQ_TSSI_TH - gav_tssi;
    else if (gtxoutput == 5)
        tssioutpower = BK7011RCBEKEN.REG0x54->bits.TSSIRD - TSSI_POUT_TH - gav_tssi;
    else
        tssioutpower = BK7011RCBEKEN.REG0x54->bits.TSSIRD - (TSSI_POUT_TH - 0x30) - gav_tssi;


    tssioutpower = abs(tssioutpower);

    return tssioutpower;
}

#define GET_AV_TSSI_CNT         4

INT32 bk7011_cal_tx_output_power(INT32 *val)
{
    INT32 gold_index = 0;
    INT32 tssilow = 0;
    INT32 tssihigh = 0;
    INT32 index;
    INT16 high, low, tx_fre_gain;

    INT32 cnt = 0;

    BK7011RCBEKEN.REG0x52->bits.TXPREGAIN = 7;

    BK7011TRX.REG0xB->value = TRX_REG_0XB_VAL;
    CAL_WR_TRXREGS(0xB);

    gav_tssi = 0;
    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = 0x200;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = 0x200;
    BK7011TRX.REG0xC->value = TRX_REG_0XC_TXLO_LEAKAGE_VAL; 
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 1;
    if (gtxoutput == 1)
    {
        BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_IQ_VAL;
    }
    else
    {
        BK7011TRX.REG0xD->value = BK7011TRXREG0xD;
    }
    BK7011TRX.REG0xC->bits.TSSIsel = 1;
    BK7011TRX.REG0xC->bits.enDCcali = 0;
    BK7011TRX.REG0xC->bits.enDCcaliGm1 = 0;
    BK7011TRX.REG0xC->bits.enIQcali = 0;
    BK7011TRX.REG0xC->bits.enPcali = 1;
    BK7011TRX.REG0xC->bits.enPcaliGm = 0;
    BK7011TRX.REG0xF->bits.tssicalen = 1;
    CAL_WR_TRXREGS(0xD);
    CAL_WR_TRXREGS(0xC);
    CAL_WR_TRXREGS(0xF);
    cal_delay(150);//for reg C,D,F write spi=80/16=5M,total time is 21.6us
    BK7011TRX.REG0xB->bits.dcorMod30 = 0;
    CAL_WR_TRXREGS(0xB);

    for(cnt = 0; cnt < GET_AV_TSSI_CNT; cnt++)
    {
        cal_delay(1 * gst_sar_adc);
        gav_tssi += BK7011RCBEKEN.REG0x54->bits.TSSIRD;
    }
    cnt = 0;
    gav_tssi /= GET_AV_TSSI_CNT;//Get small power tssi of each sample to remove tssi dc

    if(gtxoutput == 1) // first time for DC & IQ
        BK7011TRX.REG0xC->value = TRX_REG_0XC_VAL;//BK7011TRXREG0xC;
    else  // second for power calibration
    {
        BK7011TRX.REG0xC->value = bk7011_trx_val[0xC];
    }
    BK7011TRX.REG0xC->bits.dcorPA30 = 8;
    CAL_WR_TRXREGS(0xC);

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = gconst_pout;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = gconst_pout;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 1;
    if (gtxoutput == 1)
    {
        BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_IQ_VAL;
    }
    else
    {
        BK7011TRX.REG0xD->value = BK7011TRXREG0xD;
    }
    BK7011TRX.REG0xC->bits.TSSIsel = 1;
    BK7011TRX.REG0xC->bits.enDCcali = 0;
    BK7011TRX.REG0xC->bits.enDCcaliGm1 = 0;
    BK7011TRX.REG0xC->bits.enIQcali = 0;
    BK7011TRX.REG0xC->bits.enPcali = 1;
    BK7011TRX.REG0xC->bits.enPcaliGm = 0;
    BK7011TRX.REG0xF->bits.tssicalen = 1;
    CAL_WR_TRXREGS(0xD);
    CAL_WR_TRXREGS(0xC);
    CAL_WR_TRXREGS(0xF);
    cal_delay(150);//for reg C,D,F write spi=80/16=5M,total time is 21.6us

    low = 0;
    high = 15;

    BK7011TRX.REG0xB->bits.dcorMod30 = low;
    CAL_WR_TRXREGS(0xB);
    cal_delay(CAL_TX_NUM);//first sar dac delay needs double time
    tssilow = bk7011_get_tx_output_power();


    BK7011TRX.REG0xB->bits.dcorMod30 = high;
    CAL_WR_TRXREGS(0xB);
    tssihigh = bk7011_get_tx_output_power();

    do
    {
        CAL_PRT("cnt:%d, index:%d, tssilow:0x%x-%d, tssihigh:0x%x-%d\r\n",
                cnt++, index, tssilow, low, tssihigh, high);
        if(tssilow < tssihigh)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011TRX.REG0xB->bits.dcorMod30 = high;
            CAL_WR_TRXREGS(0xB);
            tssihigh = bk7011_get_tx_output_power();
        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011TRX.REG0xB->bits.dcorMod30 = low;
            CAL_WR_TRXREGS(0xB);
            tssilow = bk7011_get_tx_output_power();
        }
    }
    while((high - low) > 1);

    index = ((tssilow < tssihigh) ? low : high);
    if ((gtxoutput == 0) || (gtxoutput == 5))
    {
        gtx_dcorMod = index;
    }
#ifdef CALIBRATE_TIMES
	else
	{
	    if (p_gbias_after_cal_array == NULL)
	    	{
	    		CAL_PRT("bk7011 TX IQ Cal. Output Power: \r\ntx_dcorMod= %d, ", index);
	    	}
	}
#endif

    CAL_PRT("gtx_dcorMod over: 0x%x\r\n", gtx_dcorMod);
    CAL_PRT("cnt:%d, index:%d, tssilow:0x%x-%d, tssihigh:0x%x-%d\r\n",
            cnt++, index, tssilow, low, tssihigh, high);

    BK7011TRX.REG0xB->bits.dcorMod30 = index;
    CAL_WR_TRXREGS(0xB);
    gold_index = index << 8;
    cal_delay(6);

    low = 0;
    high = 15;
    BK7011TRX.REG0xC->bits.dcorPA30 = low;
    CAL_WR_TRXREGS(0xC);
    tssilow = bk7011_get_tx_output_power();

    BK7011TRX.REG0xC->bits.dcorPA30 = high;
    CAL_WR_TRXREGS(0xC);
    tssihigh = bk7011_get_tx_output_power();

    do
    {
        CAL_PRT("cnt:%d, index:%d, tssilow:0x%x-%d, tssihigh:0x%x-%d\r\n",
                cnt++, index, tssilow, low, tssihigh, high);
        if(tssilow < tssihigh)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011TRX.REG0xC->bits.dcorPA30 = high;
            CAL_WR_TRXREGS(0xC);
            tssihigh = bk7011_get_tx_output_power();
        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011TRX.REG0xC->bits.dcorPA30 = low;
            CAL_WR_TRXREGS(0xC);
            tssilow = bk7011_get_tx_output_power();
        }
    }
    while((high - low) > 1);

    index = ((tssilow < tssihigh) ? low : high);
    if ((gtxoutput == 0) || (gtxoutput == 5))
    {
        gtx_dcorPA = index;
    }
#ifdef CALIBRATE_TIMES
	else
	{
	    if (p_gbias_after_cal_array == NULL)
	    	{
	    		CAL_PRT("tx_dcorPA= %d\r\n", index);
	    	}
	}
#endif
    BK7011TRX.REG0xC->bits.dcorPA30 = index;
    CAL_WR_TRXREGS(0xC);
    gold_index += index;

    //method2:  first searching "dcorPA30",then serching "dcormod30";
    /****************************************************************
    *
    *****************************************************************/
    *val = gold_index;

    CAL_PRT("gtx_dcorMod:0x%x, gtx_dcorPA:0x%x\r\n", gtx_dcorMod, gtx_dcorPA);

#if 1
    tx_fre_gain = BK7011RCBEKEN.REG0x52->bits.TXPREGAIN;
    //CAL_WARN("tx_fre_gain:0x%x\r\n", tx_fre_gain);

    if(tx_fre_gain > 2)
        low = tx_fre_gain - 2;
    else
        low = 0;
    high = tx_fre_gain + 2;


    BK7011RCBEKEN.REG0x52->bits.TXPREGAIN = low;

    tssilow = bk7011_get_tx_output_power();

    BK7011RCBEKEN.REG0x52->bits.TXPREGAIN = high;
    tssihigh = bk7011_get_tx_output_power();

    do
    {
        if(tssilow < tssihigh)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x52->bits.TXPREGAIN = high;
            tssihigh = bk7011_get_tx_output_power();
        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x52->bits.TXPREGAIN = low;
            tssilow = bk7011_get_tx_output_power();
        }

    }
    while((high - low) > 1);

    index = ((tssilow < tssihigh) ? low : high);

    BK7011RCBEKEN.REG0x52->bits.TXPREGAIN = index;
    if ((gtxoutput == 0) || (gtxoutput == 5))
    {
        gtx_pre_gain = index;
    }
#ifdef CALIBRATE_TIMES
	else
	{
	    if (p_gbias_after_cal_array == NULL)
	    	{
	    		CAL_PRT("tx_pre_gain= %d\r\n", index);
	    	}
	}
#endif

    CAL_PRT("gtx_pre_gain:0x%x\r\n", gtx_pre_gain);
#endif

    return (gold_index);
}

static INT32 bk7011_set_tx_pa(INT32 val1, INT32 val2, INT32 val3, INT32 val4)
{
    BK7011TRX.REG0xC->bits.dgainPA20 = val1;
    BK7011TRX.REG0xC->bits.dgainbuf20 = val2;
    BK7011TRX.REG0xC->bits.gctrlpga20 = val3;
    BK7011TRX.REG0xB->bits.gctrlmod30 = val4;
    CAL_WR_TRXREGS(0xC);
    CAL_WR_TRXREGS(0xB);

    return 0;
}

static INT32 bk7011_update_tx_power(UINT8 nstep)
{
#ifdef CALIBRATE_TIMES
    bk7011_set_tx_pa(gi_dc_tx_pa_dgainPA20, gi_dc_tx_pa_dgainbuf20, 5, 8);
#else
    bk7011_set_tx_pa(2, 6, 5, 8);
#endif

    return 0;
}

static INT32 bk7011_update_tx_loopback_power(UINT8 nstep)
{
#ifdef CALIBRATE_TIMES
    bk7011_set_tx_pa(gi_dc_tx_loopback_pa_dgainPA20, gi_dc_tx_loopback_pa_dgainbuf20, 5, 8);
#else
    bk7011_set_tx_pa(5, 7, 5, 8);
#endif
    return 0;
}

static INT32 bk7011_get_tx_dc(void)
{
    INT32 detect_dc = 0;
    INT16 i;

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10  - 1;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 - 1 ;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 1;
    //BK7011TRX.REG0xD->value = BK7011TRXREG0xD;
    BK7011TRX.REG0xC->bits.TSSIsel = 0;
    BK7011TRX.REG0xC->bits.enDCcali = 1;
    BK7011TRX.REG0xC->bits.enDCcaliGm1 = 0;//inverse 20141014
    BK7011TRX.REG0xC->bits.enIQcali = 0;
    BK7011TRX.REG0xC->bits.enPcali = 0;
    BK7011TRX.REG0xC->bits.enPcaliGm = 0;
    BK7011TRX.REG0xF->bits.tssicalen = 1;

    //BK7011TRXONLY.REG0xD->value = BK7011TRX.REG0xD->value;
    CAL_WR_TRXREGS(0xC);
    CAL_WR_TRXREGS(0xF);

    cpu_delay(200);//18us

    for(i = 0; i < SUMNUMBERS; i++)
    {
        cal_delay(5 * gst_sar_adc);
        detect_dc += BK7011RCBEKEN.REG0x54->bits.TSSIRD;
    }

    return detect_dc;
}

static INT32 bk7011_get_tx_dc_1(void)
{
    INT32 detect_dc = 0;
    INT16 i;
    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 - 1 ;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10  - 1;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 1;
    //BK7011TRX.REG0xD->value = BK7011TRXREG0xD;
    BK7011TRX.REG0xC->bits.TSSIsel = 0;
    BK7011TRX.REG0xC->bits.enDCcali = 1;
    BK7011TRX.REG0xC->bits.enDCcaliGm1 = 1;
    BK7011TRX.REG0xC->bits.enIQcali = 1;//inverse 20141014
    BK7011TRX.REG0xC->bits.enPcali = 0;
    BK7011TRX.REG0xC->bits.enPcaliGm = 0;
    BK7011TRX.REG0xF->bits.tssicalen = 1;

    CAL_WR_TRXREGS(0xC);
    CAL_WR_TRXREGS(0xF);

    cpu_delay(200);
    for(i = 0; i < SUMNUMBERS; i++)
    {
        cal_delay(5 * gst_sar_adc);
        detect_dc += BK7011RCBEKEN.REG0x54->bits.TSSIRD;
    }
    return detect_dc;
}

INT32 bk7011_cal_tx_dc(INT32 *val)
{
    INT32 detect_dc_low = 0;
    INT32 detect_dc_high = 0;
    INT16 high, low;
    INT32 index, gold_index = 0;
    INT32 i_index, q_index;
    INT32 srchcnt = 0;
    /*step 4*/
    BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_IQ_VAL;  //BK7011TRXREG0xD;//
    CAL_WR_TRXREGS(0xD);

    pwr_nstep = 10;
    bk7011_update_tx_power(pwr_nstep);
#ifdef CALIBRATE_TIMES
	if (p_gbias_after_cal_array == NULL)
		{
	CAL_PRT("\r\nbk7011_cal_tx_dc: \r\n");
	CAL_PRT("dgainPA20  = %d\r\n", BK7011TRX.REG0xC->bits.dgainPA20 );
	CAL_PRT("dgainbuf20 =%d\r\n",  BK7011TRX.REG0xC->bits.dgainbuf20);
	CAL_PRT("gctrlpga20  = %d\r\n", BK7011TRX.REG0xC->bits.gctrlpga20 );
	CAL_PRT("gctrlmod30 = %d\r\n", BK7011TRX.REG0xB->bits.gctrlmod30);
		}
#endif
	
    //[-512 511]---->[0,1023];

    // I DC calibration;
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = UNSIGNEDOFFSET10 + 0; //default
    low = UNSIGNEDOFFSET10 - MINOFFSET ;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    high = UNSIGNEDOFFSET10 + MINOFFSET ;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    //Step 1 3~6 search;
    srchcnt = 0;
    pwr_nstep = 0;
    bk7011_update_tx_power(pwr_nstep);
    //low = 0;
    //high = 1023;
    while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
    {
        if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
        {
            //PA--
            if(pwr_nstep > 0)
                pwr_nstep--;
            else
                break;
            bk7011_update_tx_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
        {
            //PA++
            if(pwr_nstep < 0x0f)
                pwr_nstep++;
            else
                break;
            bk7011_update_tx_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        else
        {
            break;
        }
    }

    if(detect_dc_low < detect_dc_high)
    {
        low = 0;
        high = 511;
    }
    else
    {
        low = 512;
        high = 1023;
    }
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    do
    {
        while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
        {
            if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
            {
                //PA--
                if(pwr_nstep > 0)
                    pwr_nstep--;
                else
                    break;
                bk7011_update_tx_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc();
            }
            else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
            {
                //PA++
                if(pwr_nstep < 0x0f)
                    pwr_nstep++;
                else
                    break;
                bk7011_update_tx_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc();
            }
            else
            {
                break;
            }
        }

        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();

        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        srchcnt++;
        if(srchcnt > gtx_dc_n)
            break;
    }
    while((high - low) > 1);

    //Step 2  search;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc_1();
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc_1();

    do
    {
        while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
        {

            if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
            {
                //PA--
                if(pwr_nstep > 0x0)
                    pwr_nstep--;
                else
                    break;
                bk7011_update_tx_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc_1();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc_1();

            }
            else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
            {
                //PA++
                if(pwr_nstep < 0x0f)
                    pwr_nstep++;
                else
                    break;
                bk7011_update_tx_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc_1();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc_1();
            }
            else
            {
                break;
            }
        }

        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();

        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
    }
    while((high - low) > 1);

    while(abs(detect_dc_low - detect_dc_high) < (TSSI_DELTA ))
    {
        if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
        {
            //PA--
            if(pwr_nstep > 0x0)
                pwr_nstep--;
            else
                break;
            bk7011_update_tx_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();

        }
        else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
        {
            //PA++
            if(pwr_nstep < 0x0f)
                pwr_nstep++;
            else
                break;
            bk7011_update_tx_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
        else
        {
            break;
        }
    }
    i_index = ((detect_dc_low < detect_dc_high) ? low : high);

    // Q DC calibration;
    //Step 1 3~6 search;
    pwr_nstep = 10;
    bk7011_update_tx_power(pwr_nstep);
    srchcnt = 0;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = i_index; //default
    low = UNSIGNEDOFFSET10 - MINOFFSET ;;
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    high = UNSIGNEDOFFSET10 + MINOFFSET ;;
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    //low = 0;
    //high = 1023;
    pwr_nstep = 0;
    bk7011_update_tx_power(pwr_nstep);

    while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
    {
        if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
        {
            //PA--
            if(pwr_nstep > 0)
                pwr_nstep--;
            else
                break;
            bk7011_update_tx_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
        {
            //PA++
            if(pwr_nstep < 0x0f)
                pwr_nstep++;
            else
                break;
            bk7011_update_tx_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        else
        {
            break;
        }
    }

    if(detect_dc_low < detect_dc_high)
    {
        low = 0;
        high = 511;
    }
    else
    {
        low = 512;
        high = 1023;
    }
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    do
    {
        while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
        {

            if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
            {
                //PA--
                if(pwr_nstep > 0)
                    pwr_nstep--;
                else
                    break;
                bk7011_update_tx_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc();
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc();
            }
            else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
            {
                //PA++
                if(pwr_nstep < 0x0f)
                    pwr_nstep++;
                else
                    break;
                bk7011_update_tx_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc();
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc();
            }
            else
            {
                break;
            }
        }
        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();

        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        srchcnt++;
        if(srchcnt > gtx_dc_n)
            break;
    }
    while((high - low) > 1);

    //Step 2  search;
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc_1();
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc_1();
    do
    {
        while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
        {

            if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
            {
                //PA--
                if(pwr_nstep > 0)
                    pwr_nstep--;
                else
                    break;
                bk7011_update_tx_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc_1();
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc_1();
            }
            else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
            {
                //PA++
                if(pwr_nstep < 0x0f)
                    pwr_nstep++;
                else
                    break;
                bk7011_update_tx_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc_1();
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc_1();
            }
            else
            {
                break;
            }
        }


        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();

        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
    }
    while((high - low) > 1);

    while(abs(detect_dc_low - detect_dc_high) < (TSSI_DELTA))
    {

        if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
        {
            //PA--
            if(pwr_nstep > 0)
                pwr_nstep--;
            else
                break;
            bk7011_update_tx_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
        else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
        {
            //PA++
            if(pwr_nstep < 0x0f)
                pwr_nstep++;
            else
                break;
            bk7011_update_tx_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
        else
        {
            break;
        }
    }

    q_index = ((detect_dc_low < detect_dc_high) ? low : high);
    gtx_q_dc_comp = q_index;
    CAL_WARN("gtx_q_dc_comp:0x%x\r\n", gtx_q_dc_comp);
    gold_index += q_index;

    // 2nd  I DC calibration;
    //Step 1 3~6 search;
    pwr_nstep = 10;
    bk7011_update_tx_power(pwr_nstep);
    srchcnt = 0;
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = q_index; // optimum
    low = UNSIGNEDOFFSET10 - MINOFFSET ;;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    high = UNSIGNEDOFFSET10 + MINOFFSET ;;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    //low = 0;
    //high = 1023;
    pwr_nstep = 0;
    bk7011_update_tx_power(pwr_nstep);

    while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
    {
        if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
        {
            //PA--
            if(pwr_nstep > 0)
                pwr_nstep--;
            else
                break;
            bk7011_update_tx_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
        {
            //PA++
            if(pwr_nstep < 0x0f)
                pwr_nstep++;
            else
                break;
            bk7011_update_tx_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        else
        {
            break;
        }
    }

    if(detect_dc_low < detect_dc_high)
    {
        low = 0;
        high = 511;
    }
    else
    {
        low = 512;
        high = 1023;
    }
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    do
    {
        while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
        {
            if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
            {
                //PA--
                if(pwr_nstep > 0)
                    pwr_nstep--;
                else
                    break;
                bk7011_update_tx_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc();
            }
            else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
            {
                //PA++
                if(pwr_nstep < 0x0f)
                    pwr_nstep++;
                else
                    break;
                bk7011_update_tx_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc();
            }
            else
            {
                break;
            }
        }

        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();
        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        srchcnt++;
        if(srchcnt > gtx_dc_n) break;
    }
    while((high - low) > 1);

    //Step 2  search;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc_1();
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc_1();
    do
    {
        while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
        {

            if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
            {
                //PA--
                if(pwr_nstep > 0x0)
                    pwr_nstep--;
                else
                    break;
                bk7011_update_tx_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc_1();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc_1();
            }
            else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
            {
                //PA++
                if(pwr_nstep < 0x0f)
                    pwr_nstep++;
                else
                    break;
                bk7011_update_tx_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc_1();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc_1();
            }
            else
            {
                break;
            }
        }
        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();

        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
    }
    while((high - low) > 1);
    while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
    {

        if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
        {
            //PA--
            if(pwr_nstep > 0x0)
                pwr_nstep--;
            else
                break;
            bk7011_update_tx_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
        else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
        {
            //PA++
            if(pwr_nstep < 0x0f)
                pwr_nstep++;
            else
                break;
            bk7011_update_tx_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
        else
        {
            break;
        }
    }
    i_index = ((detect_dc_low < detect_dc_high) ? low : high);
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = i_index;
    gtx_i_dc_comp = i_index;
    CAL_WARN("gtx_i_dc_comp:0x%x\r\n", gtx_i_dc_comp);
    gold_index += (i_index << 16);
    *val = gold_index;
    (void)index;
    return gold_index;
}


INT32 bk7011_cal_tx_loopback_dc(INT32 *val)
{
    INT32 detect_dc_low = 0;
    INT32 detect_dc_high = 0;
    INT16 high, low;
    INT32 index, gold_index = 0;
    INT32 i_index, q_index;
    INT32 srchcnt = 0;
    /*step 4*/
    BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_LOOPBACK_IQ_VAL;
    CAL_WR_TRXREGS(0xD);

    pwr_nstep = 10;
    bk7011_update_tx_loopback_power(pwr_nstep);
#ifdef CALIBRATE_TIMES
	if (p_gbias_after_cal_array == NULL)
		{
	CAL_PRT("\r\nbk7011_cal_tx_loopback_dc: \r\n");
	CAL_PRT("dgainPA20  = %d\r\n", BK7011TRX.REG0xC->bits.dgainPA20 );
	CAL_PRT("dgainbuf20 =%d\r\n",  BK7011TRX.REG0xC->bits.dgainbuf20);
	CAL_PRT("gctrlpga20  = %d\r\n", BK7011TRX.REG0xC->bits.gctrlpga20 );
	CAL_PRT("gctrlmod30 = %d\r\n", BK7011TRX.REG0xB->bits.gctrlmod30);
		}
#endif
    //[-512 511]---->[0,1023];

    // I DC calibration;
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = UNSIGNEDOFFSET10 + 0; //default
    low = UNSIGNEDOFFSET10 - 3 * MINOFFSET ;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    high = UNSIGNEDOFFSET10 + 3 * MINOFFSET ;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    //Step 1 3~6 search;
    srchcnt = 0;
    pwr_nstep = 0;
    bk7011_update_tx_loopback_power(pwr_nstep);
    //low = 0;
    //high = 1023;
    while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
    {
        if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
        {
            //PA--
            if(pwr_nstep > 0)
                pwr_nstep--;
            else
                break;
            bk7011_update_tx_loopback_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
        {
            //PA++
            if(pwr_nstep < 0x0f)
                pwr_nstep++;
            else
                break;
            bk7011_update_tx_loopback_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        else
        {
            break;
        }
    }

    if(detect_dc_low < detect_dc_high)
    {
        low = 0;
        high = 511;
    }
    else
    {
        low = 512;
        high = 1023;
    }
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    do
    {
        while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
        {
            if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
            {
                //PA--
                if(pwr_nstep > 0)
                    pwr_nstep--;
                else
                    break;
                bk7011_update_tx_loopback_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc();
            }
            else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
            {
                //PA++
                if(pwr_nstep < 0x0f)
                    pwr_nstep++;
                else
                    break;
                bk7011_update_tx_loopback_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc();
            }
            else
            {
                break;
            }
        }

        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();

        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        srchcnt++;
        if(srchcnt > gtx_dc_n)
            break;
    }
    while((high - low) > 1);

    //Step 2  search;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc_1();
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc_1();

    do
    {
        while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
        {

            if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
            {
                //PA--
                if(pwr_nstep > 0x0)
                    pwr_nstep--;
                else
                    break;
                bk7011_update_tx_loopback_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc_1();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc_1();

            }
            else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
            {
                //PA++
                if(pwr_nstep < 0x0f)
                    pwr_nstep++;
                else
                    break;
                bk7011_update_tx_loopback_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc_1();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc_1();
            }
            else
            {
                break;
            }
        }

        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();

        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
    }
    while((high - low) > 1);

    while(abs(detect_dc_low - detect_dc_high) < (TSSI_DELTA ))
    {
        if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
        {
            //PA--
            if(pwr_nstep > 0x0)
                pwr_nstep--;
            else
                break;
            bk7011_update_tx_loopback_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();

        }
        else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
        {
            //PA++
            if(pwr_nstep < 0x0f)
                pwr_nstep++;
            else
                break;
            bk7011_update_tx_loopback_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
        else
        {
            break;
        }
    }
    i_index = ((detect_dc_low < detect_dc_high) ? low : high);

    // Q DC calibration;
    //Step 1 3~6 search;
    pwr_nstep = 10;
    bk7011_update_tx_loopback_power(pwr_nstep);
    srchcnt = 0;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = i_index; //default
    low = UNSIGNEDOFFSET10 - 3 * MINOFFSET ;;
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    high = UNSIGNEDOFFSET10 + 3 * MINOFFSET ;;
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    //low = 0;
    //high = 1023;
    pwr_nstep = 0;
    bk7011_update_tx_loopback_power(pwr_nstep);

    while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
    {
        if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
        {
            //PA--
            if(pwr_nstep > 0)
                pwr_nstep--;
            else
                break;
            bk7011_update_tx_loopback_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
        {
            //PA++
            if(pwr_nstep < 0x0f)
                pwr_nstep++;
            else
                break;
            bk7011_update_tx_loopback_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        else
        {
            break;
        }
    }

    if(detect_dc_low < detect_dc_high)
    {
        low = 0;
        high = 511;
    }
    else
    {
        low = 512;
        high = 1023;
    }
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    do
    {
        while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
        {

            if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
            {
                //PA--
                if(pwr_nstep > 0)
                    pwr_nstep--;
                else
                    break;
                bk7011_update_tx_loopback_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc();
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc();
            }
            else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
            {
                //PA++
                if(pwr_nstep < 0x0f)
                    pwr_nstep++;
                else
                    break;
                bk7011_update_tx_loopback_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc();
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc();
            }
            else
            {
                break;
            }
        }
        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();

        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        srchcnt++;
        if(srchcnt > gtx_dc_n)
            break;
    }
    while((high - low) > 1);

    //Step 2  search;
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc_1();
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc_1();
    do
    {
        while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
        {

            if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
            {
                //PA--
                if(pwr_nstep > 0)
                    pwr_nstep--;
                else
                    break;
                bk7011_update_tx_loopback_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc_1();
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc_1();
            }
            else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
            {
                //PA++
                if(pwr_nstep < 0x0f)
                    pwr_nstep++;
                else
                    break;
                bk7011_update_tx_loopback_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc_1();
                BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc_1();
            }
            else
            {
                break;
            }
        }


        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();

        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
    }
    while((high - low) > 1);

    while(abs(detect_dc_low - detect_dc_high) < (TSSI_DELTA))
    {

        if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
        {
            //PA--
            if(pwr_nstep > 0)
                pwr_nstep--;
            else
                break;
            bk7011_update_tx_loopback_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
        else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
        {
            //PA++
            if(pwr_nstep < 0x0f)
                pwr_nstep++;
            else
                break;
            bk7011_update_tx_loopback_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
        else
        {
            break;
        }
    }

    q_index = ((detect_dc_low < detect_dc_high) ? low : high);
    //gtx_q_dc_comp = q_index;
#ifdef CALIBRATE_TIMES
    if (p_gtx_q_dc_comp_temp_array != NULL)
    {
	p_gtx_q_dc_comp_temp_array[calibrate_time] = q_index;
    }
#endif
    gold_index += q_index;

    // 2nd  I DC calibration;
    //Step 1 3~6 search;
    pwr_nstep = 10;
    bk7011_update_tx_loopback_power(pwr_nstep);
    srchcnt = 0;
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = q_index; // optimum
    low = UNSIGNEDOFFSET10 - 3 * MINOFFSET ;;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    high = UNSIGNEDOFFSET10 + 3 * MINOFFSET ;;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    //low = 0;
    //high = 1023;
    pwr_nstep = 0;
    bk7011_update_tx_loopback_power(pwr_nstep);

    while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
    {
        if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
        {
            //PA--
            if(pwr_nstep > 0)
                pwr_nstep--;
            else
                break;
            bk7011_update_tx_loopback_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
        {
            //PA++
            if(pwr_nstep < 0x0f)
                pwr_nstep++;
            else
                break;
            bk7011_update_tx_loopback_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        else
        {
            break;
        }
    }

    if(detect_dc_low < detect_dc_high)
    {
        low = 0;
        high = 511;
    }
    else
    {
        low = 512;
        high = 1023;
    }
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    do
    {
        while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
        {
            if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
            {
                //PA--
                if(pwr_nstep > 0)
                    pwr_nstep--;
                else
                    break;
                bk7011_update_tx_loopback_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc();
            }
            else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
            {
                //PA++
                if(pwr_nstep < 0x0f)
                    pwr_nstep++;
                else
                    break;
                bk7011_update_tx_loopback_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc();
            }
            else
            {
                break;
            }
        }

        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();
        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        srchcnt++;
        if(srchcnt > gtx_dc_n) break;
    }
    while((high - low) > 1);

    //Step 2  search;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc_1();
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc_1();
    do
    {
        while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
        {

            if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
            {
                //PA--
                if(pwr_nstep > 0x0)
                    pwr_nstep--;
                else
                    break;
                bk7011_update_tx_loopback_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc_1();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc_1();
            }
            else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
            {
                //PA++
                if(pwr_nstep < 0x0f)
                    pwr_nstep++;
                else
                    break;
                bk7011_update_tx_loopback_power(pwr_nstep);
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
                detect_dc_high = bk7011_get_tx_dc_1();
                BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
                detect_dc_low = bk7011_get_tx_dc_1();
            }
            else
            {
                break;
            }
        }
        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();

        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
    }
    while((high - low) > 1);
    while(abs(detect_dc_low - detect_dc_high) < TSSI_DELTA)
    {

        if(TSSI_IS_TOO_HIGH(detect_dc_low) || TSSI_IS_TOO_HIGH(detect_dc_high))
        {
            //PA--
            if(pwr_nstep > 0x0)
                pwr_nstep--;
            else
                break;
            bk7011_update_tx_loopback_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
        else if(TSSI_IS_TOO_LOW(detect_dc_low) || TSSI_IS_TOO_LOW(detect_dc_high))
        {
            //PA++
            if(pwr_nstep < 0x0f)
                pwr_nstep++;
            else
                break;
            bk7011_update_tx_loopback_power(pwr_nstep);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc_1();
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc_1();
        }
        else
        {
            break;
        }
    }
    i_index = ((detect_dc_low < detect_dc_high) ? low : high);
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = i_index;
    //gtx_i_dc_comp = i_index;
#ifdef CALIBRATE_TIMES
    if (p_gtx_i_dc_comp_temp_array != NULL)
    {
	p_gtx_i_dc_comp_temp_array[calibrate_time] = i_index;
    }
#endif
    gold_index += (i_index << 16);
    *val = gold_index;
    (void)index;
    return gold_index;
}


static INT32 bk7011_get_tx_i_gain(void)
{
    INT32 detector_i_gain_p, detector_i_gain_n, detector_i_gain;

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 + gconst_iqcal_p;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 + 0;
    cal_delay(4 * gst_sar_adc);
    detector_i_gain_p = BK7011RCBEKEN.REG0x54->bits.TSSIRD;

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 - gconst_iqcal_p;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 + 0;
    cal_delay(4 * gst_sar_adc);
    detector_i_gain_n = BK7011RCBEKEN.REG0x54->bits.TSSIRD;

    detector_i_gain = detector_i_gain_p + detector_i_gain_n;
    return detector_i_gain;
}
static INT32 bk7011_get_tx_q_gain(void)
{
    INT32 detector_q_gain_p, detector_q_gain_n, detector_q_gain;

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 + 0;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 + gconst_iqcal_p;
    cal_delay(4 * gst_sar_adc);
    detector_q_gain_p = BK7011RCBEKEN.REG0x54->bits.TSSIRD;

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 + 0;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 - gconst_iqcal_p;
    cal_delay(4 * gst_sar_adc);
    detector_q_gain_n = BK7011RCBEKEN.REG0x54->bits.TSSIRD;

    detector_q_gain = detector_q_gain_p + detector_q_gain_n;
    return detector_q_gain;
}
static INT32 bk7011_get_tx_i_phase(void)
{
    INT32 detector_i_phase_n, detector_i_phase_p, detector_i_phase;


    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 + gconst_iqcal_p;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 - gconst_iqcal_p;
    cal_delay(4 * gst_sar_adc);
    detector_i_phase_p = BK7011RCBEKEN.REG0x54->bits.TSSIRD;

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 - gconst_iqcal_p;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 + gconst_iqcal_p;
    cal_delay(4 * gst_sar_adc);
    detector_i_phase_n = BK7011RCBEKEN.REG0x54->bits.TSSIRD;
    detector_i_phase = detector_i_phase_p + detector_i_phase_n;
    return detector_i_phase;
}
static INT32 bk7011_get_tx_q_phase(void)
{
    INT32 detector_q_phase_n, detector_q_phase_p, detector_q_phase;

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 + gconst_iqcal_p;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 + gconst_iqcal_p;
    cal_delay(4 * gst_sar_adc);
    detector_q_phase_p = BK7011RCBEKEN.REG0x54->bits.TSSIRD;

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 - gconst_iqcal_p;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 - gconst_iqcal_p;
    cal_delay(4 * gst_sar_adc);
    detector_q_phase_n = BK7011RCBEKEN.REG0x54->bits.TSSIRD;
    detector_q_phase = detector_q_phase_p + detector_q_phase_n;
    return detector_q_phase;
}
static INT32 bk7011_get_rx_i_avg_signed(void)
{
    INT32 val;
#ifdef BK7011_VER_A
    val = BK7011RCBEKEN.REG0x3C->bits.RXAVGIRD;
#else
    val = BK7011RCBEKEN.REG0x3C->bits.RXAVGIRD;
#endif

    if(val & 0x00000800)
    {
        val |= 0xfffff000;
    }

    return abs(val);
}

static INT32 bk7011_get_rx_q_avg_signed(void)
{
    INT32 val;

#ifdef BK7011_VER_A
    val = BK7011RCBEKEN.REG0x3C->bits.RXAVGQRD;
#else
    val = BK7011RCBEKEN.REG0x3C->bits.RXAVGQRD;
#endif

    if(val & 0x00000800)
    {
        val |= 0xfffff000;
    }
    return abs(val);
}

INT32 bk7011_cal_tx_gain_imbalance(INT32 *val)
{
    INT32 detect_gain_low = 0;
    INT32 detect_gain_high = 0;
    INT16 high, low;
    INT32 index = 0, gold_index = 0;
    INT32 detector_i_gain;
    INT32 detector_q_gain;

    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = gtx_i_dc_comp;
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = gtx_q_dc_comp;
    BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = 1023;
    BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = 1023;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 1;
    BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_IQ_VAL;//BK7011TRXREG0xD;//0214 close the TX switch to ellimite the antenna infect
    CAL_WR_TRXREGS(0xD);

#ifdef CHIPLOOPBACK
    bk7011_set_tx_pa(5, 2, 3, 4);
#else
#if BK7211_68PIN_BOARD
    bk7011_set_tx_pa(6, 4, 4, 4);
#else
#ifdef CALIBRATE_TIMES
    bk7011_set_tx_pa(gi_gain_tx_pa_dgainPA20, gi_gain_tx_pa_dgainbuf20, 5, 8);
	if (p_gbias_after_cal_array == NULL)
		{
	CAL_PRT("\r\nbk7011_cal_tx_gain_imbalance\r\n");
	CAL_PRT("dgainPA20  = %d\r\n", BK7011TRX.REG0xC->bits.dgainPA20 );
	CAL_PRT("dgainbuf20 =%d\r\n",  BK7011TRX.REG0xC->bits.dgainbuf20);
	CAL_PRT("gctrlpga20  = %d\r\n", BK7011TRX.REG0xC->bits.gctrlpga20 );
	CAL_PRT("gctrlmod30 = %d\r\n", BK7011TRX.REG0xB->bits.gctrlmod30);
		}
#else
    bk7011_set_tx_pa(2, 4, 5, 8);
#endif
#endif
#endif

    BK7011TRX.REG0xC->bits.TSSIsel = 0;
    BK7011TRX.REG0xC->bits.enDCcali = 1;
    BK7011TRX.REG0xC->bits.enDCcaliGm1 = 0;
    BK7011TRX.REG0xC->bits.enIQcali = 1;
    BK7011TRX.REG0xC->bits.enPcali = 0;
    BK7011TRX.REG0xC->bits.enPcaliGm = 0;
    BK7011TRX.REG0xF->bits.tssicalen = 1;
    CAL_WR_TRXREGS(0xC);
    CAL_WR_TRXREGS(0xB);
    CAL_WR_TRXREGS(0xF);

    gtx_i_gain_comp = BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP;
    gtx_q_gain_comp = BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP;
    cpu_delay(500);
    detector_i_gain = bk7011_get_tx_i_gain();
    detector_q_gain = bk7011_get_tx_q_gain();

    if(abs(detector_q_gain - detector_i_gain) < 3)
    {
        *val = 0;
        gtx_i_gain_comp = 0x03ff;
        gtx_q_gain_comp = 0x03ff;
        CAL_WARN("gtx_i_gain_comp:%d\r\n", gtx_i_gain_comp);
        CAL_WARN("gtx_q_gain_comp:%d\r\n", gtx_q_gain_comp);
        return (BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP + (BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP  << 16));
    }

    if(detector_i_gain > detector_q_gain) // TX_Q_GAIN_COMP NOT CHANGED
    {
        BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = 1023;
        low = 0;
        BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = low;
        detector_i_gain = bk7011_get_tx_i_gain();
        detector_q_gain = bk7011_get_tx_q_gain();

        detect_gain_low = abs(detector_i_gain - detector_q_gain);

        high = 1023;
        BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = high;
        detector_i_gain = bk7011_get_tx_i_gain();
        detector_q_gain = bk7011_get_tx_q_gain();

        detect_gain_high = abs(detector_i_gain - detector_q_gain);
        do
        {
            if(detect_gain_low < detect_gain_high)
            {
                index = low;
                high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
                BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = high;
                detector_i_gain = bk7011_get_tx_i_gain();
                detector_q_gain = bk7011_get_tx_q_gain();
                detect_gain_high = abs(detector_i_gain - detector_q_gain);
            }
            else
            {
                index = high;
                low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
                BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = low;
                detector_i_gain = bk7011_get_tx_i_gain();
                detector_q_gain = bk7011_get_tx_q_gain();
                detect_gain_low = abs(detector_i_gain - detector_q_gain);
            }

        }
        while((high - low) > 1);
        index = ((detect_gain_low < detect_gain_high) ? low : high);
        BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = index;
        gtx_i_gain_comp = index;
        gold_index = (index << 16) + 1023;
    }
    else  //// TX_I_GAIN_COMP NOT CHANGED
    {
        BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = 1023;
        low = 0;
        BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = low;
        detector_i_gain = bk7011_get_tx_i_gain();
        detector_q_gain = bk7011_get_tx_q_gain();

        detect_gain_low = abs(detector_i_gain - detector_q_gain);

        high = 1023;
        BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = high;
        detector_i_gain = bk7011_get_tx_i_gain();
        detector_q_gain = bk7011_get_tx_q_gain();

        detect_gain_high = abs(detector_i_gain - detector_q_gain);
        do
        {
            if(detect_gain_low < detect_gain_high)
            {
                index = low;
                high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
                BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = high;
                detector_i_gain = bk7011_get_tx_i_gain();
                detector_q_gain = bk7011_get_tx_q_gain();
                detect_gain_high = abs(detector_i_gain - detector_q_gain);
            }
            else
            {
                index = high;
                low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
                BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = low;
                detector_i_gain = bk7011_get_tx_i_gain();
                detector_q_gain = bk7011_get_tx_q_gain();
                detect_gain_low = abs(detector_i_gain - detector_q_gain);
            }

        }
        while((high - low) > 1);
        index = ((detect_gain_low < detect_gain_high) ? low : high);
        BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = index;
        gtx_q_gain_comp = index;
        gold_index += (1023 << 16) + index;
    }

    *val = gold_index;
    CAL_WARN("gtx_i_gain_comp:%d\r\n", gtx_i_gain_comp);
    CAL_WARN("gtx_q_gain_comp:%d\r\n", gtx_q_gain_comp);

    return gold_index;
}

INT32 bk7011_cal_tx_loopback_gain_imbalance(INT32 *val)
{
    INT32 detect_gain_low = 0;
    INT32 detect_gain_high = 0;
    INT16 high, low;
    INT32 index = 0, gold_index = 0;
    INT32 detector_i_gain;
    INT32 detector_q_gain;

    INT16 tx_i_gain_comp;
    INT16 tx_q_gain_comp;

    BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = 1023;
    BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = 1023;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 1;
    BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_LOOPBACK_IQ_VAL;
    CAL_WR_TRXREGS(0xD);

#ifdef CHIPLOOPBACK
    bk7011_set_tx_pa(5, 2, 3, 4);
#else
#ifdef CALIBRATE_TIMES
    bk7011_set_tx_pa(gi_gain_tx_loopback_pa_dgainPA20, gi_gain_tx_loopback_pa_dgainbuf20, 5, 8);
	if (p_gbias_after_cal_array == NULL)
		{
	CAL_PRT("\r\nbk7011_cal_tx_loopback_gain_imbalance\r\n");
	CAL_PRT("dgainPA20  = %d\r\n", BK7011TRX.REG0xC->bits.dgainPA20 );
	CAL_PRT("dgainbuf20 =%d\r\n",  BK7011TRX.REG0xC->bits.dgainbuf20);
	CAL_PRT("gctrlpga20  = %d\r\n", BK7011TRX.REG0xC->bits.gctrlpga20 );
	CAL_PRT("gctrlmod30 = %d\r\n", BK7011TRX.REG0xB->bits.gctrlmod30);
		}
#else
    bk7011_set_tx_pa(6, 7, 5, 8);
#endif
#endif

    BK7011TRX.REG0xC->bits.TSSIsel = 0;
    BK7011TRX.REG0xC->bits.enDCcali = 1;
    BK7011TRX.REG0xC->bits.enDCcaliGm1 = 0;
    BK7011TRX.REG0xC->bits.enIQcali = 1;
    BK7011TRX.REG0xC->bits.enPcali = 0;
    BK7011TRX.REG0xC->bits.enPcaliGm = 0;
    BK7011TRX.REG0xF->bits.tssicalen = 1;
    CAL_WR_TRXREGS(0xC);
    CAL_WR_TRXREGS(0xB);
    CAL_WR_TRXREGS(0xF);
    BK7011TRXONLY.REG0xF->value = BK7011TRX.REG0xF->value;

    tx_i_gain_comp = BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP;
    tx_q_gain_comp = BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP;
    tx_i_gain_comp = tx_i_gain_comp;
    tx_q_gain_comp = tx_q_gain_comp;
    cpu_delay(500);
    detector_i_gain = bk7011_get_tx_i_gain();
    detector_q_gain = bk7011_get_tx_q_gain();

    if(abs(detector_q_gain - detector_i_gain) < 3)
    {
        *val = 0;
        tx_i_gain_comp = 0x03ff;
        tx_q_gain_comp = 0x03ff;
        return (BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP + (BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP  << 16));
    }

    if(detector_i_gain > detector_q_gain) // TX_Q_GAIN_COMP NOT CHANGED
    {
        BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = 1023;
        low = 0;
        BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = low;
        detector_i_gain = bk7011_get_tx_i_gain();
        detector_q_gain = bk7011_get_tx_q_gain();

        detect_gain_low = abs(detector_i_gain - detector_q_gain);

        high = 1023;
        BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = high;
        detector_i_gain = bk7011_get_tx_i_gain();
        detector_q_gain = bk7011_get_tx_q_gain();

        detect_gain_high = abs(detector_i_gain - detector_q_gain);
        do
        {
            if(detect_gain_low < detect_gain_high)
            {
                index = low;
                high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
                BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = high;
                detector_i_gain = bk7011_get_tx_i_gain();
                detector_q_gain = bk7011_get_tx_q_gain();
                detect_gain_high = abs(detector_i_gain - detector_q_gain);
            }
            else
            {
                index = high;
                low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
                BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = low;
                detector_i_gain = bk7011_get_tx_i_gain();
                detector_q_gain = bk7011_get_tx_q_gain();
                detect_gain_low = abs(detector_i_gain - detector_q_gain);
            }

        }
        while((high - low) > 1);
        index = ((detect_gain_low < detect_gain_high) ? low : high);
        BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = index;
        tx_i_gain_comp = index;
        gold_index = (index << 16) + 1023;
    }
    else  //// TX_I_GAIN_COMP NOT CHANGED
    {
        BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = 1023;
        low = 0;
        BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = low;
        detector_i_gain = bk7011_get_tx_i_gain();
        detector_q_gain = bk7011_get_tx_q_gain();

        detect_gain_low = abs(detector_i_gain - detector_q_gain);

        high = 1023;
        BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = high;
        detector_i_gain = bk7011_get_tx_i_gain();
        detector_q_gain = bk7011_get_tx_q_gain();

        detect_gain_high = abs(detector_i_gain - detector_q_gain);
        do
        {
            if(detect_gain_low < detect_gain_high)
            {
                index = low;
                high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
                BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = high;
                detector_i_gain = bk7011_get_tx_i_gain();
                detector_q_gain = bk7011_get_tx_q_gain();
                detect_gain_high = abs(detector_i_gain - detector_q_gain);
            }
            else
            {
                index = high;
                low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
                BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = low;
                detector_i_gain = bk7011_get_tx_i_gain();
                detector_q_gain = bk7011_get_tx_q_gain();
                detect_gain_low = abs(detector_i_gain - detector_q_gain);
            }

        }
        while((high - low) > 1);
        index = ((detect_gain_low < detect_gain_high) ? low : high);
        BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = index;
        tx_q_gain_comp = index;
        gold_index += (1023 << 16) + index;
    }

    *val = gold_index;

    return gold_index;
}


static INT32 bk7011_cal_tx_ty2(INT32 tx_phase_comp)
{
    float ty1, ty1_sqr, ty2;
    INT32 tx_ty2;

    ty1 = -1.0 * ((tx_phase_comp - 512) * (tx_phase_comp - 512)) / (1024.0 * 1024.0);
    ty1_sqr = ty1 * ty1;
    ty2 = 1 - ty1 / 2 + 3 * ty1_sqr / 8;
    tx_ty2 = (INT32)((ty2 - 0.5) * 1024 + 0.5);

    return tx_ty2;
}

INT32 bk7011_cal_tx_phase_imbalance(INT32 *val)
{
    INT32 detect_phase_low = 0;
    INT32 detect_phase_high = 0;
    INT16 high, low;
    INT32 index = 0, gold_index = 0;
    INT32 detector_i_phase;
    INT32 detector_q_phase;

    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = gtx_i_dc_comp;
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = gtx_q_dc_comp;
    BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = gtx_i_gain_comp;
    BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = gtx_q_gain_comp;
    BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP = 512;
    BK7011RCBEKEN.REG0x51->bits.TXTY2 = 512;

    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 1;
    BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_IQ_VAL;
    CAL_WR_TRXREGS(0xD);

#ifdef CHIPLOOPBACK
    bk7011_set_tx_pa(5, 3, 4, 4);
#else
#if BK7211_68PIN_BOARD
    bk7011_set_tx_pa(5, 3, 4, 8);
#else
#ifdef CALIBRATE_TIMES
    bk7011_set_tx_pa(gi_gain_tx_pa_dgainPA20, gi_gain_tx_pa_dgainbuf20, 5, 8);
	if (p_gbias_after_cal_array == NULL)
		{
	CAL_PRT("\r\nbk7011_cal_tx_phase_imbalance: \r\n");
	CAL_PRT("dgainPA20  = %d\r\n", BK7011TRX.REG0xC->bits.dgainPA20 );
	CAL_PRT("dgainbuf20 =%d\r\n",  BK7011TRX.REG0xC->bits.dgainbuf20);
	CAL_PRT("gctrlpga20  = %d\r\n", BK7011TRX.REG0xC->bits.gctrlpga20 );
	CAL_PRT("gctrlmod30 = %d\r\n", BK7011TRX.REG0xB->bits.gctrlmod30);
		}
#else
    bk7011_set_tx_pa(2, 4, 5, 8);
#endif
#endif
#endif
    BK7011TRX.REG0xC->bits.TSSIsel = 0;
    BK7011TRX.REG0xC->bits.enDCcali = 1;
    BK7011TRX.REG0xC->bits.enDCcaliGm1 = 0;
    BK7011TRX.REG0xC->bits.enIQcali = 1;
    BK7011TRX.REG0xC->bits.enPcali = 0;
    BK7011TRX.REG0xC->bits.enPcaliGm = 0;
    BK7011TRX.REG0xF->bits.tssicalen = 1;

    CAL_WR_TRXREGS(0xC);
    CAL_WR_TRXREGS(0xB);
    CAL_WR_TRXREGS(0xF);

    low = bk7011_cal_tx_ty2(512);
    low = 1;
    BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP =  low;
    BK7011RCBEKEN.REG0x51->bits.TXTY2 = bk7011_cal_tx_ty2( low);
    detector_i_phase = bk7011_get_tx_i_phase();
    detector_q_phase = bk7011_get_tx_q_phase();
    detect_phase_low = abs(detector_i_phase - detector_q_phase);

    high = 1023;
    BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP =  high;
    BK7011RCBEKEN.REG0x51->bits.TXTY2 = bk7011_cal_tx_ty2( high);
    detector_i_phase = bk7011_get_tx_i_phase();
    detector_q_phase = bk7011_get_tx_q_phase();
    detect_phase_high = abs(detector_i_phase - detector_q_phase);

    do
    {
        if(detect_phase_low < detect_phase_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP =  high;
            BK7011RCBEKEN.REG0x51->bits.TXTY2 = bk7011_cal_tx_ty2( high);
            detector_i_phase = bk7011_get_tx_i_phase();
            detector_q_phase = bk7011_get_tx_q_phase();
            detect_phase_high = abs(detector_i_phase - detector_q_phase);
        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP =  low;
            BK7011RCBEKEN.REG0x51->bits.TXTY2 = bk7011_cal_tx_ty2( low);
            detector_i_phase = bk7011_get_tx_i_phase();
            detector_q_phase = bk7011_get_tx_q_phase();
            detect_phase_low = abs(detector_i_phase - detector_q_phase);
        }
    }
    while((high - low) > 1);
    index = ((detect_phase_low < detect_phase_high) ? low : high);
    BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP = index;

    gtx_phase_comp =  BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP;
    gtx_phase_ty2 = BK7011RCBEKEN.REG0x51->bits.TXTY2;

    CAL_WARN("gtx_phase_comp:%d\r\n", gtx_phase_comp);
    CAL_WARN("gtx_phase_ty2:%d\r\n", gtx_phase_ty2);

    gold_index = index;
    return gold_index;
}

INT32 bk7011_cal_tx_loopback_phase_imbalance(INT32 *val)
{
    INT32 detect_phase_low = 0;
    INT32 detect_phase_high = 0;
    INT16 high, low;
    INT32 index = 0, gold_index = 0;
    INT32 detector_i_phase;
    INT32 detector_q_phase;

    INT32 tx_phase_comp;
    INT32 tx_phase_ty2;

    BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP = 512;
    BK7011RCBEKEN.REG0x51->bits.TXTY2 = 512;

    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 1;
    BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_LOOPBACK_IQ_VAL;
    CAL_WR_TRXREGS(0xD);

#ifdef CHIPLOOPBACK
    bk7011_set_tx_pa(5, 3, 4, 4);
#else
#ifdef CALIBRATE_TIMES
    bk7011_set_tx_pa(gi_gain_tx_loopback_pa_dgainPA20, gi_gain_tx_loopback_pa_dgainbuf20,5, 8);
	if (p_gbias_after_cal_array == NULL)
		{
	CAL_PRT("\r\nbk7011_cal_tx_loopback_phase_imbalance: \r\n");
	CAL_PRT("dgainPA20  = %d\r\n", BK7011TRX.REG0xC->bits.dgainPA20 );
	CAL_PRT("dgainbuf20 =%d\r\n",  BK7011TRX.REG0xC->bits.dgainbuf20);
	CAL_PRT("gctrlpga20  = %d\r\n", BK7011TRX.REG0xC->bits.gctrlpga20 );
	CAL_PRT("gctrlmod30 = %d\r\n", BK7011TRX.REG0xB->bits.gctrlmod30);
		}
#else
    bk7011_set_tx_pa(6, 5, 5, 8);
#endif
#endif

    BK7011TRX.REG0xC->bits.TSSIsel = 0;
    BK7011TRX.REG0xC->bits.enDCcali = 1;
    BK7011TRX.REG0xC->bits.enDCcaliGm1 = 0;
    BK7011TRX.REG0xC->bits.enIQcali = 1;
    BK7011TRX.REG0xC->bits.enPcali = 0;
    BK7011TRX.REG0xC->bits.enPcaliGm = 0;
    BK7011TRX.REG0xF->bits.tssicalen = 1;

    CAL_WR_TRXREGS(0xC);
    CAL_WR_TRXREGS(0xB);
    CAL_WR_TRXREGS(0xF);

    low = bk7011_cal_tx_ty2(512);
    low = 1;
    BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP =  low;
    BK7011RCBEKEN.REG0x51->bits.TXTY2 = bk7011_cal_tx_ty2( low);
    detector_i_phase = bk7011_get_tx_i_phase();
    detector_q_phase = bk7011_get_tx_q_phase();
    detect_phase_low = abs(detector_i_phase - detector_q_phase);

    high = 1023;
    BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP =  high;
    BK7011RCBEKEN.REG0x51->bits.TXTY2 = bk7011_cal_tx_ty2( high);
    detector_i_phase = bk7011_get_tx_i_phase();
    detector_q_phase = bk7011_get_tx_q_phase();
    detect_phase_high = abs(detector_i_phase - detector_q_phase);

    do
    {
        if(detect_phase_low < detect_phase_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP =  high;
            BK7011RCBEKEN.REG0x51->bits.TXTY2 = bk7011_cal_tx_ty2( high);
            detector_i_phase = bk7011_get_tx_i_phase();
            detector_q_phase = bk7011_get_tx_q_phase();
            detect_phase_high = abs(detector_i_phase - detector_q_phase);
        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP =  low;
            BK7011RCBEKEN.REG0x51->bits.TXTY2 = bk7011_cal_tx_ty2( low);
            detector_i_phase = bk7011_get_tx_i_phase();
            detector_q_phase = bk7011_get_tx_q_phase();
            detect_phase_low = abs(detector_i_phase - detector_q_phase);
        }
    }
    while((high - low) > 1);
    index = ((detect_phase_low < detect_phase_high) ? low : high);
    BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP = index;

    tx_phase_comp =  BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP;
    tx_phase_ty2 = BK7011RCBEKEN.REG0x51->bits.TXTY2;
	
#ifdef CALIBRATE_TIMES
    if (p_gtx_phase_comp_temp_array != NULL)
    {
	p_gtx_phase_comp_temp_array[calibrate_time] = tx_phase_comp;
    }
#endif

    CAL_WARN("tx_phase_comp:%d\r\n", tx_phase_comp);
    CAL_WARN("tx_phase_ty2:%d\r\n", tx_phase_ty2);

    gold_index = index;
    return gold_index;
}


static float bk7011_get_tx_filter_i_ratio(void)
{
    INT32 rx_avg_i_14M, rx_avg_i_500K;
    float rx_avg_ratio;
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 358; // 14MHz;
    cal_delay_100us(gst_rx_adc);
    rx_avg_i_14M = bk7011_get_rx_i_avg_signed();
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 13; // 500KHz;
    cal_delay_100us(gst_rx_adc);
    rx_avg_i_500K = bk7011_get_rx_i_avg_signed();
    if(rx_avg_i_14M > 0)
    {
        rx_avg_ratio = abs(1.0 * rx_avg_i_500K / rx_avg_i_14M - 1.414);
        return rx_avg_ratio;
    }
    else
    {
        return -1.0;
    }
}
static float bk7011_get_tx_filter_q_ratio(void)
{
    INT32 rx_avg_i_14M, rx_avg_i_500K;
    float rx_avg_ratio;
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 358; // 14MHz;
    cal_delay_100us(gst_rx_adc);
    rx_avg_i_14M = bk7011_get_rx_q_avg_signed();
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 13; // 500KHz;
    cal_delay_100us(gst_rx_adc);
    rx_avg_i_500K = bk7011_get_rx_q_avg_signed();
    if(rx_avg_i_14M > 0)
    {
        rx_avg_ratio = abs(1.0 * rx_avg_i_500K / rx_avg_i_14M - 1.414);
        return rx_avg_ratio;
    }
    else
    {
        return -1.0;
    }
}

#ifdef FILTER_RATIO
static float bk7011_get_rx_filter_i_ratio(void)
{
    INT32 rx_avg_i_9M, rx_avg_i_500K;
    float rx_avg_i_ratio;
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 230; // 9MHz;
    cal_delay_100us(gst_rx_adc); //ST_RX_ADC_IQ
    rx_avg_i_9M = bk7011_get_rx_i_avg_signed();
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 13; // 500KHz;
    cal_delay_100us(gst_rx_adc); //ST_RX_ADC_IQ
    rx_avg_i_500K = bk7011_get_rx_i_avg_signed();
    if(rx_avg_i_9M > 0)
    {
        rx_avg_i_ratio = abs(1.0 * rx_avg_i_500K / rx_avg_i_9M - 1.414);
        return rx_avg_i_ratio;
    }
    else
    {
        return -1.0;
    }

}

static float bk7011_get_rx_filter_q_ratio(void)
{
    INT32 rx_avg_i_9M, rx_avg_i_500K;
    float rx_avg_i_ratio;
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 358; // 9MHz;
    cal_delay_100us(gst_rx_adc);//ST_RX_ADC_IQ
    rx_avg_i_9M = bk7011_get_rx_q_avg_signed();
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 13; // 500KHz;
    cal_delay_100us(gst_rx_adc); //ST_RX_ADC_IQ
    rx_avg_i_500K = bk7011_get_rx_q_avg_signed();
    if(rx_avg_i_9M > 0)
    {
        rx_avg_i_ratio = abs(1.0 * rx_avg_i_500K / rx_avg_i_9M - 1.414);
        return rx_avg_i_ratio;
    }
    else
    {
        return -1.0;
    }
}
#endif // FILTER_RATIO

INT32 bk7011_cal_tx_filter_corner(INT32 *val)
{

    float tx_avg_ratio_low = 0.0;
    float tx_avg_ratio_high = 0.0;
    INT16 high, low;
    INT32 index = 0, gold_index = 0;

    BK7011RCBEKEN.REG0x1C->bits.FRXON = 1;
    // I CAL
    BK7011TRX.REG0x6->bits.lpfcapcali50 = 0x20;
    BK7011TRX.REG0xD->value = 0xFC4E03B9;//BK7011TRXREG0xD;//0xE00F02B9;//0xFC4E03B9;//
    CAL_WR_TRXREGS(0x6);
    CAL_WR_TRXREGS(0xD);

    //12/10/2014 for D version
    BK7011TRX.REG0xF->bits.tssicalen = 0;
    BK7011TRX.REG0xF->bits.sinadrxen = 0;
    CAL_WR_TRXREGS(0xF);

    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 2;
    BK7011RCBEKEN.REG0x4D->bits.TXSINMODE = 1;
    BK7011RCBEKEN.REG0x4D->bits.TXSINAMP = 4;
    BK7011RCBEKEN.REG0x3C->bits.RXDCCALEN = 1;
    BK7011RCBEKEN.REG0x3C->bits.RXAVGMODE = 1;

    low = 0;
    BK7011TRX.REG0x6->bits.lpfcapcali50 = low;
    CAL_WR_TRXREGS(0x6);

    tx_avg_ratio_low = bk7011_get_tx_filter_i_ratio();

    high = 63;
    BK7011TRX.REG0x6->bits.lpfcapcali50 = high;
    CAL_WR_TRXREGS(0x6);

    tx_avg_ratio_high = bk7011_get_tx_filter_i_ratio();

    do
    {
        if(tx_avg_ratio_low < tx_avg_ratio_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011TRX.REG0x6->bits.lpfcapcali50 = high;
            CAL_WR_TRXREGS(0x6);
            tx_avg_ratio_high = bk7011_get_tx_filter_i_ratio();
        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011TRX.REG0x6->bits.lpfcapcali50 = low;
            CAL_WR_TRXREGS(0x6);
            tx_avg_ratio_low = bk7011_get_tx_filter_i_ratio();
        }
    }
    while((high - low) > 1);
    index = ((tx_avg_ratio_low < tx_avg_ratio_high) ? low : high);
    gold_index = index << 8;
    gtx_ifilter_corner = index;
    // Q CAL
    BK7011TRX.REG0x6->bits.lpfcapcalq50 = 0x20;
    BK7011TRX.REG0xD->value = 0xFC4E03B9;//BK7011TRXREG0xD;
    CAL_WR_TRXREGS(0x6);
    CAL_WR_TRXREGS(0xD);

    //12/10/2014 for D version
    BK7011TRX.REG0xF->bits.tssicalen = 0;
    BK7011TRX.REG0xF->bits.sinadrxen = 0;
    CAL_WR_TRXREGS(0xF);

    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 2;
    BK7011RCBEKEN.REG0x4D->bits.TXSINMODE = 2;
    BK7011RCBEKEN.REG0x4D->bits.TXSINAMP = 4;
    BK7011RCBEKEN.REG0x3C->bits.RXDCCALEN = 1;
    BK7011RCBEKEN.REG0x3C->bits.RXAVGMODE = 1;

    low = 0;
    BK7011TRX.REG0x6->bits.lpfcapcalq50 = low;
    CAL_WR_TRXREGS(0x6);

    tx_avg_ratio_low = bk7011_get_tx_filter_q_ratio();

    high = 63;
    BK7011TRX.REG0x6->bits.lpfcapcalq50 = high;
    CAL_WR_TRXREGS(0x6);

    tx_avg_ratio_high = bk7011_get_tx_filter_q_ratio();

    do
    {
        if(tx_avg_ratio_low < tx_avg_ratio_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011TRX.REG0x6->bits.lpfcapcalq50 = high;
            CAL_WR_TRXREGS(0x6);

            tx_avg_ratio_high = bk7011_get_tx_filter_q_ratio();
        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011TRX.REG0x6->bits.lpfcapcalq50 = low;
            CAL_WR_TRXREGS(0x6);

            tx_avg_ratio_low = bk7011_get_tx_filter_q_ratio();
        }
    }
    while((high - low) > 1);
    index = ((tx_avg_ratio_low < tx_avg_ratio_high) ? low : high);
    gold_index += index;
    gtx_qfilter_corner = index;//((index + ((gold_index & 0x0000ff00)>>8))>>1);


    BK7011TRX.REG0x6->bits.lpfcapcali50 = gtx_ifilter_corner;
    BK7011TRX.REG0x6->bits.lpfcapcalq50 = gtx_ifilter_corner;//160405 iq used same value
    gtx_qfilter_corner = gtx_ifilter_corner;
    CAL_WARN("gtx_ifilter_corner over: 0x%x\r\n", gtx_ifilter_corner);
    CAL_WARN("gtx_qfilter_corner over: 0x%x\r\n", gtx_qfilter_corner);

    //BK7011TRX.REG0x6->bits.lpfcapcalq50 = gtx_qfilter_corner;
    CAL_WR_TRXREGS(0x6);

    *val = gold_index;
    return (gold_index);
}

void bk7011_rx_cal_en(void)
{
    BK7011RCBEKEN.REG0x3E->bits.RXCOMPEN = 0;

    BK7011RCBEKEN.REG0x1C->bits.FRXON = 1;
    BK7011RCBEKEN.REG0x1C->bits.FTXON = 0;

    BK7011RCBEKEN.REG0x0->bits.forceenable = 1;
    cpu_delay(1);
    BK7011RCBEKEN.REG0x19->bits.FCH0SHDN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0TXEN = 0;
    BK7011RCBEKEN.REG0x19->bits.FCH0RXEN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0EN = 1;

    // ADC clock change to 80M    
    BK7011TRX.REG0xF->bits.clkadcsel = 1;
    CAL_WR_TRXREGS(0xF);

    BK7011TRX.REG0x12->bits.adcrefbwsel = 1;
    CAL_WR_TRXREGS(0x12);

    return;
}

INT32 bk7011_cal_rx_dc(void)
{
    INT32 index = 0;
    INT32 i, j, k, t, curr, value;
	

    /*step 2*/
    BK7011RCBEKEN.REG0x3C->bits.RXDCCALEN = 1;
    BK7011RCBEKEN.REG0x3C->bits.RXAVGMODE = 0;
    BK7011RCBEKEN.REG0x19->bits.FCH0RXHP = 0; //huaming.jiang 20141017
    BK7011RCBEKEN.REG0x1C->bits.FTXON = 0;
    BK7011RCBEKEN.REG0x1C->bits.FRXON = 1;

    BK7011TRX.REG0x7->bits.chin60 = 0x64;
    CAL_WR_TRXREGS(0x7);
	
    BK7011TRX.REG0xE->value = 0xDA01BCF0; /// D801BCF0;//170217 
    CAL_WR_TRXREGS(0xE);	

#if 1
    for(i = 0; i < 16; i ++)
    {
        BK7011RCBEKEN.REG0x19->bits.FCH0B = (0x70 | i);
        for(j = 0; j < 2; j ++)
        {
            index = 128;
            k = 6;
            do
            {
                //set dc offset
                value = (*((volatile unsigned long *)(TRX_BEKEN_BASE + (0x14 + i / 2) * 4)));
                curr = ~(0xff << (16 * (i % 2) + 8 * j));
                value &= curr;
                curr = (index << (16 * (i % 2) + 8 * j));
                value |= curr;
                (*((volatile unsigned long *)(TRX_BEKEN_BASE + (0x14 + i / 2) * 4))) = value;
                while(BK7011RCBEKEN.REG0x1->value & 0xfffffff);
                cal_delay_100us(gst_rx_adc);

                //read dc avg, and calc mean
                value = 0;
                for(t = 0; t < 10; t ++)
                {
                    if(j == 0)  curr = BK7011RCBEKEN.REG0x3C->bits.RXAVGIRD;
                    else        curr = BK7011RCBEKEN.REG0x3C->bits.RXAVGQRD;
                    if(curr >= 2048) curr -= 4096;
                    value += curr;
                    cpu_delay(100);
                }
                curr = value / 10;

                //calc new dc offset
                if(curr > 0) index += (0x1 << k);
                else         index -= (0x1 << k);
                k --;
            }
            while((k >= 0) && ((curr >= 16) || (curr <= -16)));
            if(k < 0)
            {
                value = (*((volatile unsigned long *)(TRX_BEKEN_BASE + (0x14 + i / 2) * 4)));
                curr = ~(0xff << (16 * (i % 2) + 8 * j));
                value &= curr;
                curr = (index << (16 * (i % 2) + 8 * j));
                value |= curr;
                (*((volatile unsigned long *)(TRX_BEKEN_BASE + (0x14 + i / 2) * 4))) = value;
                while(BK7011RCBEKEN.REG0x1->value & 0xfffffff);
            }
        }
    }

    g_rx_dc_gain_tab[0] = BK7011TRXONLY.REG0x14->value;
    g_rx_dc_gain_tab[1] = BK7011TRXONLY.REG0x15->value;
    g_rx_dc_gain_tab[2] = BK7011TRXONLY.REG0x16->value;
    g_rx_dc_gain_tab[3] = BK7011TRXONLY.REG0x17->value;
    g_rx_dc_gain_tab[4] = BK7011TRXONLY.REG0x18->value;
    g_rx_dc_gain_tab[5] = BK7011TRXONLY.REG0x19->value;
    g_rx_dc_gain_tab[6] = BK7011TRXONLY.REG0x1A->value;
    g_rx_dc_gain_tab[7] = BK7011TRXONLY.REG0x1B->value;
    CAL_WARN("g_rx_dc_gain_tab 0 over: 0x%x\r\n", g_rx_dc_gain_tab[0]);
    CAL_WARN("g_rx_dc_gain_tab 1 over: 0x%x\r\n", g_rx_dc_gain_tab[1]);
    CAL_WARN("g_rx_dc_gain_tab 2 over: 0x%x\r\n", g_rx_dc_gain_tab[2]);
    CAL_WARN("g_rx_dc_gain_tab 3 over: 0x%x\r\n", g_rx_dc_gain_tab[3]);
    CAL_WARN("g_rx_dc_gain_tab 4 over: 0x%x\r\n", g_rx_dc_gain_tab[4]);
    CAL_WARN("g_rx_dc_gain_tab 5 over: 0x%x\r\n", g_rx_dc_gain_tab[5]);
    CAL_WARN("g_rx_dc_gain_tab 6 over: 0x%x\r\n", g_rx_dc_gain_tab[6]);
    CAL_WARN("g_rx_dc_gain_tab 7 over: 0x%x\r\n", g_rx_dc_gain_tab[7]);

    BK7011RCBEKEN.REG0x3C->bits.RXDCCALEN = 0;
	
    BK7011TRX.REG0x7->bits.chin60 = 62;
    CAL_WR_TRXREGS(0x7);
#endif

    return 0;
}


INT32 bk7011_cal_rx_iq(INT32 *val)
{
    INT32 rx_amp_err_rd, rx_phase_err_rd, rx_ty2_rd;
    INT32 rx_amp_err_wr;
    INT32 rx_phase_err_wr;
    float amp_err, phase_err, ty2_err;
    INT32 gold_index = 0;
    INT32 i, curr, value, value1, value2;
    //bk7011_load_rxiq_init_cfg(); //v1.5

    /*step 1*/
    BK7011RCBEKEN.REG0x1C->bits.FRXON = 1;
    BK7011RCBEKEN.REG0x1C->bits.FTXON = 1;

    BK7011TRX.REG0xF->bits.tssicalen = 0;
    BK7011TRX.REG0xF->bits.sinadrxen = 0;
    CAL_WR_TRXREGS(0xF);

    BK7011TRX.REG0xE->value = TRX_REG_0XE_RXIQ_VAL;
    CAL_WR_TRXREGS(0xE);
    bk7011_set_tx_pa(5, 5, 4, 4);	
    CAL_WR_TRXREGS(0xC);
    BK7011RCBEKEN.REG0x19->bits.FCH0B = 0x9;

    /*searching...*/
    BK7011RCBEKEN.REG0x3E->bits.RXCALEN = 1;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 2;
    BK7011RCBEKEN.REG0x4D->bits.TXSINMODE = 0;
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 179; // 7MHz;
    cal_delay_100us(gst_rx_adc);
    cpu_delay(500 * DELAY1US);

    BK7011RCBEKEN.REG0x41->bits.RXDCIWR = 0x0;
    BK7011RCBEKEN.REG0x41->bits.RXDCQWR = 0x0;


#if 1
    value = 0;
    value1 = 0;
    value2 = 0;
    for(i = 0; i < 10; i ++)
    {
        curr = BK7011RCBEKEN.REG0x3F->bits.RXAMPERRRD - ((BK7011RCBEKEN.REG0x3F->bits.RXAMPERRRD < 512 ) ? 0 : 1024);
        value += curr;
        curr = BK7011RCBEKEN.REG0x3F->bits.RXPHASEERRRD - (( BK7011RCBEKEN.REG0x3F->bits.RXPHASEERRRD < 512) ? 0 : 1024);
        value1 += curr;
        curr = BK7011RCBEKEN.REG0x40->bits.RXTY2RD - ((BK7011RCBEKEN.REG0x40->bits.RXTY2RD < 512) ? 0 : 1024);
        value2 += curr;
        cpu_delay(40 * DELAY1US);
    }
    rx_amp_err_rd = value / 10;
    rx_phase_err_rd = value1 / 10;
    rx_ty2_rd = value2 / 10;
    
    #ifdef CALIBRATE_TIMES      // by gwf
    if (p_rx_amp_err_rd_array != NULL)
    {
        int rx_amp_err_rd_temp;
        int rx_phase_err_rd_temp;
        if (rx_amp_err_rd & 0x200)
        {
            rx_amp_err_rd_temp = rx_amp_err_rd | 0xFFFFFC00;
        }
        else
        {
            rx_amp_err_rd_temp = rx_amp_err_rd & 0x000003FF;
        }
        if (rx_phase_err_rd & 0x200)
        {
            rx_phase_err_rd_temp = rx_phase_err_rd | 0xFFFFFC00;
        }
        else
        {
            rx_phase_err_rd_temp = rx_phase_err_rd & 0x000003FF;
        }
        p_rx_amp_err_rd_array[calibrate_time] = rx_amp_err_rd_temp;
        p_rx_phase_err_rd_array[calibrate_time] = rx_phase_err_rd_temp;
    }
    else
    #endif
    {
    CAL_FATAL("[rx_iq]rx_amp_err_rd: 0x%03x\r\n", rx_amp_err_rd );
    CAL_FATAL("[rx_iq]rx_phase_err_rd: 0x%03x\r\n", rx_phase_err_rd );    
    }
#endif

    amp_err = 1.0 * rx_amp_err_rd / 1024;
    phase_err = 1.0 * rx_phase_err_rd / 1024;
    ty2_err = 1.0 * rx_ty2_rd / 1024;

    //分母为0的判断;amp_err = [-0.5,+0.5],ty2_err = [-0.5,+0.5],
    //所以 amp_err+1 和 ty2_err+1 都不会为0;
    rx_amp_err_wr = (INT32) (512 * (ty2_err + 1) / (amp_err + 1));
    rx_phase_err_wr = (INT32) (512 * phase_err * (ty2_err + 1));

#ifdef INVPHASE
    BK7011RCBEKEN.REG0x42->bits.RXPHASEERRWR = rx_phase_err_wr;
#else
    BK7011RCBEKEN.REG0x42->bits.RXPHASEERRWR = rx_phase_err_wr;
#endif

    BK7011RCBEKEN.REG0x42->bits.RXAMPERRWR = rx_amp_err_wr;
    BK7011RCBEKEN.REG0x3E->bits.RXCOMPEN = 1;
    BK7011RCBEKEN.REG0x3E->bits.RXCALEN = 0;

    grx_amp_err_wr = rx_amp_err_wr;
    grx_phase_err_wr = rx_phase_err_wr;

    CAL_WARN("grx_amp_err_wr:0x%03x\r\n", grx_amp_err_wr);
    CAL_WARN("grx_phase_err_wr:0x%03x\r\n", grx_phase_err_wr);

    gold_index = (rx_amp_err_wr << 16 ) + rx_phase_err_wr;
    *val = gold_index;
    return gold_index;
}

void bk7011_set_rx_avg_dc(void)
{
    INT32 rx_dc_i_rd, rx_dc_q_rd;

    BK7011RCBEKEN.REG0x3E->bits.RXCOMPEN = 0;
    BK7011RCBEKEN.REG0x3C->bits.RXAVGMODE = 0;
    BK7011RCBEKEN.REG0x3C->bits.RXDCCALEN = 1;
    cal_delay_100us(gst_rx_adc);
    BK7011RCBEKEN.REG0x3C->bits.RXDCCALEN = 0;

    rx_dc_i_rd = BK7011RCBEKEN.REG0x3C->bits.RXAVGIRD;
    rx_dc_q_rd = BK7011RCBEKEN.REG0x3C->bits.RXAVGQRD;
    BK7011RCBEKEN.REG0x41->bits.RXDCIWR = rx_dc_i_rd;
    BK7011RCBEKEN.REG0x41->bits.RXDCQWR = rx_dc_q_rd;

    BK7011RCBEKEN.REG0x3E->bits.RXCOMPEN = 1;

    return;
}

INT32 bk7011_load_calibration_cfg(void)
{
    BK7011RCBEKEN.REG0x0->value  = BK7011RCBEKEN.REG0x0->value;
    BK7011RCBEKEN.REG0x1->value  = BK7011RCBEKEN.REG0x1->value;
    BK7011RCBEKEN.REG0x5->value  = BK7011RCBEKEN.REG0x5->value ;
    BK7011RCBEKEN.REG0x8->value  = BK7011RCBEKEN.REG0x8->value;
    BK7011RCBEKEN.REG0xB->value  = BK7011RCBEKEN.REG0xB->value;
    BK7011RCBEKEN.REG0x11->value = BK7011RCBEKEN.REG0x11->value;
    BK7011RCBEKEN.REG0x19->value = BK7011RCBEKEN.REG0x19->value;
    BK7011RCBEKEN.REG0x1E->value = BK7011RCBEKEN.REG0x1E->value;

    /**********NEW ADDED************/
    BK7011RCBEKEN.REG0x3C->value = BK7011RCBEKEN.REG0x3C->value;
    BK7011RCBEKEN.REG0x3E->value = BK7011RCBEKEN.REG0x3E->value;
    BK7011RCBEKEN.REG0x3F->value = BK7011RCBEKEN.REG0x3F->value;
    BK7011RCBEKEN.REG0x40->value = BK7011RCBEKEN.REG0x40->value;
    BK7011RCBEKEN.REG0x41->value = BK7011RCBEKEN.REG0x41->value;
    BK7011RCBEKEN.REG0x42->value = BK7011RCBEKEN.REG0x42->value ;
    BK7011RCBEKEN.REG0x4C->value = BK7011RCBEKEN.REG0x4C->value;
    BK7011RCBEKEN.REG0x4D->value = BK7011RCBEKEN.REG0x4D->value;
    BK7011RCBEKEN.REG0x4F->value = BK7011RCBEKEN.REG0x4F->value;
    BK7011RCBEKEN.REG0x50->value = BK7011RCBEKEN.REG0x50->value;
    BK7011RCBEKEN.REG0x51->value = BK7011RCBEKEN.REG0x51->value;
    BK7011RCBEKEN.REG0x52->value = BK7011RCBEKEN.REG0x52->value;
    BK7011RCBEKEN.REG0x54->value = BK7011RCBEKEN.REG0x54->value;
    BK7011RCBEKEN.REG0x5C->value = BK7011RCBEKEN.REG0x5C->value;

    while(BK7011RCBEKEN.REG0x1->value & 0x0FFFFFFF)
    {
        cpu_delay(1);
    }
    BK7011TRXONLY.REG0x0->value = grc_reg_map[0];
    BK7011TRXONLY.REG0x1->value = grc_reg_map[1];
    BK7011TRXONLY.REG0x2->value = grc_reg_map[2];
    BK7011TRXONLY.REG0x3->value = grc_reg_map[3];
    BK7011TRXONLY.REG0x4->value = grc_reg_map[4];
    BK7011TRXONLY.REG0x6->value = grc_reg_map[6];
    BK7011TRXONLY.REG0x7->value = grc_reg_map[7];
    BK7011TRXONLY.REG0x8->value = grc_reg_map[8];
    BK7011TRXONLY.REG0x9->value = grc_reg_map[9];
    BK7011TRXONLY.REG0xA->value = grc_reg_map[10];
    BK7011TRXONLY.REG0xB->value = grc_reg_map[11];
    BK7011TRXONLY.REG0xC->value = grc_reg_map[12];
    BK7011TRXONLY.REG0xD->value = grc_reg_map[13];
    BK7011TRXONLY.REG0xE->value = grc_reg_map[14];
    BK7011TRXONLY.REG0x11->value = grc_reg_map[17];
    BK7011TRXONLY.REG0x12->value = grc_reg_map[18];
    BK7011TRXONLY.REG0x13->value = grc_reg_map[19];
    BK7011TRXONLY.REG0x14->value = grc_reg_map[20];
    BK7011TRXONLY.REG0x15->value = grc_reg_map[21];
    BK7011TRXONLY.REG0x16->value = grc_reg_map[22];
    BK7011TRXONLY.REG0x17->value = grc_reg_map[23];
    BK7011TRXONLY.REG0x18->value = grc_reg_map[24];
    BK7011TRXONLY.REG0x19->value = grc_reg_map[25];
    BK7011TRXONLY.REG0x1A->value = grc_reg_map[26];
    BK7011TRXONLY.REG0x1B->value = grc_reg_map[27];

    while(BK7011RCBEKEN.REG0x1->value & 0x0FFFFFFF)
    {
        cpu_delay(1);
    }

    BK7011TRX.REG0x7->bits.autorxifgen = 0;
    BK7011TRX.REG0x7->bits.digdcoen = 0;
    BK7011TRX.REG0x7->bits.spilpfrxg30 = 6;
    CAL_WR_TRXREGS(0x7);

    BK7011TRX.REG0x6->bits.lpfcapcali50 = gtx_ifilter_corner;
    BK7011TRX.REG0x6->bits.lpfcapcalq50 = gtx_ifilter_corner;
    CAL_WR_TRXREGS(0x6);

    bk7011_set_rx_avg_dc(); // 11/11/2014

    return 0;
}

void bk7011_set_tx_after_cal(void)
{
    BK7011RCBEKEN.REG0x0->bits.forceenable = 0;
    cpu_delay(1);
    BK7011RCBEKEN.REG0x19->bits.FCH0EN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0SHDN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0RXEN = 0;
    BK7011RCBEKEN.REG0x19->bits.FCH0TXEN = 1;
    BK7011RCBEKEN.REG0x1C->bits.FRXON = 0;
    BK7011RCBEKEN.REG0x1C->bits.FTXON = 1;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 0;
}

void bk7011_set_rx_after_cal(void)
{
    BK7011RCBEKEN.REG0x0->bits.forceenable = 0;
    cpu_delay(1);
    BK7011RCBEKEN.REG0x19->bits.FCH0EN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0SHDN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0RXEN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0TXEN = 0;
    BK7011RCBEKEN.REG0x1C->bits.FRXON = 1;
    BK7011RCBEKEN.REG0x1C->bits.FTXON = 0;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 0;

#if BK7211_68PIN_BOARD

#else
    BK7011TRX.REG0xE->value = 0xDA01BCF0;
    CAL_WR_TRXREGS(0xE);
#endif

    // ADC clock change to 40M
    BK7011TRX.REG0xF->bits.clkadcsel = 0;
    CAL_WR_TRXREGS(0xF);

    BK7011TRX.REG0x12->bits.adcrefbwsel = 0;
    CAL_WR_TRXREGS(0x12);
}

extern void mpb_tx_mode(void);

#define CALI_DPD_TEST       0
#if CALI_DPD_TEST
#define I_TABLE_ADDR        0x01050400
#define Q_TABLE_ADDR        0x01050600

static UINT16 i_table_val[256] =
{
    0, 6, 13, 19, 26, 35, 40, 47, 52, 57, 68, 73, 76, 82, 88, 91, 96, 102, 107, 107, 118, 118, 120, 127, 132, 134, 139, 141, 146, 149, 152, 158, 161, 161, 163, 164, 168, 172, 172, 176, 181, 177, 179, 181, 185, 187, 189, 185, 191, 195, 196, 195, 196, 197, 203, 198, 204, 201, 207, 199, 206, 207, 207, 207, 207, 210, 210, 212, 214, 215, 215, 215, 206, 216, 215, 221, 217, 219, 215, 219, 222, 222, 225, 229, 225, 223, 228, 226, 226, 229, 229, 226, 225, 227, 226, 226, 228, 232, 230, 229, 230, 231, 230, 231, 234, 235, 236, 238, 241, 244, 245, 247, 248, 251, 252, 255, 255, 258, 259, 262, 263, 265, 267, 268, 271, 272, 275, 275, 278, 280, 282, 284, 287, 288, 291, 293, 295, 297, 299, 301, 304, 306, 308, 310, 312, 314, 317, 319, 321, 323, 325, 327, 330, 332, 334, 336, 338, 341, 343, 345, 347, 349, 351, 354, 356, 358, 360, 362, 364, 367, 369, 371, 373, 375, 377, 380, 382, 384, 386, 388, 390, 393, 395, 397, 399, 401, 403, 406, 408, 410, 412, 414, 416, 419, 421, 423, 425, 427, 429, 432, 434, 436, 438, 440, 442, 445, 447, 449, 451, 453, 455, 458, 460, 462, 464, 466, 468, 471, 473, 475, 477, 479, 481, 484, 486, 488, 490, 492, 495, 497, 499, 501, 503, 505, 508, 510, 512, 514, 516, 518, 521, 523, 525, 527, 529, 531, 534, 536, 538, 540, 542, 544, 547, 549, 551, 562
};

static UINT16 q_table_val[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 1, 5, 8, 5, 9, 6, 5, 7, 4, 7, 8, 17, 13, 12, 14, 15, 12, 12, 18, 12, 13, 16, 16, 17, 19, 20, 24, 22, 30, 23, 21, 24, 30, 27, 26, 24, 27, 26, 30, 28, 30, 32, 31, 31, 32, 32, 33, 35, 35, 33, 35, 34, 32, 32, 32, 34, 33, 32, 31, 32, 30, 33, 29, 30, 29, 30, 29, 29, 28, 27, 29, 27, 28, 26, 26, 26, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 37, 37, 37, 37, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 41, 41, 41, 41, 41, 42, 42, 42, 42, 42, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 47, 47, 47, 47, 48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 50, 50, 50, 50, 50, 51, 51, 51, 51, 51, 52, 52, 48
};

UINT32 bk7211_cal_tx_dpd_load(void)
{

    UINT16 *i_tbl_addr = (UINT16 *)I_TABLE_ADDR;
    UINT16 *q_tbl_addr = (UINT16 *)Q_TABLE_ADDR;
    UINT16 k;

    BK7011RCBEKEN.REG0x4C->bits.DPDEN = 0;

    os_memcpy(i_tbl_addr, (UINT16 *)&i_table_val[0], 256 * 2);
    os_memcpy(q_tbl_addr, (UINT16 *)&q_table_val[0], 256 * 2);

    for(k = 0; k < 256; k++)
    {
        i_tbl_addr[k] = 1;
        q_tbl_addr[k] = 0;
    }
	
    return 0;

}
#endif

void sctrl_dpll_int_open(void);
void calibration_main(void)
{
    INT32 goldval[32] = {0};

    bk7011_cal_ready();
    bk7011_cal_bias();

    bk7011_cal_pll();

    BK7011TRX.REG0xB->value = TRX_REG_0XB_VAL;
    CAL_WR_TRXREGS(0xB);
    BK7011TRX.REG0xC->value = TRX_REG_0XC_VAL;
    CAL_WR_TRXREGS(0xC);

    bk7011_tx_cal_en();

#if BK7211_68PIN_BOARD
#else
    gtxoutput = 1;

    cpu_delay(100);

    bk7011_cal_tx_output_power(goldval);  // first time
#endif

    bk7011_cal_tx_dc(goldval);

    bk7011_cal_tx_gain_imbalance(goldval);

    bk7011_cal_tx_phase_imbalance(goldval);
    bk7011_cal_tx_dc(goldval);    // second DC cal,improve LO leakage,but maybe degrade the EVM, Does it need?

    bk7011_cal_tx_filter_corner(goldval);

#if BK7211_68PIN_BOARD

#else
    gtxoutput = 0;
    cpu_delay(100);
	
    bk7011_cal_tx_output_power(goldval);  // second time
#endif

    bk7011_set_tx_after_cal();

#if 1
    bk7011_rx_cal_en();
    bk7011_cal_rx_dc();

#if BK7211_68PIN_BOARD
#else
    bk7011_tx_cal_en();

    gtxoutput = 1;
    cpu_delay(100);
    bk7011_cal_tx_output_power(goldval);  // 3rd time
    
    bk7011_cal_tx_loopback_dc(goldval);
    bk7011_cal_tx_loopback_gain_imbalance(goldval);

#ifdef CALIBRATE_TIMES
    if (p_gtx_i_gain_comp_temp_array != NULL)
    {
	p_gtx_i_gain_comp_temp_array[calibrate_time] =( (*((volatile unsigned long * )(0x01050000 + 0x50*4))) >> 16) & 0x03FF ;
	p_gtx_q_gain_comp_temp_array[calibrate_time] = (*((volatile unsigned long * )(0x01050000 + 0x50*4))) & 0x03FF ;
    }
#endif
    bk7011_cal_tx_loopback_phase_imbalance(goldval);
    bk7011_rx_cal_en();
    bk7011_cal_rx_iq(goldval);
#endif
    bk7011_set_rx_after_cal();

    rwnx_cal_save_cal_result();
    rwnx_cal_load_default_result();
    rwnx_cal_read_current_cal_result();
#endif

#if CALI_DPD_TEST
    rwnx_cal_load_trx_rcbekn_reg_val();
    bk7211_cal_tx_dpd_load();
#endif
    bk7011_cal_dpll();
    sctrl_dpll_int_open();

    return ;
}

void do_calibration_in_temp_dect(void)
{
    INT32 goldval[32] = {0};

    BK7011TRX.REG0xA->value = TRX_REG_0XA_VAL;
    CAL_WR_TRXREGS(0xA);
    BK7011TRX.REG0xB->value = TRX_REG_0XB_VAL;
    CAL_WR_TRXREGS(0xB);
    BK7011TRX.REG0xC->value = TRX_REG_0XC_VAL;
    CAL_WR_TRXREGS(0xC);

    bk7011_read_cal_param();
    rwnx_cal_load_default_result();
    cpu_delay(10);//cpu_delay(10);  20160801

    bk7011_cal_bias();

    BK7011RCBEKEN.REG0x4C->bits.TXCOMPDIS = 0;

    bk7011_tx_cal_en();

    gtxoutput = 5;
    bk7011_cal_tx_output_power(goldval);  // second time

    bk7011_set_tx_after_cal();

    rwnx_cal_save_cal_result();
    rwnx_cal_load_default_result();
    rwnx_cal_read_current_cal_result();

    bk7011_cal_dpll();
    sctrl_dpll_int_open();

    return ;
}

#define SET_LASMP_MODE(val) \
    i = REG_READ((0x00800000+0x12*4)); \
    i = (i & (~0x3)) | (val << 0);     \
    REG_WRITE((0x00800000+0x12*4), i);

#include "mem_pub.h"

void la_sample_printf(UINT8 smpsource)
{
    UINT32 i, len;

    UINT32 *buf_ptr = NULL;
    len = 0x1000;

    buf_ptr = os_malloc(len * sizeof(UINT32));
    if(buf_ptr == NULL)
    {
        CAL_PRT("la_sample_printf malloc buf failed\r\n");
        return;
    }

    REG_WRITE((0x00800000 + 0x12 * 4), 0);
    REG_WRITE((0x01060000 + 0x00 * 4), 0x21);
    cpu_delay(1000);
    REG_WRITE((0x00800000 + 0x15 * 4), ((UINT32)buf_ptr));
    REG_WRITE((0x00800000 + 0x12 * 4), ((len << 16)));

    switch(smpsource)
    {
    case 0: 	//ADC Data
        REG_WRITE((0x00800000 + 0x0d * 4), 0x00040000);
        REG_WRITE((0x00800000 + 0x14 * 4), 0xfe000000);
        REG_WRITE((0x00800000 + 0x13 * 4), 0x0a000000);
        break;
    case 1: 	//PHY ADC Data (After RC_BEKEN)
        REG_WRITE((0x00800000 + 0x0d * 4), 0x00050000);
        REG_WRITE((0x00800000 + 0x14 * 4), 0xfc000000);
        REG_WRITE((0x00800000 + 0x13 * 4), 0x2c000000);
        break;
    case 2: 	// 12 bit force
        REG_WRITE((0x00800000 + 0x0d * 4), 0x00050000);
        REG_WRITE((0x00800000 + 0x14 * 4), 0x00000000);
        REG_WRITE((0x00800000 + 0x13 * 4), 0x00000000);
        break;

    default:
        return;
    }

    SET_LASMP_MODE(1);
    while(!(REG_READ((0x00800000 + 0x12 * 4)) & 0x8))
        cpu_delay(1000);
    SET_LASMP_MODE(0);

    for(i = 0; i < len; i ++)
    {
        CAL_PRT("%08x\r\n", buf_ptr[i]);
    }

    if(buf_ptr)
        os_free(buf_ptr);

}
#endif  /* CFG_SUPPORT_CALIBRATION */
// eof


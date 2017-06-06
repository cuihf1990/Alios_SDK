#ifndef __TEMP_DETECT_H__
#define __TEMP_DETECT_H__

//#define TMP_DETECT_DEBUG
#ifdef TMP_DETECT_DEBUG
#define TMP_DETECT_PRT      os_printf
#define TMP_DETECT_WARN     warning_prf
#define TMP_DETECT_FATAL    fatal_prf
#else
#define TMP_DETECT_PRT      null_prf
#define TMP_DETECT_WARN     warning_prf
#define TMP_DETECT_FATAL    fatal_prf
#endif

#define ADC_TEMP_SENSER_CHANNEL                     7
#define ADC_TEMP_DETECT_BUFFER_SIZE                 5
#define ADC_TMEP_SYSCTRL_LDO_VAL                    0x14
#define ADC_TMEP_DETECT_INTVAL                      (10)  // how many second
#define ADC_TMEP_DO_CALIBRATION_TXOUTPOWR_THRE      (50)

typedef  struct temp_detect_config_st
{
    UINT32 last_detect_val;
    UINT32 detect_thre;
    UINT32 detect_intval;
    UINT32 do_cali_flag;
    struct etimer detect_timer;
    struct etimer calibration_timer;
} TEMP_DETECT_CONFIG_ST, TEMP_DETECT_CONFIG_PTR;

#endif
// eof


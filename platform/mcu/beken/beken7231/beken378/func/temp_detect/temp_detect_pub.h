#ifndef __TEMP_DETECT_PUB_H__
#define __TEMP_DETECT_PUB_H__

#if CFG_USE_TEMPERATURE_DETECT
void temp_detect_change_configuration(UINT32 intval, UINT32 thre);
UINT8 temp_detct_get_cali_flag(void);
UINT32 temp_detect_init(void);
UINT32 temp_detect_uninit(void);
#endif

#endif


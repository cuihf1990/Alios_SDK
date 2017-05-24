#ifndef _ALINK_EXPORT_INTERNAL_H_
#define _ALINK_EXPORT_INTERNAL_H_

#include "os.h"

//product line, basic/gateway/rf433/router etc.
#if defined(GATEWAY_SDK)
#include "alink_export_gateway.h"
#include "alink_export_zigbee.h"
#elif defined(RF433_SDK)
#include "alink_export_rf433.h"
#else
#include "alink_export.h"
#endif

//feature
#if defined(ASR_SDK)
#include "alink_export_asr.h"
#endif


/* Note: must consistent with alink_export_xxx.h */
enum alink_callback {
    /* cloud */
    _ALINK_CLOUD_CONNECTED = 0,
    _ALINK_CLOUD_DISCONNECTED = 1,

    /* wifi */
    _ALINK_GET_DEVICE_STATUS = 2,
    _ALINK_SET_DEVICE_STATUS = 3,
    _ALINK_GET_DEVICE_RAWDATA = 4,
    _ALINK_SET_DEVICE_RAWDATA = 5,
    
    /*OTA*/
    _ALINK_UPGRADE_DEVICE = 6,
    _ALINK_CANCEL_UPGRADE_DEVICE = 7,
    
    /* zigbee */
    _ALINK_ZIGBEE_GET_DEVICE_STATUS = 30,
    _ALINK_ZIGBEE_SET_DEVICE_STATUS,
    _ALINK_ZIGBEE_EXECUTE_DEVICE_CMD,
    _ALINK_ZIGBEE_UPDATE_ATTR_PROFILE,
    _ALINK_ZIGBEE_UPDATE_CMD_PROFILE,
    _ALINK_ZIGBEE_REMOVE_DEVICE,
    _ALINK_ZIGBEE_PERMIT_JOIN,

    /* asr */
};

#define ALINK_CB_MAX_NUM            (128)
extern void *alink_cb_func[ALINK_CB_MAX_NUM];

extern void system_monitor_init(void);
extern void system_monitor_exit(void);

#endif

#ifndef __IMPORT_PRODUCT_H__
#define __IMPORT_PRODUCT_H__

#include "iot_import.h"

#define PID_STR_MAXLEN              (64)
#define MID_STR_MAXLEN              (64)
#define PRODUCT_KEY_MAXLEN          (20 + 1)
#define DEVICE_NAME_MAXLEN          (32 + 1)
#define DEVICE_ID_MAXLEN            (64 + 1)
#define DEVICE_SECRET_MAXLEN        (64 + 1)
#define PRODUCT_SECRET_MAXLEN       (64 + 1)
#define FIRMWARE_VERSION_MAXLEN     (32 + 1)

/**
 * @brief   获取设备的`Partner ID`, 仅用于紧密合作伙伴
 *
 * @param   pid_str : 用来存放Partner ID字符串的数组
 * @return  写到pid_str[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_GetPartnerID(_OU_ char pid_str[PID_STR_MAXLEN]);

/**
 * @brief   获取设备的`Module ID`, 仅用于紧密合作伙伴
 *
 * @param   mid_str : 用来存放Module ID字符串的数组
 * @return  写到mid_str[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_GetModuleID(_OU_ char mid_str[MID_STR_MAXLEN]);

/**
 * @brief   获取设备的`ProductKey`, 用于标识设备的品类, 三元组之一
 *
 * @param   product_key : 用来存放ProductKey字符串的数组
 * @return  写到product_key[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_GetProductKey(_OU_ char product_key[PRODUCT_KEY_MAXLEN]);

/**
 * @brief   设置设备的`ProductKey`, 用于标识设备的品类, 三元组之一
 *
 * @param   product_key : 用于设置设备的`ProductKey`字符串
 * @return  成功写入设备的`ProductKey`的长度，单位是字节(Byte)
 */
int HAL_SetProductKey(_IN_ char product_key[PRODUCT_KEY_MAXLEN]);

/**
 * @brief   获取设备的`DeviceName`, 用于标识设备单品的名字, 三元组之一
 *
 * @param   device_name : 用来存放DeviceName字符串的数组
 * @return  写到device_name[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_GetDeviceName(_OU_ char device_name[DEVICE_NAME_MAXLEN]);

/**
 * @brief   设置设备的`DeviceName`, 用于标识设备单品的名字, 三元组之一
 *
 * @param   device_name : 用来设置设备的`DeviceName`字符串
 * @return  成功写入设备的`DeviceName`的长度,单位是字节(Byte)
 */
int HAL_SetDeviceName(_IN_ char device_name[DEVICE_NAME_MAXLEN]);

/**
 * @brief   获取设备的`DeviceSecret`, 用于标识设备单品的密钥, 三元组之一
 *
 * @param   device_secret : 用来存放DeviceSecret字符串的数组
 * @return  写到device_secret[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_GetDeviceSecret(_OU_ char device_secret[DEVICE_SECRET_MAXLEN]);

/**
 * @brief   设置设备的`DeviceSecret`, 用于标识设备单品的密钥, 三元组之一
 *
 * @param   device_secret : 用来设置设备的`DeviceSecret`字符串
 * @return  成功写入到设备的`DeviceSecret`, 单位是字节(Byte)
 */
int HAL_SetDeviceSecret(_IN_ char device_secret[DEVICE_SECRET_MAXLEN]);

/**
 * @brief   获取设备的`ProductSecret`, 用于标识设备单品的密钥
 *
 * @param   product_secret : 用来存放ProductSecret字符串的数组
 * @return  写到product_secret[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_GetProductSecret(_OU_ char product_secret[PRODUCT_SECRET_MAXLEN]);

/**
 * @brief   设置设备的`ProductSecret`, 用于标识设备单品的密钥
 *
 * @param   product_secret : 用来设置设备的`ProductSecret`字符串
 * @return  成功写入到设备的`ProductSecret`, 单位是字节(Byte)
 */
int HAL_SetProductSecret(_IN_ char product_secret[PRODUCT_SECRET_MAXLEN]);

/**
 * @brief   获取设备的`DeviceID`, 用于标识设备单品的ID
 *
 * @param   device_id : 用来存放DeviceID字符串的数组
 * @return  写到device_id[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_GetDeviceID(_OU_ char device_id[DEVICE_ID_MAXLEN]);

/**
 * @brief   获取设备的固件版本字符串
 *
 * @param   version : 用来存放版本字符串的数组
 * @return  写到version[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_GetFirmwareVesion(_OU_ char version[FIRMWARE_VERSION_MAXLEN]);

#define HAL_CID_LEN (64 + 1)
/**
 * @brief   获取唯一的芯片ID字符串
 *
 * @param   cid_str : 存放芯片ID字符串的缓冲区数组
 * @return  指向缓冲区数组的起始地址
 */
char *HAL_GetChipID(_OU_ char cid_str[HAL_CID_LEN]);

typedef struct _hal_wireless_info_t {
    char band;               /*wireless band, 0: wifi 2.4G, 1: wifi 5G, ... */
    unsigned short channel;  /* wireless channel. */
    char rssi;               /* wireless RSSI, range of [-127 : -1]. */
    unsigned char snr;       /* wireless SNR. */
    char mac[6];             /* parent'mac, such as Ap's bssid. */
    float tx_rate;           /* 0.25 : 250Kbps, 1: 1Mbps，2: 2Mbps，5.5: 5.5Mbps，...*/
    float rx_rate;           /* 0.25 : 250Kbps, 1: 1Mbps，2: 2Mbps，5.5: 5.5Mbps，...*/
} hal_wireless_info_t;
/**
 * @brief   获取无线信号信息
 *
 * @param   wireless_info : 存放获取无线信号信息的缓冲区
 * @return  执行结果
 *   0:  success
 *   -1: failure
 */
int HAL_GetWirelessInfo(_OU_ hal_wireless_info_t *wireless_info);

#endif  /* __IMPORT_PRODUCT_H__ */

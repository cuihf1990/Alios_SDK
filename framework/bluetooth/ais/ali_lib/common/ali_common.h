#ifndef _ALI_COMMON_H_
#define _ALI_COMMON_H_

#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#define VERIFY_PARAM_NOT_NULL(p)
#define VERIFY_SUCCESS(e)
#define VERIFY_PARAM_NOT_NULL_VOID(p)
#define VERIFY_MODULE_INITIALIZED_VOID()
#define VERIFY_MODULE_INITIALIZED()
#define VERIFY_SUCCESS_VOID(e)
#define APP_ERROR_CHECK(e)

#define GATT_MTU_SIZE_DEFAULT 23

typedef uint16_t ret_code_t;

/* Error codes. */
#define NRF_SUCCESS                          (0)
#define NRF_ERROR_INVALID_PARAM              (1)
#define NRF_ERROR_DATA_SIZE                  (2)
#define NRF_ERROR_INVALID_STATE              (3)
#define NRF_ERROR_GATT_NOTIFY                (4)
#define NRF_ERROR_GATT_INDICATE              (5)
#define NRF_ERROR_BT_ENABLE                  (6)
#define NRF_ERROR_BT_ADV                     (7)
#define NRF_ERROR_TIMEOUT                    (8)
#define NRF_ERROR_BUSY                       (9)
#define NRF_ERROR_INVALID_DATA               (10)
#define NRF_ERROR_INTERNAL                   (12)
#define NRF_ERROR_INVALID_ADDR               (13)
#define NRF_ERROR_NOT_SUPPORTED              (14)
#define NRF_ERROR_NO_MEM                     (15)
#define NRF_ERROR_FORBIDDEN                  (16)
#define NRF_ERROR_NULL                       (17)
#define NRF_ERROR_INVALID_LENGTH             (18)

#define BLE_CONN_HANDLE_INVALID 0xffff

#if defined (NRF51)
    #define ALI_BLUETOOTH_VER       0x00        /**< Bluetooth version 4.0 (see spec. v1.0.4 ch. 2.2). */
    #define ALI_MAX_SUPPORTED_MTU   23          /**< Maximum supported MTU. */
    #define ALI_CONTEXT_SIZE        394         /**< Context size required, in number of 4-byte words. */
#elif defined (NRF52) || defined(CONFIG_ESP32_WITH_BLE)
    #define ALI_BLUETOOTH_VER       0x01        /**< Bluetooth version 4.2 (see spec. v1.0.4 ch. 2.2). */
    #define ALI_MAX_SUPPORTED_MTU   247         /**< Maximum supported MTU. */
    #define ALI_CONTEXT_SIZE        450         /**< Context size required, in number of 4-byte words. */
#else
    #error No valid target set for ALI_CONTEXT_SIZE.
#endif

#endif

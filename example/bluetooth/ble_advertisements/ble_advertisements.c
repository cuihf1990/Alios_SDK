/**
 ******************************************************************************
 * @file    ble_advertisements.c
 * @author  Jian Zhang
 * @version V1.1.0
 * @date    24-Aug-2016
 * @file    BLE advertisement demonstration via command line interface
 * ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2014 MXCHIP Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ******************************************************************************
 *  BLE Vendor Specific Device
 *
 * Features demonstrated
 *  - GATT database and Device configuration initialization
 *  - Processing user command over UART to control BLE Advertisements.
 *  - Demonstrate advertisement type and filter policy by UART command line.
 *
 *   To demonstrate the demo, work through the following steps.
 *   1. Plug the MiCOKit board into your computer, open a serial terminal
 *   2. Build and download the application (to the MiCOKit board)
 *   3. Type command "advert start" and "advert stop" on serial terminal to control
 *      BLE Advertisements.
 *   4. Turn on your a Central device and connect to this borad. This Central device 
 *      is added to the white list now.
 *   5. Disconnect and re-advert.
 *   6. Type command "policy" on serial terminal to control BLE Advertisements 
 *      filter policy - using white list or not. You can 
 *   7. Now you can observe the filter policy.
 *   8. Type command "wlsize" to get a size of white list.
 *
 ******************************************************************************
 **/

#include "mico.h"
#include "sdpdefs.h"
#include "mico_bt_cfg.h"
#include "mico_bt.h"
#include "mico_bt_peripheral.h"

#include "ble_advertisements.h"

#ifdef USE_MiCOKit_EXT
#include "MiCOKit_EXT/micokit_ext.h"
#endif

#define hello_sensor_log(M, ...) custom_log( "LE ADVERT", M, ##__VA_ARGS__ )

/******************************************************************************
 *                                Constants
 ******************************************************************************/

/******************************************************************************
 *                           Function Prototypes
 ******************************************************************************/

static void hello_sensor_change_advertisements( CLI_ARGS );
static void hello_sensor_change_filter_policy ( CLI_ARGS );
static void hello_sensor_get_whitelist_size   ( CLI_ARGS );

static void hello_sensor_start_advertisements( void );
static void hello_sensor_stop_advertisements( void );

#ifdef USE_MiCOKit_EXT
static const char *pBT_ShortStr( mico_bt_smart_advertising_type_t type );
#endif

static const char *pBT_AdvertStr( mico_bt_smart_advertising_type_t type );

/******************************************************************************
 *                                Structures
 ******************************************************************************/

/* Peripheral auto advertising settings */
static mico_bt_smart_advertising_settings_t advertising_settings = 
{
    .type               = BT_SMART_UNDIRECTED_ADVERTISING,                  /**< Advertising type                                               */
    .use_high_duty      = MICO_TRUE,                                        /**< Using high duty to start advertising                           */
    .high_duty_interval = MICO_BT_CFG_DEFAULT_HIGH_DUTY_ADV_MIN_INTERVAL,   /**< High duty advertising interval                                 */
    .high_duty_duration = 5,                                                /**< High duty advertising duration in seconds (0 for infinite)     */
    .low_duty_interval  = MICO_BT_CFG_DEFAULT_LOW_DUTY_ADV_MIN_INTERVAL,    /**< Low duty advertising interval                                  */
    .low_duty_duration  = 60,                                               /**< Low duty advertising duration in seconds (0 for infinite)      */

    /* Used Directed Advertisement */
    .directed_advertisement_addr_type   = BT_SMART_ADDR_TYPE_PUBLIC,
    .directed_advertisement_addr        = { 11, 22, 33, 44, 55, 66 },
};

/* Peripheral security settings */
static const mico_bt_smart_security_settings_t security_settings = 
{
    .timeout_second                 = 15,
    .io_capabilities                = BT_SMART_IO_DISPLAY_ONLY,
    .authentication_requirements    = BT_SMART_AUTH_REQ_BONDING,
    .oob_authentication             = BT_SMART_OOB_AUTH_NONE,
    .max_encryption_key_size        = 16,
    .master_key_distribution        = BT_SMART_DISTRIBUTE_ALL_KEYS,
    .slave_key_distribution         = BT_SMART_DISTRIBUTE_ALL_KEYS,
};

struct cli_command hello_sensor_send_message_cmd[] = 
{
    {   
        "advert",
        "<start/stop> <0:Generic undirected/1:Discoverable advertising/2:Directed/3:Non-connectable Undirected> <0:Low Duty/1:High Duty>",
        hello_sensor_change_advertisements
    },
    {
        "policy",
        "<0:Not use white list/1:filter all connection request and whitelist scan reuqest/2:filter whitelist connection request and all scan request/3:filter whitelist connection request and whitelist scan request>",
        hello_sensor_change_filter_policy
    },
    {
        "wlsize",
        "Get the number of devices in the white list",
        hello_sensor_get_whitelist_size
    }
};

#define USER_CLI_COMMAND_SIZE   ( sizeof( hello_sensor_send_message_cmd ) / sizeof( hello_sensor_send_message_cmd[0] ) )

/******************************************************************************
 *                                Variables Definitions
 ******************************************************************************/

/* Initialized attribute value */
static uint8_t hello_sensor_appearance_name[2] = { BIT16_TO_8( APPEARANCE_GENERIC_TAG ) };
static uint8_t hello_sensor_char_system_id_value[] = { 0xbb, 0xb8, 0xa1, 0x80, 0x5f, 0x9f, 0x91,
    0x71 };

/* MICO BT Smart peripheral connection controller */
static mico_bt_peripheral_socket_t peripheral_socket;

/******************************************************************************
 *                                GATT DATABASE
 ******************************************************************************/

const uint8_t hello_sensor_gatt_database[] = 
{
    /* Declare mandatory GATT service */
    PRIMARY_SERVICE_UUID16( HDLS_GENERIC_ATTRIBUTE, UUID_SERVCLASS_GATT_SERVER ),

    CHARACTERISTIC_UUID16( HDLC_GENERIC_ATTRIBUTE_SERVICE_CHANGED,
                           HDLC_GENERIC_ATTRIBUTE_SERVICE_CHANGED_VALUE,
                           GATT_UUID_GATT_SRV_CHGD,
                           LEGATTDB_CHAR_PROP_INDICATE,
                           LEGATTDB_PERM_NONE ),

    /* Declare mandatory GAP service. Device Name and Appearance are mandatory
     * characteristics of GAP service                                        */
    PRIMARY_SERVICE_UUID16( HDLS_GENERIC_ACCESS,
                            UUID_SERVCLASS_GAP_SERVER ),

    /* Declare mandatory GAP service characteristic: Dev Name */
    CHARACTERISTIC_UUID16(
                           HDLC_GENERIC_ACCESS_DEVICE_NAME,
                           HDLC_GENERIC_ACCESS_DEVICE_NAME_VALUE,
                           GATT_UUID_GAP_DEVICE_NAME,
                           LEGATTDB_CHAR_PROP_READ,
                           LEGATTDB_PERM_READABLE ),

    /* Declare mandatory GAP service characteristic: Appearance */
    CHARACTERISTIC_UUID16( HDLC_GENERIC_ACCESS_APPEARANCE,
                           HDLC_GENERIC_ACCESS_APPEARANCE_VALUE,
                           GATT_UUID_GAP_ICON,
                           LEGATTDB_CHAR_PROP_READ,
                           LEGATTDB_PERM_READABLE ),

    /* Declare Device info service */
    PRIMARY_SERVICE_UUID16( HDLS_DEV_INFO,
                            UUID_SERVCLASS_DEVICE_INFO ),

    /* Handle 0x4e: characteristic Manufacturer Name */
    CHARACTERISTIC_UUID16( HDLC_DEV_INFO_MFR_NAME,
                           HDLC_DEV_INFO_MFR_NAME_VALUE,
                           GATT_UUID_MANU_NAME,
                           LEGATTDB_CHAR_PROP_READ,
                           LEGATTDB_PERM_READABLE ),

    /* Handle 0x50: characteristic Model Number */
    CHARACTERISTIC_UUID16( HDLC_DEV_INFO_MODEL_NUM,
                           HDLC_DEV_INFO_MODEL_NUM_VALUE,
                           GATT_UUID_MODEL_NUMBER_STR,
                           LEGATTDB_CHAR_PROP_READ,
                           LEGATTDB_PERM_READABLE ),

    /* Handle 0x52: characteristic System ID */
    CHARACTERISTIC_UUID16( HDLC_DEV_INFO_SYSTEM_ID,
                           HDLC_DEV_INFO_SYSTEM_ID_VALUE,
                           GATT_UUID_SYSTEM_ID,
                           LEGATTDB_CHAR_PROP_READ,
                           LEGATTDB_PERM_READABLE ),
};

/******************************************************************************
 *                          Function Definitions
 ******************************************************************************/

static void hello_sensor_create_attribute_db( void );

/* Peripheral connection handlers */
static OSStatus connection_handler( mico_bt_peripheral_socket_t *socket );
static OSStatus disconnection_handler( mico_bt_peripheral_socket_t *socket );

static OSStatus hello_sensor_set_advertisement_data( void );
static OSStatus advertisement_complete_handle( void *arg );

/* 
 * Entry point to the application.
 */

int application_start( void )
{
    OSStatus err = kNoErr;

#ifdef USE_MiCOKit_EXT
    OLED_ShowString( OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_2, "BLE Advertising" );
#endif 

    /* Initialize MiCO bluetooth Framework */
    err = mico_bt_init( MICO_BT_HCI_MODE, "MK3239 Advert", 0, 1 );
    require_noerr_string( err, exit, "Error initialising MiCO Bluetooth Framework" );

    /* Initialize MiCO bluetooth peripheral */
    mico_bt_peripheral_init( &peripheral_socket, &security_settings, connection_handler,
                             disconnection_handler, NULL );

    /* Build BT Stack layer GATT database */
    mico_bt_gatt_db_init( hello_sensor_gatt_database, sizeof(hello_sensor_gatt_database) );

    /* Build BT Application Layer GATT database */
    hello_sensor_create_attribute_db( );

    /* Set the advertising parameters and make the device discoverable */
    hello_sensor_set_advertisement_data( );

    /* Start advertising */
    hello_sensor_start_advertisements( );

#ifdef MICO_CLI_ENABLE 
    cli_register_commands( hello_sensor_send_message_cmd, USER_CLI_COMMAND_SIZE );
#endif 

exit:
    mico_rtos_delete_thread( NULL );
    return err;
}

/***********************************************************************************************************
 * Handler or Callback function
 */

/* This function is invoked when advertisements changed. */
static OSStatus advertisement_complete_handle( void *arg )
{
    mico_bt_peripheral_socket_status_t status;

    UNUSED_PARAMETER( arg );

    hello_sensor_log( "Advertisements Stopped successfully!" );
    hello_sensor_log( "" );

    mico_bt_peripheral_get_socket_status( &peripheral_socket, &status );
    if ( status == PERIPHERAL_SOCKET_DISCONNECTED )
    {
        hello_sensor_start_advertisements( );
    }
    return kNoErr;
}

/* Peripheral connection handlers */
static OSStatus connection_handler( mico_bt_peripheral_socket_t *socket )
{
    hello_sensor_stop_advertisements( );

    hello_sensor_log( "Connection established! id: 0x%x", socket->connection_handle );
    hello_sensor_log( "" );

#ifdef USE_MiCOKit_EXT
    OLED_ShowString( OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_3, "     STOP!!     " );
    OLED_ShowString( OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_4, "   Connected    " );
#endif 

    /* Add device to whitelist */
    mico_bt_peripheral_update_advertisements_white_list( MICO_TRUE, socket->remote_device.address);

    return kNoErr;
}

static OSStatus disconnection_handler( mico_bt_peripheral_socket_t *socket )
{
    hello_sensor_log( "Connection disconnected!\r\n" );
    hello_sensor_log( "" );

    /* Start advertising */
    hello_sensor_start_advertisements( );

    return kNoErr;
}

/**************************************************************************************************************
 * Command line handle 
 */

/* This function will be triggered by command "advert" on command line */
static void hello_sensor_change_advertisements( char *pcWriteBuffer, int xWriteBufferLen, int argc,
                                                char **argv )
{
    mico_bt_smart_advertising_type_t type = BT_SMART_UNDIRECTED_ADVERTISING; //default
    mico_bool_t duty = MICO_TRUE; //default

    require( argc >= 2, err );

    /* Change current advertisement type and state. */
    if ( (strcmp( argv[1], "stop" ) == 0) )
    {
        /* Stop advertisement */
        hello_sensor_stop_advertisements( );

    } 
    else if ( strcmp( argv[1], "start" ) == 0 )
    {

        /* Check the second option parameter -- advertisement type */
        if ( argc >= 3 )
        {
            require( strlen( argv[2] ) == 1, err );
            type = argv[2][0] - '0';
            require( type <= BT_SMART_NON_CONNECTABLE_UNDIRECTED_ADVERTISING, err );
        }

        /* Check the third option parameter -- advertisement duty */
        if ( argc >= 4 )
        {
            require( strlen( argv[3] ) == 1, err );
            duty = argv[3][0] - '0';
            require( duty == MICO_TRUE || duty == MICO_FALSE, err );
        }

        switch ( type )
        {
            case BT_SMART_UNDIRECTED_ADVERTISING:
                advertising_settings.high_duty_interval = MICO_BT_CFG_DEFAULT_HIGH_DUTY_ADV_MIN_INTERVAL;
                advertising_settings.low_duty_interval  = MICO_BT_CFG_DEFAULT_LOW_DUTY_ADV_MAX_INTERVAL;
                break;
            case BT_SMART_DISCOVERABLE_ADVERTISING:
                case BT_SMART_NON_CONNECTABLE_UNDIRECTED_ADVERTISING:
                advertising_settings.high_duty_interval = MICO_BT_CFG_DEFAULT_HIGH_DUTY_NONCONN_ADV_MIN_INTERVAL;
                advertising_settings.low_duty_interval  = MICO_BT_CFG_DEFAULT_LOW_DUTY_NONCONN_ADV_MAX_INTERVAL;
                break;
            case BT_SMART_DIRECTED_ADVERTISING:
                advertising_settings.high_duty_interval = MICO_BT_CFG_DEFAULT_HIGH_DUTY_DIRECTED_ADV_MIN_INTERVAL;
                advertising_settings.low_duty_interval  = MICO_BT_CFG_DEFAULT_LOW_DUTY_DIRECTED_ADV_MIN_INTERVAL;
                break;
            default:
                break;
        }

        /* Start Advertisement */
        advertising_settings.use_high_duty = duty;
        advertising_settings.type = type;

        hello_sensor_start_advertisements( );
    } 
    else
    {
err:
        hello_sensor_log( "CLI: Unknown command or Unknown arguments" );
        hello_sensor_log( "" );
    }
}

/* This function will be triggered by command "policy" on command line */
static void hello_sensor_change_filter_policy( char *pcWriteBuffer, int xWriteBufferLen, int argc,
                                               char **argv )
{
    mico_bt_peripheral_adv_filter_policy_t policy;

    require( argc == 2, err );

    switch ( argv[1][0] ) 
    {
        case '0': // PERIPHERAL_ADVERT_FILTER_ALL_CONNECTION_REQ_ALL_SCAN_REQ
            hello_sensor_log( "PERIPHERAL_ADVERT_FILTER_ALL_CONNECTION_REQ_ALL_SCAN_REQ" );
            policy = PERIPHERAL_ADVERT_FILTER_ALL_CONNECTION_REQ_ALL_SCAN_REQ;
            break;
        case '1': // PERIPHERAL_ADVERT_FILTER_ALL_CONNECTION_REQ_WHITELIST_SCAN_REQ
            hello_sensor_log(  "PERIPHERAL_ADVERT_FILTER_ALL_CONNECTION_REQ_WHITELIST_SCAN_REQ" );
            policy = PERIPHERAL_ADVERT_FILTER_ALL_CONNECTION_REQ_WHITELIST_SCAN_REQ;
            break;
        case '2': // PERIPHERAL_ADVERT_FILTER_WHITELIST_CONNECTION_REQ_ALL_SCAN_REQ
            hello_sensor_log( "PERIPHERAL_ADVERT_FILTER_WHITELIST_CONNECTION_REQ_ALL_SCAN_REQ" );
            policy = PERIPHERAL_ADVERT_FILTER_WHITELIST_CONNECTION_REQ_ALL_SCAN_REQ;
            break;
        case '3': // PERIPHERAL_ADVERT_FILTER_WHITELIST_CONNECTION_REQ_WHITELIST_SCAN_REQ
            hello_sensor_log( "PERIPHERAL_ADVERT_FILTER_WHITELIST_CONNECTION_REQ_WHITELIST_SCAN_REQ" );
            policy = PERIPHERAL_ADVERT_FILTER_WHITELIST_CONNECTION_REQ_WHITELIST_SCAN_REQ;
            break;
        default:
err:
            hello_sensor_log( "hello_sensor_change_filter_policy: Unknown command or Unknown arguments" );
            return;
    }

    if ( mico_bt_peripheral_set_advertisements_filter_policy( policy ) != kNoErr ) 
    {
        hello_sensor_log( "update advertisement filter policy unsuccessfully!!" );
    } 
    else 
    {
        hello_sensor_log( "update advertisement filter policy successfully!!" );
    }
}

/* This function will be triggered by command "wlsize" on command line */
static void hello_sensor_get_whitelist_size( char *pcWriteBuffer, int xWriteBufferLen, int argc,
                                             char **argv )
{
    uint8_t size = 0;

    require( argc == 2, err );

    switch ( argv[1][0] ) 
    {
        case '1':
            mico_bt_peripheral_get_advertisements_white_list_size( &size );
            snprintf( pcWriteBuffer, xWriteBufferLen, "size: %d", size );
            break;

        case '2':
            mico_bt_get_whitelist_capability( &size );
            snprintf( pcWriteBuffer, xWriteBufferLen, "capability: %d", size );
            break;

        default:
err:
            hello_sensor_log( "hello_sensor_get_whitelist_size: Unknown command or Unknown arguments" );
            return;
    } 
}

/*
 * Local function 
 */

/* Set advertisement data and scanning response data */
static OSStatus hello_sensor_set_advertisement_data( void )
{
    OSStatus err = kNoErr;

    uint16_t uuid[1] = { UUID_SERVCLASS_DEVICE_INFO };
    mico_bt_ble_advert_data_t adv_data;

    mico_bt_ble_service_t advert_services_16 = 
    {
        .num_service = 1,
        .list_cmpl  = true,
        .p_uuid     = uuid,
    };

    mico_bt_ble_manu_t advert_manufacture = 
    {
        .len    = strlen( "MXCHIP" ),
        .p_val  = (uint8_t *) "MXCHIP",
    };

    adv_data.flag = BTM_BLE_GENERAL_DISCOVERABLE_FLAG | BTM_BLE_BREDR_NOT_SUPPORTED;
    adv_data.p_services = &advert_services_16;
    adv_data.p_manu = &advert_manufacture;

    mico_bt_ble_set_advertisement_data(
        BTM_BLE_ADVERT_BIT_DEV_NAME | BTM_BLE_ADVERT_BIT_SERVICE | BTM_BLE_ADVERT_BIT_FLAGS,
        &adv_data );
    mico_bt_ble_set_scan_response_data( BTM_BLE_ADVERT_BIT_MANUFACTURER, &adv_data );

    return err;
}

/* Display Advertisement Arguments */
static void hello_sensor_start_advertisements( void )
{
#ifdef USE_MiCOKit_EXT 
    char display_str[20];
#endif 

    mico_bt_dev_status_t status;
    mico_bt_peripheral_socket_status_t socket_status = PERIPHERAL_SOCKET_CONNECTED;

    /* Check current connection state. */
    mico_bt_peripheral_get_socket_status( &peripheral_socket, &socket_status );
    if ( socket_status != PERIPHERAL_SOCKET_DISCONNECTED )
    {
        hello_sensor_log( "Advertisements Started unsuccessfully! Existed connection" );
        hello_sensor_log( "" );
        return;
    }

    /* Start advertisements */
    status = mico_bt_peripheral_start_advertisements( &advertising_settings,
                                                      advertisement_complete_handle );

    /* MiCOKit LCD Display */
#ifdef USE_MiCOKit_EXT
    sprintf( display_str, "Type: %s", pBT_ShortStr( advertising_settings.type ) );
    OLED_ShowString( OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_3, display_str );
    sprintf( display_str, "Duty: %s  ", advertising_settings.use_high_duty == MICO_TRUE ? "High" : "Low" );
    OLED_ShowString( OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_4, display_str );
#endif 

    /* LOG */
    hello_sensor_log("");
    if ( status == MICO_BT_SUCCESS )
    {
        hello_sensor_log( "Advertisements Started successfully! Arguments: " );
    } 
    else if ( status == MICO_BT_ILLEGAL_VALUE )
    {
        hello_sensor_log( "Advertisements Started unsuccessfully! Illegal value: " );
    } 
    else
    {
        hello_sensor_log( "Advertisements Started unsuccessfully! Unknown error: " );
    }
    hello_sensor_log( "" );
    hello_sensor_log( "\t\t type              : %s", pBT_AdvertStr( advertising_settings.type ) );
    hello_sensor_log( "\t\t duty              : %s",
        advertising_settings.use_high_duty == MICO_TRUE ? "High duty" : "Low duty" );
    hello_sensor_log( "\t\t high_duty_interval: %-4d (slots, 0.625ms)",
        advertising_settings.high_duty_interval );
    hello_sensor_log( "\t\t high_duty_duration: %-4d (seconds)",
        advertising_settings.high_duty_duration );
    hello_sensor_log( "\t\t low_duty_interval : %-4d (slots, 0.625ms)",
        advertising_settings.low_duty_interval );
    hello_sensor_log( "\t\t low_duty_duration : %-4d (seconds)",
        advertising_settings.low_duty_duration );
    hello_sensor_log( "" );
}

/* Stop Advertisement Arguments */
static void hello_sensor_stop_advertisements( void )
{
    mico_bt_dev_status_t status;

    /* MiCOKit LCD Display */
#ifdef USE_MiCOKit_EXT
    OLED_ShowString( OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_3, "     STOP!!     " );
    OLED_ShowString( OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_4, "                " );
#endif 

    status = mico_bt_peripheral_stop_advertisements( );
    if ( status == MICO_BT_SUCCESS )
    {
        hello_sensor_log( "Advertisements Stopped successfully!" );
    } 
    else
    {
        hello_sensor_log( "Advertisements Stopped unsuccessfully!" );
    }
    hello_sensor_log( "" );
}

/* Add attribute value for some characteristic attribute */
static void hello_sensor_create_attribute_db( void )
{
    extern mico_bt_cfg_settings_t mico_bt_cfg_settings;

    /*
     * Create BLE GATT external value database
     */

    // Primary service 'Generic Attribute'
    mico_bt_peripheral_ext_attribute_add( HDLC_GENERIC_ATTRIBUTE_SERVICE_CHANGED_VALUE,
                                          0,
                                          NULL, NULL );

    // Primary service 'Generic Access'
    mico_bt_peripheral_ext_attribute_add( HDLC_GENERIC_ACCESS_DEVICE_NAME_VALUE,
                                          strlen( (char *) mico_bt_cfg_settings.device_name ),
                                          mico_bt_cfg_settings.device_name,
                                          NULL );
    mico_bt_peripheral_ext_attribute_add( HDLC_GENERIC_ACCESS_APPEARANCE_VALUE,
                                          sizeof(hello_sensor_appearance_name),
                                          hello_sensor_appearance_name,
                                          NULL );

    // Primary service 'Device info'
    mico_bt_peripheral_ext_attribute_add( HDLC_DEV_INFO_MFR_NAME_VALUE,
                                          strlen( (char *) MANUFACTURER ),
                                          (uint8_t *) MANUFACTURER,
                                          NULL );
    mico_bt_peripheral_ext_attribute_add( HDLC_DEV_INFO_MODEL_NUM_VALUE,
                                          strlen( (char *) MODEL ),
                                          (uint8_t *) MODEL,
                                          NULL );
    mico_bt_peripheral_ext_attribute_add( HDLC_DEV_INFO_SYSTEM_ID_VALUE,
                                          sizeof(hello_sensor_char_system_id_value),
                                          hello_sensor_char_system_id_value,
                                          NULL );
}

#ifdef USE_MiCOKit_EXT

/* Used in OLED Display */
static const char *pBT_ShortStr(mico_bt_smart_advertising_type_t type)
{
    switch ( type )
    {
        case BT_SMART_UNDIRECTED_ADVERTISING:
            return "UNDIRECT";

        case BT_SMART_DISCOVERABLE_ADVERTISING:
            return "DISCOVER";

        case BT_SMART_DIRECTED_ADVERTISING:
            return "DIRECTED";

        case BT_SMART_NON_CONNECTABLE_UNDIRECTED_ADVERTISING:
            return "NON-CONN";

        default:
            return "Unknown ";
    }
}

#endif 

/* Convert advertising type to friendly information. */
static const char *pBT_AdvertStr( mico_bt_smart_advertising_type_t type )
{
    switch ( type )
    {
        case BT_SMART_UNDIRECTED_ADVERTISING:
            return "Generic undirected advertising";

        case BT_SMART_DISCOVERABLE_ADVERTISING:
            return "Discoverable advertising";

        case BT_SMART_DIRECTED_ADVERTISING:
            return "Directed advertising";

        case BT_SMART_NON_CONNECTABLE_UNDIRECTED_ADVERTISING:
            return "Non-connectable Undirected advertising";

        default:
            return "Unknown advertising type";
    }
}


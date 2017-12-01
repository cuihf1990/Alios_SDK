/**
 ******************************************************************************
 * @file    ble_hello_center.c
 * @author  William Xu
 * @version V1.0.0
 * @date    30-Apr-2016
 * @file    Demonstrate a BLE center device function
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
 * BLE Vendor Specific Central Device
 *
 * Features demonstrated
 *  - Connect "MiCOKit EMW3238" device and create Client GATT database cache
 *  - Sending data to the hello sensor demo device to control led color
 *  - Read notification and indication from hello sensor demo device
 *  - Create a hello peripheral for device information
 *
 * To demonstrate the demo, work through the following steps.
 * 1. Plug the MiCOKit board into your computer
 * 2. Build and download the application (to the MiCOKit board)
 * 3. On application start the device acts as a GATT client and scan for "MiCOKit EMW3238" devices
 * 4. Plug the another MiCOKit board into your computer
 * 5. Build and download the hello sensor demo(demos/bluetooth/ble_hello_sensor) to another MiCOKit board
 * 6. On hello sensor demo application, start the device acts as a GATT server and advertises itself as "MiCOKit EMW3238" devices
 * 7. Center application will connect to hello sensor, start pairing and establish GATT database cache
 * 8. Send led control characteristic data to hello sensor
 * 9. Read indicate and notify from hello sensor
 ******************************************************************************
 **/

#include <string.h>
#include <stdio.h>

#include "mico.h"
#include "mico_bt.h"
#include "mico_bt_cfg.h"
#include "mico_bt_smart_interface.h"
#include "mico_bt_smartbridge.h"
#include "mico_bt_smartbridge_gatt.h"

#include "StringUtils.h"
#include "LinkListUtils.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define mico_bt_app_log(M, ...) custom_log("BT Demo", M, ##__VA_ARGS__)

/******************************************************
 *                    Constants
 ******************************************************/

#define MAX_CONCURRENT_CONNECTIONS          (10)

#define MAX_ATTRIBUTE_CACHE_COUNTS          (10)

#define OUT_OF_BAND_AUTHENTICATION          BT_SMART_OOB_AUTH_NONE

#define AUTHENTICATION_REQUIREMENTS         BT_SMART_AUTH_REQ_BONDING

/* UUID value of the Hello Sensor Service */
#define UUID_HELLO_SERVICE                  0x23, 0x20, 0x56, 0x7c, 0x05, 0xcf, 0x6e, 0xb4, 0xc3, 0x41, 0x77, 0x28, 0x51, 0x82, 0x7e, 0x1b

/* UUID value of the Hello Sensor Characteristic, Value Notification */
#define UUID_HELLO_CHARACTERISTIC_NOTIFY    0x26, 0xf6, 0x69, 0x91, 0x68, 0xee, 0xc2, 0xbe, 0x44, 0x4d, 0xb9, 0x5c, 0x3f, 0x2d, 0xc3, 0x8a

/* UUID value of the Hello Sensor Characteristic, Color Configuration */
#define UUID_HELLO_CHARACTERISTIC_COLOR     0x1a, 0x89, 0x07, 0x4a, 0x2f, 0x3b, 0x7e, 0xa6, 0x81, 0x44, 0x3f, 0xf9, 0xa8, 0xf2, 0x9b, 0x5e


#define LENGTH_HELLO_CHARACTERISTIC_COLOR   (1)


/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

 /* Device info ready to connect */
typedef struct
{
    linked_list_node_t          this_node;     /* Linked-list node of this deivice */
    mico_bt_smart_device_t      device;        /* Remote BT device */
} connecting_device_t;

/******************************************************
 *                    Structures
 ******************************************************/

/* SmartBridge security settings */
const mico_bt_smart_security_settings_t security_settings =
{
    .timeout_second              = 15,
    .io_capabilities             = BT_SMART_IO_DISPLAY_ONLY,
    .authentication_requirements = AUTHENTICATION_REQUIREMENTS,
    .oob_authentication          = OUT_OF_BAND_AUTHENTICATION,
    .max_encryption_key_size     = 16,
    .master_key_distribution     = BT_SMART_DISTRIBUTE_ENCRYPTION_AND_SIGN_KEYS,
    .slave_key_distribution      = BT_SMART_DISTRIBUTE_ALL_KEYS,
};

/* SmartBridge connection settings */
static const mico_bt_smart_connection_settings_t connection_settings =
{
    .timeout_second                = 10,
    .filter_policy                 = FILTER_POLICY_NONE,
    .interval_min                  = MICO_BT_CFG_DEFAULT_CONN_MIN_INTERVAL,
    .interval_max                  = MICO_BT_CFG_DEFAULT_CONN_MAX_INTERVAL,
    .latency                       = MICO_BT_CFG_DEFAULT_CONN_LATENCY,
    .supervision_timeout           = MICO_BT_CFG_DEFAULT_CONN_SUPERVISION_TIMEOUT,
    .ce_length_min                 = 0,
    .ce_length_max                 = 0,
    .attribute_protocol_timeout_ms = 10000,
};

/* SmartBridge auto scan settings */
static const mico_bt_smart_scan_settings_t scan_settings =
{
    .type              = BT_SMART_PASSIVE_SCAN,
    .filter_policy     = FILTER_POLICY_NONE,
    .filter_duplicates = DUPLICATES_FILTER_ENABLED,
    .interval          = 512,
    .window            = 48,
    .duration_second   = 10,
};

/******************************************************
 *               Static Function Declarations
 ******************************************************/
static OSStatus connect_handler                 ( void* arg );
static OSStatus disconnection_handler           ( mico_bt_smartbridge_socket_t* socket );
static OSStatus notification_handler            ( mico_bt_smartbridge_socket_t* socket, uint16_t attribute_handle );
static OSStatus scan_complete_handler           ( void *arg );
static OSStatus scan_result_handler             ( const mico_bt_smart_advertising_report_t* result );
static OSStatus sensor_trigger_handler_t        ( void *arg );

/* Remote device list management */
static OSStatus connect_list_init   ( void );
static OSStatus connect_list_add    ( mico_bt_smart_device_t  remote_device );
static OSStatus connect_list_get    ( mico_bt_smart_device_t** address );
static OSStatus connect_list_remove ( mico_bt_smart_device_t* device );

/******************************************************
 *               Function Declarations
 ******************************************************/

extern void ble_commands_init ( mico_bt_smartbridge_socket_t *socket, uint8_t socket_count );
extern OSStatus ble_hello_peripheral_init( void );

/* Wlan test thread, connect to a TCP server and echo */
extern OSStatus wlan_coexist_test_start( void );

/******************************************************
 *               Variable Definitions
 ******************************************************/

const char desired_peer_device[] = "MiCOKit-3239";      /* Name of desired peer device to connect to */
const mico_bt_uuid_t hello_service_uuid = { .len = LEN_UUID_128, .uu.uuid128 = { UUID_HELLO_SERVICE } };  /* Service for operation */
const mico_bt_uuid_t hello_color_uuid = { .len = LEN_UUID_128, .uu.uuid128 = { UUID_HELLO_CHARACTERISTIC_COLOR } }; /* Characteristic for MiCOKit led control */

static mico_bt_smartbridge_socket_t  smartbridge_socket[MAX_CONCURRENT_CONNECTIONS]; /* Sockets to manage connections */
static uint32_t hello_color_value_handle[MAX_CONCURRENT_CONNECTIONS]; /* Handles to manage characteristic for each connection */

static mico_worker_thread_t  hello_center_worker_thread; /* Worker thread to manage connection events */

static mico_worker_thread_t  hello_sensor_color_toggle_worker_thread;  /* Worker thread to manage timed event for MiCOKit led control */
static mico_timed_event_t   hello_sensor_color_toggle_event;

static uint8_t color_idx = 0; /* Currnet color index for every connected MiCOKit */
static linked_list_t connecting_device_list;

/******************************************************
 *               Function Definitions
 ******************************************************/
int application_start(void)
{
    OSStatus err = kNoErr;
    uint32_t a;

    /* start a wlan function */
#ifdef WLAN_COEXIST_TEST
    err = wlan_coexist_test_start();
    require_noerr( err, exit );
#endif

    /* Initialise MiCO Bluetooth Framework */
    err = mico_bt_init( MICO_BT_HCI_MODE, "SmartBridge Device", MAX_CONCURRENT_CONNECTIONS, 1 );  //Client + server connections
    require_noerr_string( err, exit, "Error initialising MiCO Bluetooth Framework" );   

    err = ble_hello_peripheral_init( );
    require_noerr_string( err, exit, "Failed to init ble hello peripheral demo!" );   

    /* Initialise MiCO SmartBridge */
    err = mico_bt_smartbridge_init( MAX_CONCURRENT_CONNECTIONS );
    require_noerr( err, exit );

    mico_bt_app_log( "Initialize success");

    /* Enable Attribute Cache and set maximum number of caches */
    err = mico_bt_smartbridge_enable_attribute_cache( MAX_ATTRIBUTE_CACHE_COUNTS, (mico_bt_uuid_t *)&hello_service_uuid, 1 );
    require_noerr_string( err, exit, "Failed to enable ATT Cache" );

    /* Create all sockets and make them ready to connect. A socket can connect and disconnect multiple times. */
    for ( a = 0; a < MAX_CONCURRENT_CONNECTIONS; a++ )
    {
        mico_bt_smartbridge_create_socket( &smartbridge_socket[a] );
    }

    /* Create a worker thread for making a connection and control event */
    mico_rtos_create_worker_thread( &hello_center_worker_thread, MICO_APPLICATION_PRIORITY, 2048, 1 );

    mico_rtos_create_worker_thread( &hello_sensor_color_toggle_worker_thread, MICO_APPLICATION_PRIORITY, 1024, 1 );

    mico_rtos_register_timed_event( &hello_sensor_color_toggle_event, &hello_sensor_color_toggle_worker_thread, sensor_trigger_handler_t, 1000, NULL );

    mico_bt_app_log("Scanning for %s...\n", desired_peer_device);

    err = connect_list_init( );
    require_noerr( err, exit );

    mico_bt_smartbridge_start_scan( &scan_settings, scan_complete_handler, scan_result_handler );

#ifdef MICO_CLI_ENABLE
    /* Register ble helper commands to cli interface */
    ble_commands_init( smartbridge_socket, MAX_CONCURRENT_CONNECTIONS );
#endif

exit:
  mico_rtos_delete_thread( NULL );
  return err;
}

OSStatus sensor_trigger_handler_t(void *arg)
{
    UNUSED_PARAMETER(arg);
    uint32_t i;
    mico_bt_smartbridge_socket_status_t status;
    OSStatus err = kNoErr;

    mico_bt_smart_attribute_t *characteristic_value = NULL;

    mico_bt_smart_attribute_create( &characteristic_value, MICO_ATTRIBUTE_TYPE_CHARACTERISTIC_VALUE, LENGTH_HELLO_CHARACTERISTIC_COLOR );

    for ( i = 0; i < MAX_CONCURRENT_CONNECTIONS; i++ )
    {
        mico_bt_smartbridge_get_socket_status( &smartbridge_socket[i], &status );

        if ( status == SMARTBRIDGE_SOCKET_CONNECTED && smartbridge_socket[i].att_cache != NULL )
        {
            err = mico_bt_smartbridge_get_attribute_cache_by_handle( &smartbridge_socket[i], hello_color_value_handle[i], characteristic_value, ATTR_CHARACTERISTIC_VALUE_SIZE( LENGTH_HELLO_CHARACTERISTIC_COLOR ) );
            if( err == kNoErr )
            {
                memcpy( characteristic_value->value.value,  &color_idx, 1);
                mico_bt_smartbridge_write_attribute_cache_characteristic_value( &smartbridge_socket[i], characteristic_value );
            }
            else if( err == MICO_BT_DISCOVER_IN_PROGRESS )
            {
                mico_bt_app_log( "ATT cache discover is in progress");
            }
            else
            {
                mico_bt_app_log( "Hello color characteristic not found, disconnect");
                mico_bt_smartbridge_disconnect( &smartbridge_socket[i], FALSE );
            }
        }
    }

    color_idx ++;

    if( characteristic_value != NULL )
        mico_bt_smart_attribute_delete( characteristic_value );

    return kNoErr;
}

/* Scan complete handler. Scan complete event reported via this callback.
 * It runs on the MICO_NETWORKING_WORKER_THREAD context.
 */
OSStatus scan_complete_handler( void *arg )
{
    UNUSED_PARAMETER(arg);
    /* Scan complete, start a new scan. Donot use a infinit scan, it may store every result in RAM. */
    mico_bt_smartbridge_start_scan( &scan_settings, scan_complete_handler, scan_result_handler );
    return kNoErr;
}


OSStatus scan_result_handler( const mico_bt_smart_advertising_report_t* scan_result )
{
    /* If connection not initiated yet, and this device has the name we are looking for, then initiate teh connection */
    if ( memcmp( scan_result->remote_device.name, desired_peer_device, strlen(desired_peer_device)) == 0)
    {        
        connect_list_add( scan_result->remote_device );
        mico_rtos_send_asynchronous_event( &hello_center_worker_thread, connect_handler, (void*)scan_result );
    }
    return kNoErr;
}

/* Connect handler. Smartbridge connect is executed in this callback.
 * It runs on the connect_worker_thread context
 */
static OSStatus connect_handler( void* arg )
{
    UNUSED_PARAMETER(arg);
    uint32_t i;
    OSStatus err = kNoErr;
    mico_bt_smartbridge_socket_status_t status;

    uint8_t attribute_buffer[100];
    mico_bt_smart_attribute_t *attribute = (mico_bt_smart_attribute_t *)attribute_buffer;
    char* bt_addr_str = NULL;

    mico_bt_smart_device_t *remote_device;

    while( connect_list_get( &remote_device ) == kNoErr )
    {
        bt_addr_str = DataToHexStringWithColons( (uint8_t *)remote_device->address, 6 );
        mico_bt_app_log("Opening GATT connection to [%s] (addr type=%s)...", bt_addr_str,
                (remote_device->address_type == BT_SMART_ADDR_TYPE_PUBLIC ) ? "Public":"Random");
        free( bt_addr_str );

        /* Iterate all sockets and look for the first available socket */
        for ( i = 0; i < MAX_CONCURRENT_CONNECTIONS; i++ )
        {
            mico_bt_smartbridge_get_socket_status( &smartbridge_socket[i], &status );

            /* A free socket is found. Use it to connect */
            if ( status == SMARTBRIDGE_SOCKET_DISCONNECTED )
            {
                /* If there is a previously stored device, then connect to it */
                if (security_settings.authentication_requirements != BT_SMART_AUTH_REQ_NONE)
                {
                    if ( mico_bt_dev_find_bonded_device( (uint8_t *)remote_device->address ) == MICO_FALSE )
                    {
                        mico_bt_app_log( "Bond info not found. Initiate pairing request." );
                        mico_bt_smartbridge_enable_pairing( &smartbridge_socket[i], &security_settings, NULL );  
                    }
                    else
                    {
                        mico_bt_app_log( "Bond info found. Encrypt use bond info." );
                        mico_bt_smartbridge_set_bond_info( &smartbridge_socket[i], &security_settings, NULL );
                    }
                }

                /* Connect */
                err = mico_bt_smartbridge_connect( &smartbridge_socket[i], remote_device, &connection_settings, disconnection_handler, notification_handler );
                require_noerr_string( err, exit, "Hello sensor connect failed." );
                mico_bt_app_log( "Smartbridge connection established." );

                /* Find service */
                err = mico_bt_smartbridge_get_service_from_attribute_cache_by_uuid( &smartbridge_socket[i], &hello_service_uuid, 0x0, 0xFFFF, attribute, 100 );
                require_noerr_action_string( err, exit, mico_bt_smartbridge_disconnect( &smartbridge_socket[i], FALSE ), "Hello service not found, disconnect." );
                
                /* Find characteristic, and save characteristic value handle */
                err = mico_bt_smartbridge_get_characteritics_from_attribute_cache_by_uuid( &smartbridge_socket[i], &hello_color_uuid, attribute->value.service.start_handle, attribute->value.service.end_handle, (mico_bt_smart_attribute_t *)attribute_buffer, 100 );
                if( err != kNoErr )
                {
                    mico_bt_app_log( "Hello color characteristic not found, remove att cache and disconnect." );
                    mico_bt_smartbridge_remove_attribute_cache( &smartbridge_socket[i] );
                    mico_bt_smartbridge_disconnect( &smartbridge_socket[i], FALSE );
                }

                hello_color_value_handle[i] = attribute->value.characteristic.value_handle;

                /* Enable Attribute Cache notification */
                //err = mico_bt_smartbridge_enable_attribute_cache_notification( &smartbridge_socket[i], MICO_TRUE );
                //require_noerr_string( err, exit, "Attribute cache notification failed." );

                goto exit;
            }
        }
        exit:
            connect_list_remove( remote_device );
            continue;
    }
    return err;
}

/* Disconnection handler. Disconnection by remote device is reported via this callback.
 * It runs on the MICO_NETWORKING_WORKER_THREAD context.
 */
static OSStatus disconnection_handler( mico_bt_smartbridge_socket_t* socket )
{
    char* bt_addr_str = DataToHexStringWithColons( (uint8_t *)socket->remote_device.address, 6 );
    mico_bt_app_log( "Disconnected from [%s]", bt_addr_str );
    free( bt_addr_str );
    return kNoErr;
}

/* Notification handler. GATT notification by remote device is reported via this callback.
 * It runs on the MICO_NETWORKING_WORKER_THREAD context.
 */
static OSStatus notification_handler( mico_bt_smartbridge_socket_t* socket, uint16_t attribute_handle )
{
    /* GATT value notification event. attribute_handle is the handle
     * which value of the attribute is updated by the remote device.
     */
    OSStatus err = kNoErr;
    char* bt_addr_str = NULL;
    char* data_str = NULL;
    mico_bt_smart_attribute_t *characteristic_value = NULL;

    /* Read sender's address */
    bt_addr_str = DataToHexStringWithColons( (uint8_t *)socket->remote_device.address, 6 );

    /* Read cached data */
    err = mico_bt_smart_attribute_create( &characteristic_value, MICO_ATTRIBUTE_TYPE_CHARACTERISTIC_VALUE, MAX_CHARACTERISTIC_VALUE_LENGTH );
    require_noerr(err, exit);
    err = mico_bt_smartbridge_get_attribute_cache_by_handle( socket, attribute_handle, characteristic_value, ATTR_CHARACTERISTIC_VALUE_SIZE( MAX_CHARACTERISTIC_VALUE_LENGTH ) );
    require_noerr_string(err, exit, "This is not a cached value handle, ignore...");
    data_str = DataToHexStringWithSpaces( characteristic_value->value.value, characteristic_value->value_length );

    mico_bt_app_log( "Recv data from [%s], handle 0x%x: %s", bt_addr_str, attribute_handle, data_str );

exit:
    if( bt_addr_str != NULL ) free( bt_addr_str );
    if( data_str != NULL ) free( data_str );
    if( characteristic_value != NULL ) mico_bt_smart_attribute_delete( characteristic_value );
    return err;
}

static OSStatus connect_list_init( void )
{
    return linked_list_init( &connecting_device_list );
}

static bool compare_device_by_address( linked_list_node_t* node_to_compare, void* user_data )
{
    connecting_device_t* device = (connecting_device_t* )node_to_compare;
    mico_bt_device_address_t* device_address  = (mico_bt_device_address_t *)user_data;

    if ( memcmp( device->device.address, device_address, BD_ADDR_LEN ) == 0 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

static OSStatus connect_list_add( mico_bt_smart_device_t remote_device )
{
    OSStatus err = kNoErr;
    connecting_device_t *device_found, *new_device;

    err = linked_list_find_node( &connecting_device_list, compare_device_by_address, remote_device.address, (linked_list_node_t**)&device_found );
    require_quiet( err == kNotFoundErr, exit );

    new_device = malloc( sizeof(connecting_device_t) );
    require_action( new_device, exit, err = kNoMemoryErr );

    memcpy( &new_device->device, &remote_device, sizeof(mico_bt_smart_device_t) );

    err = linked_list_insert_node_at_rear( &connecting_device_list, &new_device->this_node );
    require_noerr(err, exit);

exit:
    return err;
}

static OSStatus connect_list_get( mico_bt_smart_device_t** device )
{
    OSStatus err = kNoErr;
    connecting_device_t* current_device;

    err = linked_list_get_front_node( &connecting_device_list, (linked_list_node_t **)&current_device );
    require_noerr_quiet(err, exit);

    *device = &current_device->device;

exit:
    return err;
}

static OSStatus connect_list_remove( mico_bt_smart_device_t* device )
{
    OSStatus err = kNoErr;
    connecting_device_t* current_device;

    err = linked_list_find_node( &connecting_device_list, compare_device_by_address, device->address, (linked_list_node_t**)&current_device );
    require_noerr( err, exit );

    err = linked_list_remove_node( &connecting_device_list, &current_device->this_node );
    require_noerr( err, exit );

    free( current_device );
exit:
    return err;    
}











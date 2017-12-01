/**
 ******************************************************************************
 * @file    ble_scan.c
 * @author  William Xu
 * @version V1.0.0
 * @date    30-Apr-2016
 * @file    BLE scan function demonstration
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
 *
 * BLE scan function demonstration
 * Features demonstrated
 *  - Passive scan and active scan function
 *
 * To demonstrate the demo, type the corresponding command on command line interface
 * "blescan passive" or "blescan active"
 ******************************************************************************
 **/

#include "aos/aos.h"
#include "mico_bt.h"
#include "mico_bt_smartbridge.h"

#include "StringUtils.h"

#define ble_scan_log(M, ...) custom_log("LE SCAN", M, ##__VA_ARGS__)

/******************************************************************************
 *                                Constants
******************************************************************************/

/******************************************************************************
 *                           Function Prototypes
 ******************************************************************************/

/******************************************************************************
 *                                Structures
 ******************************************************************************/


/******************************************************************************
 *                                Variables Definitions
 ******************************************************************************/

static void start_scan                      ( CLI_ARGS );
static OSStatus scan_complete_handler       ( void *arg );

struct cli_command  ble_scan_cmd[] = 
{
    { "blescan", "blescan <passive/active>", start_scan },
};

/******************************************************************************
 *                          Function Definitions
 ******************************************************************************/

/*
 *  Entry point to the application.
 */

int application_start( void )
{
    OSStatus err = kNoErr;


    /* Initialise MiCO Bluetooth Framework */
    err = mico_bt_init( MICO_BT_HCI_MODE, "SmartBridge Device", 0, 0 );
    require_noerr( err, exit );

    /* Initialise MiCO SmartBridge, no connection is actually needed */
    err = mico_bt_smartbridge_init( 0 );
    require_noerr( err, exit );

    /* Register an user command to cli interface */
    err = aos_cli_register_commands( ble_scan_cmd, 1 );
    require_noerr( err, exit );

    ble_scan_log( "Initialize success, input \"blescan\" to start...");

exit:
  mico_rtos_delete_thread( NULL );
  return err;
}

/* Start scan process
 */
static void start_scan ( CLI_ARGS )
{
    /* Scan settings */
    mico_bt_smart_scan_settings_t scan_settings;

    scan_settings.type              = BT_SMART_PASSIVE_SCAN;
    scan_settings.filter_policy     = FILTER_POLICY_NONE;
    scan_settings.filter_duplicates = DUPLICATES_FILTER_DISABLED;
    scan_settings.interval          = 96;
    scan_settings.window            = 48;
    scan_settings.duration_second   = 3;

    if (!strcasecmp(argv[1], "active")) 
    {
        aos_cli_printf("Start ble active scan\r\n");
        scan_settings.type = BT_SMART_ACTIVE_SCAN;
    } else {
        aos_cli_printf("Start ble passive scan\r\n");
        scan_settings.type = BT_SMART_PASSIVE_SCAN;
    }    

    /* Start scan */
    //mico_bt_smartbridge_stop_scan( );
    mico_bt_smartbridge_start_scan( &scan_settings, scan_complete_handler, NULL );
}

static void print_adv_data( uint8_t *adv_data, uint8_t adv_data_len )
{
    uint8_t         adv_type;
    uint8_t         adv_data_length;
    uint8_t         *p;
    char            *adv_data_str = NULL;

    p = adv_data;
    STREAM_TO_UINT8( adv_data_length, p );

    while( (p - adv_data <= adv_data_len ) )
    {
        STREAM_TO_UINT8( adv_type, p );

        adv_data_str = DataToHexStringWithSpaces( p, adv_data_length );
        printf( " =>(%2x):%s\r\n", adv_type, adv_data_str );
        free( adv_data_str );

        p += adv_data_length - 1; /* skip the length of data */
        STREAM_TO_UINT8( adv_data_length, p );
    }
}


/* Scan complete handler. Scan complete event reported via this callback.
 * It runs on the MICO_NETWORKING_WORKER_THREAD context.
 */
static OSStatus scan_complete_handler( void *arg )
{
    OSStatus err = kNoErr;
    uint32_t count = 0;
    char*        bd_addr_str = NULL;
    mico_bt_smart_scan_result_t *scan_result = NULL;

    printf("Scan complete\r\n");
    err = mico_bt_smartbridge_get_scan_result_list( &scan_result, &count );
    require_noerr( err, exit );

    if( count == 0 )
    {
        printf( "No ble device found\r\n" );
        err = kNotFoundErr;
        goto exit;
    }

    while( scan_result != NULL )
    {
        bd_addr_str = DataToHexStringWithColons( (uint8_t *)scan_result->remote_device.address, 6 );
        printf("[%s] ", bd_addr_str );
        printf("address type: %x ", (uint16_t)scan_result->remote_device.address_type );
        printf("rssi: %d ", scan_result->signal_strength );
        printf("name: %s\r\n", scan_result->remote_device.name );
        free( bd_addr_str );

        printf(" =>Advertisement data:   \r\n" ); 
        print_adv_data( scan_result->last_advertising_event_received.eir_data, scan_result->last_advertising_event_received.eir_data_length );

        if( scan_result->last_scan_response_received.event == BT_SMART_SCAN_RESPONSE_EVENT)
        {
            printf(" =>Scan respond data:   \r\n" ); 
            print_adv_data( scan_result->last_scan_response_received.eir_data, scan_result->last_scan_response_received.eir_data_length );
        }

        printf("\r\n");
        scan_result = scan_result->next;
    }

exit:
    /* Scan duration is complete */
    return err;
}



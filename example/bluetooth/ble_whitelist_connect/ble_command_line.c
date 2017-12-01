/**
 ******************************************************************************
 * @file    ble_command_line.c
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
 *  - Processing user command over UART to control BLE Auto Connection Procedure.
 *  - Demonstrate BLE White List and Auto Connection Procedure.
 *
 *   To demonstrate the demo, work through the following steps.
 *   1. Plug the MiCOKit board into your computer, open a serial terminal
 *   2. Build and download the application (to the MiCOKit board)
 *   3. Type command "blescan" on serial terminal to control BLE Scanning Procedure.
 *   4. Turn on your a Peripheral device and connect to it. This Peripheral device
 *      is added to the white list now.
 *   5. Disconnect
 *   6. Type command "bleauto" on serial terminal to start The Auto Connection
 *      Procedure.
 *   7. Now you can observe that this local board would automatically connect to
 *      those device existed in the white list.
 * Note:
 *   The Peripheral Device must use the Public Address. So, the iOS device is
 *   not supported.
 *
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


static void socket_list     ( CLI_ARGS );
static void paired_list     ( CLI_ARGS );
static void auto_conn       ( CLI_ARGS );
static void scanning_change ( CLI_ARGS );

static mico_bt_smartbridge_socket_t *smartbridge_socket = NULL;
static uint8_t smartbridge_socket_count = 0;


struct cli_command  ble_scan_cmd[] = 
{
    { "blesocket", "list ble connection status", socket_list },
    { "blepaired", "list ble paired devices information", paired_list },
    { "bleautoconn", "auto connection procedure", auto_conn },
    { "blescan", "start or stop scanning procedure", scanning_change },
};

void ble_commands_init (mico_bt_smartbridge_socket_t *socket, uint8_t socket_count)
{
    smartbridge_socket = socket;
    smartbridge_socket_count = socket_count;
    
    /* Register an user command to cli interface */
    aos_cli_register_commands(ble_scan_cmd, sizeof(ble_scan_cmd)/sizeof(struct cli_command));
}



/* Define a  command: blesocket, display ble socket infors
 */
static void socket_list ( CLI_ARGS )
{
    uint32_t i;

    char *status_str[3] = { "DISCONNECTED", "CONNECTING  ", "CONNECTED   " };
    mico_bt_smartbridge_socket_status_t status;
    char* bt_addr_str = NULL;

    /* Iterate all sockets and look for the first available socket */
    printf("+-ID-+-------Address-------+-----Status-----+--Handle---+\r\n");
    for ( i = 0; i < smartbridge_socket_count; i++ )
    {
        bt_addr_str = DataToHexStringWithColons( smartbridge_socket[i].remote_device.address, BD_ADDR_LEN );
        mico_bt_smartbridge_get_socket_status( &smartbridge_socket[i], &status );
        printf("+-%02lu-+-[%s]-+--%s--+--0x%04x---+\r\n", i, bt_addr_str, status_str[status], smartbridge_socket[i].connection_handle);
    }
}

/* Define a  command: blepairedinfo, display paired devices and keys
 */
static void printf_device_data( uint8_t *address )
{
    char* debug_str = NULL;
    debug_str = DataToHexStringWithSpaces(address, BD_ADDR_LEN);
    printf("===============Peer Device:[%s]=====================\r\n", debug_str);
    free(debug_str);
}

static void printf_key_data( char *name, uint8_t *p_data, uint16_t len )
{
    char* debug_str = NULL;
    debug_str = DataToHexStringWithSpaces(p_data, len);
    printf("    %s:[%s]\r\n", name, debug_str);
    free(debug_str);
}

static void printf_key_number( char *name, uint32_t number )
{
    printf("    %s:%lu\r\n", name, number);
}

static void printf_key_desp( char *name, char *desp )
{
    printf("    %s:%s\r\n", name, desp);
}

static void printf_security_level( mico_bt_security_level level )
{
    if( level == BTM_LE_SEC_NONE )
    {
        printf("    Security Level:Not secured\r\n");
    }
    else if( level == BTM_LE_SEC_UNAUTHENTICATE )
    {
        printf("    Security Level:Not authenticated\r\n");
    }
    else if( level == BTM_LE_SEC_AUTHENTICATED )
    {
        printf("    Security Level:Authenticated\r\n");
    }
    else
    {
        printf("    Security Level:Unknown value\r\n");
    }
}


static void paired_list ( CLI_ARGS )
{
    OSStatus err = kNoErr;
    mico_bt_dev_bonded_device_info_t device_info[20];
    uint16_t i, num_devices = 20;
    mico_bt_security_key_value_t p_sec_keys;

    err = mico_bt_dev_get_bonded_devices(device_info, &num_devices);
    require_noerr(err, exit);

    for ( i = 0; i < num_devices; i++ )
    {
        printf_device_data( device_info[i].bd_addr );

        if( kNoErr == mico_bt_dev_get_key_by_keytype(device_info[i].bd_addr, BTM_LE_KEY_PENC, &p_sec_keys) )
        {
            printf(" Encryption information of peer device:\r\n");
            printf_key_data     ( "LTK           ", p_sec_keys.le_keys.penc_key.ltk,    BT_OCTET16_LEN );
            printf_key_data     ( "rand          ", p_sec_keys.le_keys.penc_key.rand,   BT_OCTET8_LEN );
            printf_key_number   ( "ediv          ", p_sec_keys.le_keys.penc_key.ediv );
            printf_key_number   ( "Key size      ", p_sec_keys.le_keys.penc_key.key_size );
            printf_security_level ( p_sec_keys.le_keys.penc_key.sec_level );
        }

        if( kNoErr == mico_bt_dev_get_key_by_keytype(device_info[i].bd_addr, BTM_LE_KEY_PID, &p_sec_keys))
        {
            printf(" Identity key of the peer device:\r\n");
            printf_key_data     ( "IRK           ", p_sec_keys.le_keys.pid_key.irk,         BT_OCTET16_LEN );
            printf_key_data     ( "Static Addr   ", p_sec_keys.le_keys.pid_key.static_addr, BD_ADDR_LEN );
            printf_key_desp     ( "Address Type  ", (p_sec_keys.le_keys.pid_key.addr_type)?"Static":"Random" );
        }

        if( kNoErr == mico_bt_dev_get_key_by_keytype(device_info[i].bd_addr, BTM_LE_KEY_LENC, &p_sec_keys))
        {
            printf(" Encryption information of local device:\r\n");
            printf_key_data     ( "LTK           ", p_sec_keys.le_keys.lenc_key.ltk,        BT_OCTET16_LEN );
            printf_key_number   ( "Div           ", p_sec_keys.le_keys.lenc_key.div );
            printf_key_number   ( "Key size      ", p_sec_keys.le_keys.lenc_key.key_size );
            printf_security_level ( p_sec_keys.le_keys.lenc_key.sec_level );
        }

        if( kNoErr == mico_bt_dev_get_key_by_keytype(device_info[i].bd_addr, BTM_LE_KEY_LID, &p_sec_keys))
        {
            printf(" Identity key of the loacl device:\r\n");
        }

        if( kNoErr == mico_bt_dev_get_key_by_keytype(device_info[i].bd_addr, BTM_LE_KEY_LCSRK, &p_sec_keys))
        {
            printf(" local CSRK has been deliver to peer:\r\n");
            printf_key_data     ( "LCSRK         ", p_sec_keys.le_keys.lcsrk_key.csrk,      BT_OCTET16_LEN );
            printf_key_number   ( "counter       ", p_sec_keys.le_keys.lcsrk_key.counter );
            printf_key_number   ( "Div           ", p_sec_keys.le_keys.lcsrk_key.div );
            printf_security_level ( p_sec_keys.le_keys.lcsrk_key.sec_level );
        }
    }

exit:
    return;
}


static void auto_conn(CLI_ARGS)
{
    uint8_t size = 0;
    OSStatus err;
    mico_bool_t start = FALSE;

    mico_bt_smart_scan_settings_t scan_settings;

    extern OSStatus ble_auto_conn_parms_callback_handler(const mico_bt_device_address_t device_address, 
                                                         const char *name, 
                                                         const uint8_t *p_data, 
                                                         uint8_t len,
                                                         mico_bt_smartbridge_auto_conn_cback_parms_t *parm);
    
    if (argc != 2) {
        printf("Usage: bleautoconn <action>\r\n");
        printf("  bleautoconn : 0 -- Stop Auto connection establishment procedure\r\n");
        printf("              : 1 -- Start Auto connection establishment procedure\r\n");
        return;
    }

    switch (argv[1][0]) {
    case '0':
        start = FALSE;
        break;
    case '1':
        start = TRUE;
        break; 
    default:
        printf("BleAutoconnection change: unknown arguments\r\n");
        return;
    }

    err = mico_bt_smartbridge_get_background_connection_devices_size(&size);
    if (err != kNoErr) {
        printf("Get whitelist size unsuccessfully!! status = %d\r\n", err);
        return;
    }
    
    printf("whitelist size: %d\r\n", size);
    
    scan_settings.interval = 256;
    scan_settings.window = 48;
    scan_settings.type = BT_SMART_PASSIVE_SCAN;
    scan_settings.duration_second = 30;
    err = mico_bt_smartbridge_set_auto_connection_action(start, &scan_settings, ble_auto_conn_parms_callback_handler);
    if (err != kNoErr) {
        printf("Start auto connection type unsuccessfully!! status = %d\r\n", err);
        return;
    }
    
    printf("%s to establish background connection...\r\n", start ? "Start" : "Stop");
}

/* This function is invoked when Console Command "adv" is received */
static void scanning_change(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    extern OSStatus scan_complete_handler ( void *arg );
    extern OSStatus scan_result_handler ( const mico_bt_smart_advertising_report_t* result );

    extern const mico_bt_smart_scan_settings_t scan_settings;
    mico_bt_dev_status_t ret;
    
    if (argc != 2) {
        printf("Usage: blescan <action>\r\n");
        printf("  act: 0 -- stop scanning procedure\r\n");
        printf("     : 1 -- start scanning procedure\r\n");
        return;
    }
    
    switch (argv[1][0]) {
    case '0':
        ret = mico_bt_smartbridge_stop_scan();
        printf("scanning_change: STOP SCAN %d\r\n", ret);
        break; 
    case '1':
        ret = mico_bt_smartbridge_start_scan(&scan_settings, scan_complete_handler, scan_result_handler);
        printf("scanning_change: START SCAN %d\r\n", ret);
        break; 
    default:
        printf("scanning_change: unknown arguments\r\n");
        break;
    }
}

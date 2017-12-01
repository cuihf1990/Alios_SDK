/**
 ******************************************************************************
 * @file    ble_access_main.c
 * @author  Jian Zhang
 * @version V1.1.0
 * @date    26-Aug-2016
 * @file    BLE Access Components demonstration via command line interface
 * ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2016 MXCHIP Inc.
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
 *  - Demonstrate BLE_ACCESS_CORE component by auto testing.
 *
 *   To demonstrate the demo, work through the following steps.
 *   1. Plug the MiCOKit board into your computer, open a serial terminal
 *   2. Build and download the application (to the MiCOKit board)
 *   3. Download ble_access_peripheral application to BLEKit-1036 Board.
 *   4. Power up BLEKit-1036 and MiCOKit-3239 Board.
 *   5. Type command "ble_scan" on MiCOKit-3239 Board UART to scan all device 
 *      implemented BLE_ACCESS protocol which is developed by MXCHIP.
 *      They are automatically added to the BLE_ACCESS group.  
 *   7. Type command "ble_auto_conn 1" on MiCOKit-3239 Board UART to start 
 *      auto testing. 
 *   8. Type command "ble_auto_conn 0" to stop auto testing.
 *
 ******************************************************************************
 **/

#include "mico.h"
#include "mico_bt.h"
#include "mico_bt_cfg.h"

#include "LinkListUtils.h"

#include "ble_access_core.h"

/*
 * Constants
 */

#define ble_access_app_log(M, ...) custom_log("MXCHIP_BLE_ACCESS", M, ##__VA_ARGS__)

/*
 * Structures
 */

typedef struct {
    UINT32 device_id;
    UINT32 disc_timeout;
    UINT32 sleep_timeout;
} mxchip_disc_info_t;

/*
 * Function prototypes
 */
static void     mxchip_ble_access_event_handler(UINT8 event, const ble_access_evt_parms_t *params);
static OSStatus mxchip_ble_access_time_event_handler(void *arg);

static void     mxchip_ble_access_scan_slave(CLI_ARGS);
static void     mxchip_ble_access_add_slave(CLI_ARGS);
static void     mxchip_ble_access_disc_slave(CLI_ARGS);
static void     mxchip_ble_access_auto_conn(CLI_ARGS);
static void     mxchip_ble_access_remote_led(CLI_ARGS);
static void     mxchip_ble_access_remove_slave(CLI_ARGS);

static const char *pBLE_Status(uint8_t status);

/*
 * Variables definitions
 */

struct cli_command mxchip_alert_cmd[] = {
    { "ble_scan", "Scan new slave", mxchip_ble_access_scan_slave },
    { "ble_add", "Add new slave", mxchip_ble_access_add_slave },
    { "ble_disc", "Disconnect slave", mxchip_ble_access_disc_slave },
    { "ble_rm", "Remove a slave", mxchip_ble_access_remove_slave },
    { "ble_auto_conn", "Start or Stop auto connect", mxchip_ble_access_auto_conn },
    { "ble_led", "Control Remote BLE Led", mxchip_ble_access_remote_led },
};

static mico_mutex_t         g_mxchip_mutex;
static linked_list_t        g_mxchip_timer;
static mico_worker_thread_t g_mxchip_worker_thread;
static mico_timed_event_t   g_mxchip_time_event;

static volatile uint32_t    g_mxchip_last_dev_id = 0;

/** Entry point to the application.
 *
 */
int application_start(void)
{
    OSStatus err = kNoErr;

    /* Initialize other global resource */
    linked_list_init( &g_mxchip_timer );
    mico_rtos_init_mutex( &g_mxchip_mutex );

    /* Initialize Bluetooth Interface */
    err = ble_access_bluetooth_init();
    require_noerr_string(err, exit, "Error initializing Bluetooth Interface");

    /* Start BLE Auto Connect Procedure */
    err = ble_access_bluetooth_start(mxchip_ble_access_event_handler);
    require_noerr_string(err, exit, "Error starting bluetooth");

    /* Create & Start a thread to wait FogCloud request & event. */
    mico_rtos_create_worker_thread(&g_mxchip_worker_thread, MICO_APPLICATION_PRIORITY, 2048, 5);

    /* Register CommandLine */
    aos_cli_register_commands(mxchip_alert_cmd, sizeof(mxchip_alert_cmd)/sizeof(struct cli_command));

exit:
    return err;
}

/** Terminates the ACC Thread
 *
 * Sets a flag then wakes the ACC Thread to force it to terminate.
 *
 */
static OSStatus mxchip_ble_access_time_event_handler(void *arg)
{
    OSStatus                 err = kNoErr;
    ble_access_cmd_parms_t   cmd;
    mxchip_disc_info_t      *info;
    linked_list_node_t      *node = NULL, *rear = NULL;
    uint32_t                 count = 0;
    mico_bt_device_address_t addr;

    mico_rtos_lock_mutex(&g_mxchip_mutex);
    if (linked_list_get_count(&g_mxchip_timer, &count) == kNoErr && count > 0) {
        linked_list_get_rear_node(&g_mxchip_timer, &rear);
        do {
            /* Iterate list */
            node = rear;
            rear = rear->prev;
            /* Check */
            info = (mxchip_disc_info_t *)node->data;
            if (--info->disc_timeout == 0) {
                ble_access_generate_device_address(addr, info->device_id);
                ble_access_app_log("Start to disconnecting %lu[%02x:%02x:%02x:%02x:%02x:%02x]", 
                                    info->device_id,
                                    addr[0], addr[1], addr[2],
                                    addr[3], addr[4], addr[5]);
                /* Request */ 
                cmd.device_id = info->device_id;
                cmd.p.timeout = info->sleep_timeout;
                if ((err = ble_access_bluetooth_request(BLE_ACCESS_REQ_DEV_DISC, &cmd)) != kNoErr) {
                    ble_access_app_log("ble_access_bluetooth_request failed, err = %d", err);
                } else {
                    ble_access_app_log("ble_access_bluetooth_request succ");
                }
                /* Free Resource */
                linked_list_remove_node(&g_mxchip_timer, node);
                free(node->data);
                free(node);
            }
        } while (rear);
    }
    mico_rtos_unlock_mutex(&g_mxchip_mutex);
    return err;
}

static bool find_callback(linked_list_node_t* node_to_compare, void* user_data)
{
    uint32_t device_id = *(uint32_t *)user_data;
    if (((mxchip_disc_info_t *)(node_to_compare->data))->device_id == device_id) {
        return 1;
    } else {
        return 0;
    }
}

static void uuid_to_str(const ble_access_uuid_t *uuid)
{
    if (uuid->len == 2) {
        ble_access_app_log("0x%04x", uuid->uu.uuid16);
    } else if (uuid->len == 4) {
        ble_access_app_log("0x%04x 0x%04x", (uint16_t)(uuid->uu.uuid32 >> 16), (uint16_t)(uuid->uu.uuid32 & 0xff00));
    } else {
        ble_access_app_log("{%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
                            uuid->uu.uuid128[15], uuid->uu.uuid128[14], uuid->uu.uuid128[13], uuid->uu.uuid128[12], uuid->uu.uuid128[11],
                            uuid->uu.uuid128[10], uuid->uu.uuid128[9], uuid->uu.uuid128[8], uuid->uu.uuid128[7], uuid->uu.uuid128[6], 
                            uuid->uu.uuid128[5], uuid->uu.uuid128[4], uuid->uu.uuid128[3], uuid->uu.uuid128[2], uuid->uu.uuid128[1],
                            uuid->uu.uuid128[0]);
    }
}

static void mxchip_ble_access_event_handler(UINT8 event, const ble_access_evt_parms_t *parms)
{
    OSStatus               err;
    ble_access_cmd_parms_t cmd;

    linked_list_node_t    *node = NULL;
    mxchip_disc_info_t    *disc_info = NULL;

    //ble_access_attribute_t char_value;
    uint8_t                idx;

    switch (event) {
    case BLE_ACCESS_EVENT_DEV_NEW:
        ble_access_app_log("BLE_ACCESS_EVENT_DEV_NEW: name - %s, id - %lu, rssi - %d",
                             parms->p.scan.name,
                             parms->device_id,
                             parms->p.scan.RSSI);

        cmd.device_id = parms->device_id;
        err = ble_access_bluetooth_request(BLE_ACCESS_REQ_DEV_ADD, &cmd);
        ble_access_app_log("Add device[%lu] %s", cmd.device_id, err == kNoErr ? "Successfully" : "Failed");
        break;

    case BLE_ACCESS_EVENT_DEV_ADD:
        ble_access_app_log("BLE_ACCESS_EVENT_DEV_ADD: id - %lu, status - %s, service - %u",
                             parms->device_id,
                             pBLE_Status(parms->status),
                             parms->p.add.serv_count);

        g_mxchip_last_dev_id = parms->device_id;

        if (parms->status == BLE_ACCESS_STATUS_SUCCESS) {

            uint8_t count = 0;
            for (count = 0; count < parms->p.add.serv_count; count++) {
                ble_access_app_log("service[uuid]: ");
                uuid_to_str(&parms->p.add.serv[count].type);
                ble_access_app_log("handle[start]: %u, handle[end]: %u", 
                                    parms->p.add.serv[count].start_handle,
                                    parms->p.add.serv[count].end_handle);
                
                err = ble_access_get_characteristics(parms->device_id, &parms->p.add.serv[count]);
                if (err != kNoErr) {
                    ble_access_app_log("Get characteristics failed: err = %d", err);
                }
            }

            uint8_t random[2] = {0};

            node = (linked_list_node_t *)malloc(sizeof(linked_list_node_t));
            require_string(node != NULL, exit, "Malloc failed");
            disc_info = (mxchip_disc_info_t *)malloc(sizeof(mxchip_disc_info_t));
            require_string(disc_info != NULL, exit, "Malloc failed");

            random[0] = rand();
            random[1] = rand();
            ble_access_app_log("Random: %d, %d", random[0], random[1]);

            disc_info->device_id = parms->device_id;
            disc_info->disc_timeout = random[0] + 10;
            disc_info->sleep_timeout = random[1] + 60;
            node->data = (void *)disc_info;

            mico_rtos_lock_mutex(&g_mxchip_mutex);
            err = linked_list_insert_node_at_front(&g_mxchip_timer, node);
            mico_rtos_unlock_mutex(&g_mxchip_mutex);
            require_noerr_string(err, exit, "Insert list failed");
        }
        break;
    case BLE_ACCESS_EVENT_DEV_AUTHING:
        ble_access_app_log("BLE_ACCESS_EVENT_DEV_AUTHING");
        break;
    case BLE_ACCESS_EVENT_DEV_CONN:
        ble_access_app_log("BLE_ACCESS_EVENT_DEV_CONN: id - %lu, status - %s",
                             parms->device_id,
                             pBLE_Status(parms->status));

        if (parms->status == BLE_ACCESS_STATUS_SUCCESS) {

            uint8_t random[2] = {0};

            node = (linked_list_node_t *)malloc(sizeof(linked_list_node_t));
            require_string(node != NULL, exit, "Malloc failed");
            disc_info = (mxchip_disc_info_t *)malloc(sizeof(mxchip_disc_info_t));
            require_string(disc_info != NULL, exit, "Malloc failed");

            random[0] = rand();
            random[1] = rand();
            ble_access_app_log("Random: %d, %d", random[0], random[1]);

            disc_info->device_id = parms->device_id;
            disc_info->disc_timeout = random[0] + 10;
            disc_info->sleep_timeout = random[1] + 60;
            node->data = (void *)disc_info;

            mico_rtos_lock_mutex(&g_mxchip_mutex);
            err = linked_list_insert_node_at_front(&g_mxchip_timer, node);
            mico_rtos_unlock_mutex(&g_mxchip_mutex);
            require_noerr_string(err, exit, "Insert list failed");

            g_mxchip_last_dev_id = parms->device_id;
        }
        break;
    case BLE_ACCESS_EVENT_DEV_DISC:
        ble_access_app_log("BLE_ACCESS_EVENT_DEV_DISC: id - %lu, status - %s", 
                            parms->device_id,
                            pBLE_Status(parms->status));

        /* Remove */
        mico_rtos_lock_mutex(&g_mxchip_mutex);
        if (parms->status == BLE_ACCESS_STATUS_SLAVE_REQ_DISC 
                && (kNoErr == linked_list_find_node(&g_mxchip_timer, find_callback, 
                                                    (void *)&parms->device_id, &node))) {
            linked_list_remove_node(&g_mxchip_timer, node);
        }         
        mico_rtos_unlock_mutex(&g_mxchip_mutex);
        break;
    case BLE_ACCESS_EVENT_GATT_CHAR:
        ble_access_app_log("BLE_ACCESS_EVENT_GATT_CHAR: id - %lu, status - %s", 
                            parms->device_id,
                            pBLE_Status(parms->status));

        /* Print some logs for a characteristic information */ 
        ble_access_app_log("value_handle: 0x%02x, properties: 0x%x, desc_start_handle: 0x%02x, desc_end_handle: 0x%02x", 
                            parms->p.gatt.attr->value.characteristic.value_handle,
                            parms->p.gatt.attr->value.characteristic.properties,
                            parms->p.gatt.attr->value.characteristic.descriptor_start_handle,
                            parms->p.gatt.attr->value.characteristic.descriptor_end_handle);
        ble_access_app_log("uuid: ");
        uuid_to_str(&parms->p.gatt.attr->value.characteristic.uuid);

        /* Get characteristic Value */
        //err = ble_access_get_attribute_by_handle(parms->device_id, 
        //                                         parms->p.gatt.attr->value.characteristic.value_handle, 
        //                                         &char_value);
        //require_noerr_string(err, exit, "Get Characteristic Value failed");
        //ble_access_app_log("char: handle[0x%02x], length[%u], structure_size[%u]", 
        //                    char_value.handle, 
        //                    char_value.value_length, 
        //                    char_value.value_struct_size);

        //for (idx = 0; idx < char_value.value_length; idx++) {
        //    ble_access_app_log("0x%02x", char_value.value.characteristic_value.value[idx]);
        //}
        break;
    case BLE_ACCESS_EVENT_GATT_CHAR_CMPL:
        ble_access_app_log("BLE_ACCESS_EVENT_GATT_CHAR_CMPL: id - %lu, status - %s", 
                            parms->device_id,
                            pBLE_Status(parms->status));
        ble_access_app_log("Get all characteristic finished!");
        break;
    case BLE_ACCESS_EVENT_GATT_NOTIFY:
        ble_access_app_log("BLE_ACCESS_EVENT_GATT_NOTIFY: id - %lu, status - %s", 
                            parms->device_id,
                            pBLE_Status(parms->status));

        ble_access_app_log("The server Notify Event: len[%lu], data:", parms->p.gatt.attr->value_length);
        for (idx = 0; idx < parms->p.gatt.attr->value_length; idx++) {
            ble_access_app_log("0x%02x", parms->p.gatt.attr->value.value[0]); 
        }
        break;
    default:
        break;
    }

    return;

exit:
    if (disc_info) free(disc_info);
    if (node) free(node);
    return;
}

static void mxchip_ble_access_scan_slave(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    ble_access_cmd_parms_t parms;

    if (argc >= 2) {
        parms.p.start = atoi(argv[1]);
    } else {
        parms.p.start = MICO_TRUE;
    }

    ble_access_bluetooth_request(BLE_ACCESS_REQ_DEV_SCAN, &parms);
}

static void mxchip_ble_access_add_slave(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    ble_access_cmd_parms_t parms;

    if (argc != 2) {
         ble_access_app_log("add slave arguments error");
         return;
    }

    parms.device_id = atoi(argv[1]);
    ble_access_bluetooth_request(BLE_ACCESS_REQ_DEV_ADD, &parms);
}

static void mxchip_ble_access_disc_slave(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    ble_access_cmd_parms_t parms;

    if (argc != 3) {
        ble_access_app_log("Request to disconnect failed");
        return;
    }
    parms.device_id = atoi(argv[1]);
    parms.p.timeout = atoi(argv[2]);
    ble_access_bluetooth_request(BLE_ACCESS_REQ_DEV_DISC, &parms);
}

static void mxchip_ble_access_auto_conn(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    if (argc != 2) {
        ble_access_app_log("auto conn arguments error");
        return;
    }
    if (atoi(argv[1])) {
        mico_rtos_register_timed_event(&g_mxchip_time_event, &g_mxchip_worker_thread, 
                                        mxchip_ble_access_time_event_handler, 1000, NULL);
        ble_access_bluetooth_request(BLE_ACCESS_REQ_DEV_START_AUTO, NULL);
    } else {
        mico_rtos_deregister_timed_event(&g_mxchip_time_event);
        ble_access_bluetooth_request(BLE_ACCESS_REQ_DEV_STOP_AUTO, NULL);
    }
}

static void mxchip_ble_access_remote_led(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    OSStatus err = kNoErr;
    uint8_t value = 0;
    
    if (argc != 2 || !g_mxchip_last_dev_id) {
        ble_access_app_log("Control Remote LED ERROR");
        return;
    }

    value = atoi(argv[1]);
    ble_access_app_log("update characteristic value: device_id[%lu], value[%u]", 
                        g_mxchip_last_dev_id, value);
    err = ble_access_update_characteristic_value(g_mxchip_last_dev_id, 0x0c, 1, &value);
    require_noerr_string(err, exit, "update characteristic value failed");
    
exit:
    return;
}

static void mxchip_ble_access_remove_slave(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    OSStatus               err = kNoErr;
    ble_access_cmd_parms_t parms;

    require_string(argc == 2, exit, "invalid parameters");

    parms.device_id = atoi(argv[1]);
    err = ble_access_bluetooth_request(BLE_ACCESS_REQ_DEV_REMOVE, &parms);
    require_noerr_string(err, exit, "remove a slave failed");

exit: 
    return;
}

static const char *pBLE_Status(uint8_t status)
{
    switch (status) {
    case BLE_ACCESS_STATUS_SUCCESS       : 
        return "Successfully";
    case BLE_ACCESS_STATUS_FAILED        :
        return "Failed";
    case BLE_ACCESS_STATUS_AUTH_FAILED   :
        return "Authentication failed";
    case BLE_ACCESS_STATUS_ADD_FAILED    :
        return "Add Device failed";
    case BLE_ACCESS_STATUS_SLAVE_REJECT  :
        return "Slave reject";
    case BLE_ACCESS_STATUS_RECONN_FAILED :
        return "Re-connect failed";
    case BLE_ACCESS_STATUS_NO_RESOURCES  :
        return "No resource";
    case BLE_ACCESS_STATUS_BUSY          :
        return "Busy";
    case BLE_ACCESS_STATUS_TIMEOUT       :
        return "Timeout";
    case BLE_ACCESS_STATUS_SLAVE_REQ_DISC:
        return "Slave terminate";
    case BLE_ACCESS_STATUS_INVALID_PARMS :
        return "Invalid params";
    case BLE_ACCESS_STATUS_NO_CONNECTION : 
        return "No Connection";
    default:
        return "Unknown status";
    }
}

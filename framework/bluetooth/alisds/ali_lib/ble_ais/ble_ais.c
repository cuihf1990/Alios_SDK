/*
 * Copyright (c) 2015 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Use in source and binary forms, redistribution in binary form only, with
 * or without modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 2. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 3. This software, with or without modification, must only be used with a Nordic
 *    Semiconductor ASA integrated circuit.
 * 
 * 4. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "sdk_common.h"
#include "ble_ais.h"
#include "ble_srv_common.h"

#define BLE_UUID_AIS_RC_CHARACTERISTIC    0xFED4                   /**< The UUID of the "Read Characteristics" Characteristic. */
#define BLE_UUID_AIS_WC_CHARACTERISTIC    0xFED5                   /**< The UUID of the "Write Characteristics" Characteristic. */
#define BLE_UUID_AIS_IC_CHARACTERISTIC    0xFED6                   /**< The UUID of the "Indicate Characteristics" Characteristic. */
#define BLE_UUID_AIS_WWNRC_CHARACTERISTIC 0xFED7                   /**< The UUID of the "Write WithNoRsp Characteristics" Characteristic. */
#define BLE_UUID_AIS_NC_CHARACTERISTIC    0xFED8                   /**< The UUID of the "Notify Characteristics" Characteristic. */

#define BLE_AIS_MAX_RC_CHAR_LEN           BLE_AIS_MAX_DATA_LEN     /**< Maximum length of the "Read Characteristics" Characteristic (in bytes). */

#define MAX_MTU                           247                      /**< MTU defined by Alibaba specification: maximum data length = 240. */


/**@brief Notify Rx data to higher layer. */
static void notify_data (ble_ais_t * p_ais, uint8_t * p_data, uint16_t length)
{
    ble_ais_event_t evt;

    evt.type                = BLE_AIS_EVT_RX_DATA;
    evt.data.rx_data.p_data = p_data;
    evt.data.rx_data.length = length;
    p_ais->event_handler(p_ais->p_context, &evt);
}


/**@brief Notify number of packets sent to higher layer (i.e. Tx-done). */
static void notify_pkt_sent (ble_ais_t * p_ais, uint8_t pkt_sent)
{
    ble_ais_event_t evt;

    evt.type                  = BLE_AIS_EVT_TX_DONE;
    evt.data.tx_done.pkt_sent = pkt_sent;
    p_ais->event_handler(p_ais->p_context, &evt);
}


/**@brief Notify higher layer that service has been enabled.
 *
 * @note  Both the notification of "Notify Characteristics" characteristic
 *        and the indication of "Indicate Characteristics" characteristic
 *        has been enabled
 */
static void notify_svc_enabled (ble_ais_t * p_ais)
{
    ble_ais_event_t evt;

    if (p_ais->is_indication_enabled && p_ais->is_notification_enabled)
    {
        evt.type = BLE_AIS_EVT_SVC_ENABLED;
        p_ais->event_handler(p_ais->p_context, &evt);
    }
}


/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the SoftDevice.
 *
 * @param[in] p_ais     Alibaba IOT Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect(ble_ais_t * p_ais, ble_evt_t * p_ble_evt)
{
    p_ais->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    p_ais->is_authenticated = false;
    p_ais->is_indication_enabled   = false;
    p_ais->is_notification_enabled = false;
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the SoftDevice.
 *
 * @param[in] p_ais     Alibaba IOT Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_ais_t * p_ais, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_ais->conn_handle      = BLE_CONN_HANDLE_INVALID;
    p_ais->is_authenticated = false;
    p_ais->is_indication_enabled   = false;
    p_ais->is_notification_enabled = false;
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the SoftDevice.
 *
 * @param[in] p_ais     Alibaba IOT Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_ais_t * p_ais, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (p_ble_evt->evt.gatts_evt.conn_handle != p_ais->conn_handle)
    {
        return;
    }

    if (p_evt_write->handle == p_ais->wc_handles.value_handle
             || p_evt_write->handle == p_ais->wwnrc_handles.value_handle)
    {
        notify_data(p_ais, p_evt_write->data, p_evt_write->len);
    }
    else if (p_evt_write->handle == p_ais->nc_handles.cccd_handle
             && p_evt_write->len == 2)
    {
        p_ais->is_notification_enabled = ble_srv_is_notification_enabled(p_evt_write->data);
        notify_svc_enabled(p_ais);
    }
    else if (p_evt_write->handle == p_ais->ic_handles.cccd_handle
             && p_evt_write->len == 2)
    {
        p_ais->is_indication_enabled = ble_srv_is_indication_enabled(p_evt_write->data);
        notify_svc_enabled(p_ais);
    }
    else
    {
        // Do Nothing. This event is not relevant for this service.
    }
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST event from the SoftDevice.
 *
 * @param[in] p_ais     Alibaba IOT Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_rw_auth_req(ble_ais_t * p_ais, ble_evt_t * p_ble_evt)
{
    uint32_t err_code;
    ble_gatts_evt_rw_authorize_request_t * req = &p_ble_evt->evt.gatts_evt.params.authorize_request;
    ble_gatts_rw_authorize_reply_params_t  auth_reply;

    if (p_ble_evt->evt.gatts_evt.conn_handle != p_ais->conn_handle)
    {
        return;
    }

    switch (req->type)
    {
        case BLE_GATTS_AUTHORIZE_TYPE_READ:
            if (req->request.read.handle == p_ais->rc_handles.value_handle)
            {
                auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                auth_reply.params.read.gatt_status = (p_ais->is_authenticated)?
                                                     BLE_GATT_STATUS_SUCCESS:
                                                     BLE_GATT_STATUS_ATTERR_INSUF_AUTHORIZATION;
                err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle, &auth_reply);
                VERIFY_SUCCESS_VOID(err_code);
            }
            break;

        case BLE_GATTS_AUTHORIZE_TYPE_WRITE:
            if (req->request.write.handle == p_ais->wc_handles.value_handle)
            {
                uint16_t gatt_status;

                auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                auth_reply.params.write.update      = 1;

                gatt_status = (req->request.write.op == BLE_GATTS_OP_WRITE_REQ
                               || req->request.write.op == BLE_GATTS_OP_WRITE_CMD)?
                              BLE_GATT_STATUS_SUCCESS:
                              BLE_GATT_STATUS_ATTERR_REQUEST_NOT_SUPPORTED;
                gatt_status = (p_ais->is_authenticated)? gatt_status: BLE_GATT_STATUS_ATTERR_INSUF_AUTHORIZATION;

                auth_reply.params.write.gatt_status = gatt_status;
                auth_reply.params.write.offset      = req->request.write.offset;
                auth_reply.params.write.len         = req->request.write.len;
                auth_reply.params.write.p_data      = req->request.write.data;

                err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle, &auth_reply);
                VERIFY_SUCCESS_VOID(err_code);

                // pass data to higher layer
                if (gatt_status == BLE_GATT_STATUS_SUCCESS)
                {
                    notify_data(p_ais, req->request.write.data, req->request.write.len);
                }
            }
            break;

        default:
            break;
    }
}


/**@brief Function for handling the @ref BLE_EVT_TX_COMPLETE event from the SoftDevice.
 *
 * @param[in] p_ais     Alibaba IOT Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_tx_complete(ble_ais_t * p_ais, ble_evt_t * p_ble_evt)
{
    if (p_ble_evt->evt.common_evt.conn_handle == p_ais->conn_handle)
    {
        notify_pkt_sent(p_ais, p_ble_evt->evt.common_evt.params.tx_complete.count);
    }
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_HVC event from the SoftDevice.
 *
 * @param[in] p_ais     Alibaba IOT Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_hvc(ble_ais_t * p_ais, ble_evt_t * p_ble_evt)
{
    if (p_ble_evt->evt.gatts_evt.params.hvc.handle == p_ais->ic_handles.value_handle
        && p_ble_evt->evt.gatts_evt.conn_handle == p_ais->conn_handle)
    {
        notify_pkt_sent(p_ais, 1);
    }
}


/**@brief Function for adding "Read Characteristics" characteristic.
 *
 * @param[in] p_ais       Alibaba IOT Service structure.
 * @param[in] p_ais_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t rc_char_add(ble_ais_t * p_ais, const ble_ais_init_t * p_ais_init)
{
    /**@snippet [Adding characteristic to SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read  = 1;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf        = NULL;
    char_md.p_user_desc_md   = NULL;
    char_md.p_cccd_md        = NULL;
    char_md.p_sccd_md        = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_AIS_RC_CHARACTERISTIC);

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 1;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_AIS_MAX_RC_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(p_ais->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ais->rc_handles);
    /**@snippet [Adding characteristic to SoftDevice] */
}


/**@brief Function for adding "Write Characteristics" characteristic.
 *
 * @param[in] p_ais       Alibaba IOT Service structure.
 * @param[in] p_ais_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t wc_char_add(ble_ais_t * p_ais, const ble_ais_init_t * p_ais_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read          = 1;
    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 0;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_AIS_WC_CHARACTERISTIC);

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 1;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = p_ais->max_pkt_size;

    return sd_ble_gatts_characteristic_add(p_ais->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ais->wc_handles);
}


/**@brief Function for adding "Indicate Characteristics" characteristic.
 *
 * @param[in] p_ais       Alibaba IOT Service structure.
 * @param[in] p_ais_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t ic_char_add(ble_ais_t * p_ais, const ble_ais_init_t * p_ais_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read     = 1;
    char_md.char_props.indicate = 1;
    char_md.char_props.notify   = 0;
    char_md.p_char_user_desc    = NULL;
    char_md.p_char_pf           = NULL;
    char_md.p_user_desc_md      = NULL;
    char_md.p_cccd_md           = &cccd_md;
    char_md.p_sccd_md           = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_AIS_IC_CHARACTERISTIC);

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = p_ais->max_pkt_size;

    return sd_ble_gatts_characteristic_add(p_ais->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ais->ic_handles);
}


/**@brief Function for adding "Write WithNoRsp Characteristics" characteristic.
 *
 * @param[in] p_ais       Alibaba IOT Service structure.
 * @param[in] p_ais_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t wwnrc_char_add(ble_ais_t * p_ais, const ble_ais_init_t * p_ais_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read          = 1;
    char_md.char_props.write         = 0;
    char_md.char_props.write_wo_resp = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_AIS_WWNRC_CHARACTERISTIC);

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = p_ais->max_pkt_size;

    return sd_ble_gatts_characteristic_add(p_ais->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ais->wwnrc_handles);
}


/**@brief Function for adding "Notify Characteristics" characteristic.
 *
 * @param[in] p_ais       Alibaba IOT Service structure.
 * @param[in] p_ais_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t nc_char_add(ble_ais_t * p_ais, const ble_ais_init_t * p_ais_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read     = 1;
    char_md.char_props.indicate = 0;
    char_md.char_props.notify   = 1;
    char_md.p_char_user_desc    = NULL;
    char_md.p_char_pf           = NULL;
    char_md.p_user_desc_md      = NULL;
    char_md.p_cccd_md           = &cccd_md;
    char_md.p_sccd_md           = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_AIS_NC_CHARACTERISTIC);

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = p_ais->max_pkt_size;

    return sd_ble_gatts_characteristic_add(p_ais->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ais->nc_handles);
}


void ble_ais_on_ble_evt(ble_ais_t * p_ais, ble_evt_t * p_ble_evt)
{
    if ((p_ais == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_ais, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_ais, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_ais, p_ble_evt);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            on_rw_auth_req(p_ais, p_ble_evt);
            break;

        case BLE_EVT_TX_COMPLETE:
            on_tx_complete(p_ais, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVC:
            on_hvc(p_ais, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}


uint32_t ble_ais_init(ble_ais_t * p_ais, const ble_ais_init_t * p_ais_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;

    VERIFY_PARAM_NOT_NULL(p_ais);
    VERIFY_PARAM_NOT_NULL(p_ais_init);
    VERIFY_PARAM_NOT_NULL(p_ais_init->event_handler);
    if (p_ais_init->mtu < GATT_MTU_SIZE_DEFAULT || p_ais_init->mtu > MAX_MTU)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    // Initialize the service structure.
    memset(p_ais, 0, sizeof(ble_ais_t));
    p_ais->conn_handle             = BLE_CONN_HANDLE_INVALID;
    p_ais->event_handler           = p_ais_init->event_handler;
    p_ais->p_context               = p_ais_init->p_context;
    p_ais->is_indication_enabled   = false;
    p_ais->is_notification_enabled = false;
    p_ais->is_authenticated        = false;
    p_ais->max_pkt_size            = p_ais_init->mtu - 3;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_AIS_SERVICE);

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_ais->service_handle);
    /**@snippet [Adding proprietary Service to SoftDevice] */
    VERIFY_SUCCESS(err_code);

    // Add the "Read Characteristics" Characteristic.
    err_code = rc_char_add(p_ais, p_ais_init);
    VERIFY_SUCCESS(err_code);

    // Add the "Write Characteristics" Characteristic.
    err_code = wc_char_add(p_ais, p_ais_init);
    VERIFY_SUCCESS(err_code);

    // Add the "Indicate Characteristics" Characteristic.
    err_code = ic_char_add(p_ais, p_ais_init);
    VERIFY_SUCCESS(err_code);

    // Add the "Write WithNoRsp Characteristics" Characteristic.
    err_code = wwnrc_char_add(p_ais, p_ais_init);
    VERIFY_SUCCESS(err_code);

    // Add the "Notify Characteristics" Characteristic.
    err_code = nc_char_add(p_ais, p_ais_init);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}


uint32_t ble_ais_set_auth(ble_ais_t * p_ais, bool is_authenticated)
{
    VERIFY_PARAM_NOT_NULL(p_ais);

    if (p_ais->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    p_ais->is_authenticated = is_authenticated;
    return NRF_SUCCESS;
}


uint32_t ble_ais_set_mtu(ble_ais_t * p_ais, uint16_t mtu)
{
    VERIFY_PARAM_NOT_NULL(p_ais);

    if (p_ais->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (mtu > MAX_MTU || mtu < GATT_MTU_SIZE_DEFAULT)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    p_ais->max_pkt_size = mtu - 3;
    return NRF_SUCCESS;
}


uint32_t ble_ais_send_notification(ble_ais_t * p_ais, uint8_t * p_data, uint16_t length)
{
    ble_gatts_hvx_params_t hvx_params;

    VERIFY_PARAM_NOT_NULL(p_ais);

    if (p_ais->conn_handle == BLE_CONN_HANDLE_INVALID || !p_ais->is_notification_enabled)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (length > p_ais->max_pkt_size)
    {
        return NRF_ERROR_DATA_SIZE;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));
    hvx_params.p_data = p_data;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.handle = p_ais->nc_handles.value_handle;

    return sd_ble_gatts_hvx(p_ais->conn_handle, &hvx_params);
}


uint32_t ble_ais_send_indication(ble_ais_t * p_ais, uint8_t * p_data, uint16_t length)
{
    ble_gatts_hvx_params_t hvx_params;

    VERIFY_PARAM_NOT_NULL(p_ais);

    if (p_ais->conn_handle == BLE_CONN_HANDLE_INVALID || !p_ais->is_indication_enabled)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (length > p_ais->max_pkt_size)
    {
        return NRF_ERROR_DATA_SIZE;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));
    hvx_params.p_data = p_data;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_INDICATION;
    hvx_params.handle = p_ais->ic_handles.value_handle;

    return sd_ble_gatts_hvx(p_ais->conn_handle, &hvx_params);
}



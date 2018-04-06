/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include "linklora.h"
#include "commissioning.h"
#include "timeServer.h"

static uint8_t dev_eui[] = LORAWAN_DEVICE_EUI;
static uint8_t app_eui[] = LORAWAN_APPLICATION_EUI;
static uint8_t app_key[] = LORAWAN_APPLICATION_KEY;

#define LORAWAN_APP_DATA_BUFF_SIZE 242

static uint8_t app_data_buff[LORAWAN_APP_DATA_BUFF_SIZE];
static lora_AppData_t app_data = { app_data_buff, 0, 0 };

static LoRaMainCallback_t *app_callbacks;
static int8_t is_tx_confirmed;

static bool next_tx = true;

static TimerEvent_t TxNextPacketTimer;
static DeviceState_t device_state = DEVICE_STATE_INIT;

lora_config_t g_lora_config = {471900000, DR_2, NODE_MODE_NORMAL, VALID_LORA_CONFIG};
//lora_config_t g_lora_config = {471900000, DR_2, NODE_MODE_NORMAL, INVALID_LORA_CONFIG};
join_method_t g_join_method;

struct ComplianceTest_s {
    bool Running;
    uint8_t State;
    FunctionalState IsTxConfirmed;
    uint8_t AppPort;
    uint8_t AppDataSize;
    uint8_t *AppDataBuffer;
    uint16_t DownLinkCounter;
    bool LinkCheck;
    uint8_t DemodMargin;
    uint8_t NbGateways;
} ComplianceTest;

static void prepare_tx_frame(uint8_t port)
{
    switch (port) {
        case 224:
            break;
        default:
            LoRaMainCallbacks->LoraTxData( &app_data, &is_tx_confirmed );
            break
    }
}

static bool send_frame(void)
{
    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;

    if (LoRaMacQueryTxPossible(AppData.BuffSize, &txInfo) != LORAMAC_STATUS_OK) {
        // Send empty frame in order to flush MAC commands
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fBuffer = NULL;
        mcpsReq.Req.Unconfirmed.fBufferSize = 0;
        mcpsReq.Req.Unconfirmed.Datarate = LoRaParamInit->TxDatarate;
    } else {
        if (IsTxConfirmed == DISABLE) {
            mcpsReq.Type = MCPS_UNCONFIRMED;
            mcpsReq.Req.Unconfirmed.fPort = AppData.Port;
            mcpsReq.Req.Unconfirmed.fBuffer = AppData.Buff;
            mcpsReq.Req.Unconfirmed.fBufferSize = AppData.BuffSize;
            mcpsReq.Req.Unconfirmed.Datarate = LoRaParamInit->TxDatarate;
        } else {
            mcpsReq.Type = MCPS_CONFIRMED;
            mcpsReq.Req.Confirmed.fPort = AppData.Port;
            mcpsReq.Req.Confirmed.fBuffer = AppData.Buff;
            mcpsReq.Req.Confirmed.fBufferSize = AppData.BuffSize;
            mcpsReq.Req.Confirmed.NbTrials = 8;
            mcpsReq.Req.Confirmed.Datarate = LoRaParamInit->TxDatarate;
        }
    }

    if (LoRaMacMcpsRequest(&mcpsReq) == LORAMAC_STATUS_OK) {
        return false;
    }
    return true;
}

static void on_tx_next_packet_timer_event(void)
{
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t status;

    TimerStop(&TxNextPacketTimer);

    mibReq.Type = MIB_NETWORK_JOINED;
    status = LoRaMacMibGetRequestConfirm(&mibReq);

    if (status == LORAMAC_STATUS_OK) {
        if (mibReq.Param.IsNetworkJoined == true) {
            device_state = DEVICE_STATE_SEND;
            next_tx = true;
        } else {
            g_join_method = (g_join_method + 1) % JOIN_METHOD_NUM;
            device_state = DEVICE_STATE_JOIN;
        }
    }
}

static void reset_join_state(void)
{
    g_lora_config.flag = INVALID_LORA_CONFIG;
    //  aos_kv_set();
    device_state = DEVICE_STATE_JOIN;
}

static void mcps_confirm(McpsConfirm_t *mcpsConfirm)
{
    if (mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK) {
        switch (mcpsConfirm->McpsRequest) {
            case MCPS_UNCONFIRMED:
            {
                // Check Datarate
                // Check TxPower
                break;
            }
            case MCPS_CONFIRMED:
            {
                // Check Datarate
                // Check TxPower
                // Check AckReceived
                // Check NbTrials
                break;
            }
            case MCPS_PROPRIETARY:
            {
                break;
            }
            default:
                break;
        }
    }
    else
    {
        switch( mcpsConfirm->McpsRequest )
        {
            case MCPS_UNCONFIRMED:
            {
                // Check Datarate
                // Check TxPower
                break;
            }
            case MCPS_CONFIRMED:
            {
                // Check Datarate
                // Check TxPower
                // Check AckReceived
                // Check NbTrials

                reset_join_state();
                g_join_method = STORED_JOIN_METHOD;
                PRINTF("Not receive Ack,Start to Join...\r\n");
                break;
            }
            case MCPS_PROPRIETARY:
            {
                break;
            }
            default:
                break;
        }
    }
    next_tx = true;
}

static void McpsIndication( McpsIndication_t *mcpsIndication )
{
    if ( mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
        return;
    }

    switch ( mcpsIndication->McpsIndication )
    {
        case MCPS_UNCONFIRMED:
            {
            break;
        }
        case MCPS_CONFIRMED:
            {
            break;
        }
        case MCPS_PROPRIETARY:
            {
            break;
        }
        case MCPS_MULTICAST:
            {
            DBG_PRINTF( "MCPS_MULTICAST\n" );
            break;
        }
        default:
            break;
    }

    // Check Multicast
    // Check Port
    // Check Datarate
    // Check FramePending
    // Check Buffer
    // Check BufferSize
    // Check Rssi
    // Check Snr
    // Check RxSlot
    DBG_PRINTF( "rssi = %d, snr = %d, datarate = %d\n", mcpsIndication->Rssi, mcpsIndication->Snr,
                mcpsIndication->RxDatarate );

    if ( ComplianceTest.Running == true )
    {
        ComplianceTest.DownLinkCounter++;
    }

    if ( mcpsIndication->RxData == true )
    {
        switch ( mcpsIndication->Port )
        {
            case 224:
                if ( ComplianceTest.Running == false )
                {
                    // Check compliance test enable command (i)
                    if ( (mcpsIndication->BufferSize == 4) &&
                         (mcpsIndication->Buffer[0] == 0x01)
                         &&
                         (mcpsIndication->Buffer[1] == 0x01)
                         &&
                         (mcpsIndication->Buffer[2] == 0x01)
                         &&
                         (mcpsIndication->Buffer[3] == 0x01) )
                    {
                        IsTxConfirmed = DISABLE;
                        AppData.Port = 224;
                        AppData.BuffSize = 2;
                        ComplianceTest.DownLinkCounter = 0;
                        ComplianceTest.LinkCheck = false;
                        ComplianceTest.DemodMargin = 0;
                        ComplianceTest.NbGateways = 0;
                        ComplianceTest.Running = true;
                        ComplianceTest.State = 1;

                        MibRequestConfirm_t mibReq;
                        mibReq.Type = MIB_ADR;
                        mibReq.Param.AdrEnable = true;
                        LoRaMacMibSetRequestConfirm( &mibReq );

#if defined(REGION_EU868)
                        LoRaMacTestSetDutyCycleOn(false);
#endif
                    }
                }
                else
                {
                    ComplianceTest.State = mcpsIndication->Buffer[0];
                    switch ( ComplianceTest.State )
                    {
                        case 0: // Check compliance test disable command (ii)
                            ComplianceTest.DownLinkCounter = 0;
                            ComplianceTest.Running = false;

                            MibRequestConfirm_t mibReq;
                            mibReq.Type = MIB_ADR;
                            mibReq.Param.AdrEnable = LoRaParamInit->AdrEnable;
                            LoRaMacMibSetRequestConfirm( &mibReq );
#if defined(REGION_EU868)
                            lora_config_duty_cycle_set(LORAWAN_DUTYCYCLE_ON ? ENABLE : DISABLE);
#endif
                            break;
                        case 1: // (iii, iv)
                            AppData.BuffSize = 2;
                            break;
                        case 2: // Enable confirmed messages (v)
                            IsTxConfirmed = ENABLE;
                            ComplianceTest.State = 1;
                            break;
                        case 3: // Disable confirmed messages (vi)
                            IsTxConfirmed = DISABLE;
                            ComplianceTest.State = 1;
                            break;
                        case 4: // (vii)
                            AppData.BuffSize = mcpsIndication->BufferSize;

                            AppData.Buff[0] = 4;
                            for ( uint8_t i = 1; i < AppData.BuffSize; i++ )
                            {
                                AppData.Buff[i] = mcpsIndication->Buffer[i] + 1;
                            }
                            break;
                        case 5: // (viii)
                        {
                            MlmeReq_t mlmeReq;
                            mlmeReq.Type = MLME_LINK_CHECK;
                            LoRaMacMlmeRequest( &mlmeReq );
                        }
                            break;
                        case 6: // (ix)
                        {
                            MlmeReq_t mlmeReq;

                            // Disable TestMode and revert back to normal operation

                            ComplianceTest.DownLinkCounter = 0;
                            ComplianceTest.Running = false;

                            MibRequestConfirm_t mibReq;
                            mibReq.Type = MIB_ADR;
                            mibReq.Param.AdrEnable = LoRaParamInit->AdrEnable;
                            LoRaMacMibSetRequestConfirm( &mibReq );
#if defined(REGION_EU868)
                            lora_config_duty_cycle_set(LORAWAN_DUTYCYCLE_ON ? ENABLE : DISABLE);
#endif

                            mlmeReq.Type = MLME_JOIN;

                            mlmeReq.Req.Join.DevEui = DevEui;
                            mlmeReq.Req.Join.AppEui = AppEui;
                            mlmeReq.Req.Join.AppKey = AppKey;
                            mlmeReq.Req.Join.NbTrials = 3;

                            LoRaMacMlmeRequest( &mlmeReq );
                            device_state = DEVICE_STATE_SLEEP;
                        }
                            break;
                        case 7: // (x)
                        {
                            if ( mcpsIndication->BufferSize == 3 )
                            {
                                MlmeReq_t mlmeReq;
                                mlmeReq.Type = MLME_TXCW;
                                mlmeReq.Req.TxCw.Timeout = (uint16_t) ((mcpsIndication->Buffer[1] << 8)
                                    | mcpsIndication->Buffer[2]);
                                LoRaMacMlmeRequest( &mlmeReq );
                            }
                            else if ( mcpsIndication->BufferSize == 7 )
                            {
                                MlmeReq_t mlmeReq;
                                mlmeReq.Type = MLME_TXCW_1;
                                mlmeReq.Req.TxCw.Timeout = (uint16_t) ((mcpsIndication->Buffer[1] << 8)
                                    | mcpsIndication->Buffer[2]);
                                mlmeReq.Req.TxCw.Frequency = (uint32_t) ((mcpsIndication->Buffer[3] << 16)
                                    | (mcpsIndication->Buffer[4] << 8) | mcpsIndication->Buffer[5])
                                                             * 100;
                                mlmeReq.Req.TxCw.Power = mcpsIndication->Buffer[6];
                                LoRaMacMlmeRequest( &mlmeReq );
                            }
                            ComplianceTest.State = 1;
                        }
                            break;
                        default:
                            break;
                    }
                }
                break;
            default:

                AppData.Port = mcpsIndication->Port;
                AppData.BuffSize = mcpsIndication->BufferSize;
                memcpy1( AppData.Buff, mcpsIndication->Buffer, AppData.BuffSize );

                LoRaMainCallbacks->LoraRxData( &AppData );
                break;
        }
    }
}

static void MlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
    switch ( mlmeConfirm->MlmeRequest )
    {
        case MLME_JOIN:
        {
            if (mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK) {
                // Status is OK, node has joined the network
                device_state = DEVICE_STATE_JOINED;
            } else {
                // Join was not successful. Try to join again
                reset_join_state();
                g_join_method = (g_join_method + 1) % JOIN_METHOD_NUM;
                DBG_LINKLORA("Rejoin\r\n");
            }
            break;
        }
        case MLME_LINK_CHECK:
            {
            if ( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
                // Check DemodMargin
                // Check NbGateways
                if ( ComplianceTest.Running == true )
                {
                    ComplianceTest.LinkCheck = true;
                    ComplianceTest.DemodMargin = mlmeConfirm->DemodMargin;
                    ComplianceTest.NbGateways = mlmeConfirm->NbGateways;
                }
            }
            break;
        }
        default:
            break;
    }
    next_tx = true;
}

void lora_init(LoRaMainCallback_t *callbacks, LoRaParam_t *LoRaParam)
{
    device_state = DEVICE_STATE_INIT;
    LoRaParamInit = LoRaParam;
    LoRaMainCallbacks = callbacks;

#if (STATIC_DEVICE_EUI != 1)
    LoRaMainCallbacks->BoardGetUniqueId( DevEui );
#endif

#if (OVER_THE_AIR_ACTIVATION != 0)

    PRINTF_RAW( "OTAA\r\n" );
    PRINTF_RAW( "DevEui= %02X", DevEui[0] );
    for ( int i = 1; i < 8; i++ )
    {
        PRINTF_RAW( "-%02X", DevEui[i] );
    };
    PRINTF_RAW( "\r\n" );
    PRINTF_RAW( "AppEui= %02X", AppEui[0] );
    for ( int i = 1; i < 8; i++ )
    {
        PRINTF_RAW( "-%02X", AppEui[i] );
    };
    PRINTF_RAW( "\r\n" );
    PRINTF_RAW( "AppKey= %02X", AppKey[0] );
    for ( int i = 1; i < 16; i++ )
    {
        PRINTF_RAW( " %02X", AppKey[i] );
    };
    PRINTF_RAW( "\n\r\n" );
#else

#if (STATIC_DEVICE_ADDRESS != 1)
    // Random seed initialization
    srand1(LoRaMainCallbacks->BoardGetRandomSeed());
    // Choose a random device address
    DevAddr = randr(0, 0x01FFFFFF);
#endif
    PRINTF_RAW("ABP\r\n");
    PRINTF_RAW("DevEui= %02X", DevEui[0]);
    for (int i = 1; i < 8; i++)
    {
        PRINTF_RAW("-%02X", DevEui[i]);
    };
    PRINTF_RAW("\r\n");
    PRINTF_RAW("DevAdd=  %08X\n\r", DevAddr);
    PRINTF_RAW("NwkSKey= %02X", NwkSKey[0]);
    for (int i = 1; i < 16; i++)
    {
        PRINTF_RAW(" %02X", NwkSKey[i]);
    };
    PRINTF_RAW("\r\n");
    PRINTF_RAW("AppSKey= %02X", AppSKey[0]);
    for (int i = 1; i < 16; i++)
    {
        PRINTF_RAW(" %02X", AppSKey[i]);
    };
    PRINTF_RAW("\r\n");
#endif
}

void lora_fsm( void )
{
    switch (device_state) {
        case DEVICE_STATE_INIT:
        {
            LoRaMacPrimitives.MacMcpsConfirm = mcps_confirm;
            LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
            LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
            LoRaMacCallbacks.GetBatteryLevel = LoRaMainCallbacks->BoardGetBatteryLevel;
#if defined(REGION_AS923)
            LoRaMacInitialization(&LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_AS923);
#elif defined(REGION_AS923)
            LoRaMacInitialization(&LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_AU915);
#elif defined(REGION_CN470)
            LoRaMacInitialization(&LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_CN470);
#elif defined(REGION_CN779)
            LoRaMacInitialization(&LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_CN779);
#elif defined(REGION_EU433)
            LoRaMacInitialization(&LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_EU433);
#elif defined(REGION_IN865)
            LoRaMacInitialization(&LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_IN865);
#elif defined(REGION_EU868)
            LoRaMacInitialization(&LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_EU868);
#elif defined(REGION_KR920)
            LoRaMacInitialization(&LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_KR920);
#elif defined(REGION_US915)
            LoRaMacInitialization(&LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_US915);
#elif defined(REGION_US915_HYBRID)
            LoRaMacInitialization(&LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_US915_HYBRID);
#else
#error "Please define a region in the compiler options."
#endif

            TimerInit( &TxNextPacketTimer, on_tx_next_packet_timer_event );

            mibReq.Type = MIB_ADR;
            mibReq.Param.AdrEnable = LoRaParamInit->AdrEnable;
            LoRaMacMibSetRequestConfirm( &mibReq );

            mibReq.Type = MIB_PUBLIC_NETWORK;
            mibReq.Param.EnablePublicNetwork = LoRaParamInit->EnablePublicNetwork;
            LoRaMacMibSetRequestConfirm( &mibReq );

            mibReq.Type = MIB_DEVICE_CLASS;
            mibReq.Param.Class = LoRaParamInit->Class;
            LoRaMacMibSetRequestConfirm( &mibReq );

#if defined(REGION_EU868)
            lora_config_duty_cycle_set(LORAWAN_DUTYCYCLE_ON ? ENABLE : DISABLE);

#if (USE_SEMTECH_DEFAULT_CHANNEL_LINEUP == 1)
            LoRaMacChannelAdd(3, (ChannelParams_t)LC4);
            LoRaMacChannelAdd(4, (ChannelParams_t)LC5);
            LoRaMacChannelAdd(5, (ChannelParams_t)LC6);
            LoRaMacChannelAdd(6, (ChannelParams_t)LC7);
            LoRaMacChannelAdd(7, (ChannelParams_t)LC8);
            LoRaMacChannelAdd(8, (ChannelParams_t)LC9);
            LoRaMacChannelAdd(9, (ChannelParams_t)LC10);

            mibReq.Type = MIB_RX2_DEFAULT_CHANNEL;
            mibReq.Param.Rx2DefaultChannel = (Rx2ChannelParams_t)
            {   869525000, DR_3};
            LoRaMacMibSetRequestConfirm(&mibReq);

            mibReq.Type = MIB_RX2_CHANNEL;
            mibReq.Param.Rx2Channel = (Rx2ChannelParams_t)
            {   869525000, DR_3};
            LoRaMacMibSetRequestConfirm(&mibReq);
#endif

#endif
            // TODO: get config from flash
            //aos_kv_get("lora_cfg", &g_lora_config, sizeof(g_lora_config));
            if (g_lora_config.flag == VALID_JOIN_FLAG) {
                g_join_method = STORED_JOIN_METHOD;
            }
            DeviceState = DEVICE_STATE_JOIN;
            break;
        }

        case DEVICE_STATE_JOIN:
        {
#if (OVER_THE_AIR_ACTIVATION != 0)
            MlmeReq_t mlmeReq;

            mlmeReq.Type = MLME_JOIN;
            mlmeReq.Req.Join.DevEui = DevEui;
            mlmeReq.Req.Join.AppEui = AppEui;
            mlmeReq.Req.Join.AppKey = AppKey;

            mlmeReq.Req.Join.method = g_join_method;
            if (g_join_method == STORED_JOIN_METHOD) {
                mlmeReq.Req.Join.NbTrials = 3;
            } else {
                mlmeReq.Req.Join.NbTrials = 2;
            }

            if ( next_tx == true )
            {
                LoRaMacMlmeRequest(&mlmeReq);
                DBG_LINKLORA("Start to Join,Nbtrials:%d\r\n", mlmeReq.Req.Join.NbTrials);
            }

            DeviceState = DEVICE_STATE_SLEEP;
#else
            mibReq.Type = MIB_NET_ID;
            mibReq.Param.NetID = LORAWAN_NETWORK_ID;
            LoRaMacMibSetRequestConfirm(&mibReq);

            mibReq.Type = MIB_DEV_ADDR;
            mibReq.Param.DevAddr = DevAddr;
            LoRaMacMibSetRequestConfirm(&mibReq);

            mibReq.Type = MIB_NWK_SKEY;
            mibReq.Param.NwkSKey = NwkSKey;
            LoRaMacMibSetRequestConfirm(&mibReq);

            mibReq.Type = MIB_APP_SKEY;
            mibReq.Param.AppSKey = AppSKey;
            LoRaMacMibSetRequestConfirm(&mibReq);

            mibReq.Type = MIB_NETWORK_JOINED;
            mibReq.Param.IsNetworkJoined = true;
            LoRaMacMibSetRequestConfirm(&mibReq);

            DeviceState = DEVICE_STATE_SEND;
#endif
            break;
        }
        case DEVICE_STATE_JOINED:
        {
            DBG_LINKLORA("Joined\n\r");
            //aos_kv_set();
            DeviceState = DEVICE_STATE_SEND;
            break;
        }
        case DEVICE_STATE_SEND:
            {
            if ( next_tx == true )
            {
                prepare_tx_frame( );
                next_tx = send_frame( );
            }
            if ( ComplianceTest.Running == true )
            {
                // Schedule next packet transmission as soon as possible
                TimerSetValue( &TxNextPacketTimer, 5000 ); /* 5s */
                TimerStart( &TxNextPacketTimer );
            }
            else if ( LoRaParamInit->TxEvent == TX_ON_TIMER )
            {
                // Schedule next packet transmission
                TimerSetValue( &TxNextPacketTimer, LoRaParamInit->TxDutyCycleTime );
                TimerStart( &TxNextPacketTimer );
            }

            DeviceState = DEVICE_STATE_SLEEP;
            break;
        }
        case DEVICE_STATE_SLEEP:
            {
            // Wake up through events
            break;
        }
        default:
        {
            DeviceState = DEVICE_STATE_INIT;
            break;
        }
    }
}

DeviceState_t lora_getDeviceState( void )
{
    return DeviceState;
}

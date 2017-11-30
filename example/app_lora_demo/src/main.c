/*
 / _____)             _              | |
 ( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
 (______/|_____)_|_|_| \__)_____)\____)_| |_|
 (C)2013 Semtech

 Description: Generic lora driver implementation

 License: Revised BSD License, see LICENSE.TXT file include in the project

 Maintainer: Miguel Luis, Gregory Cristian and Wael Guibene
 */
/******************************************************************************
 * @file    main.c
 * @author  MCD Application Team
 * @version V1.1.1
 * @date    01-June-2017
 * @brief   this is the main!
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
 * All rights reserved.</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted, provided that the following conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other
 *    contributors to this software may be used to endorse or promote products
 *    derived from this software without specific written permission.
 * 4. This software, including modifications and/or derivative works of this
 *    software, must execute solely and exclusively on microcontroller or
 *    microprocessor devices manufactured by or for STMicroelectronics.
 * 5. Redistribution and use of this software other than as permitted under
 *    this license is void and will automatically terminate your rights under
 *    this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "low_power.h"
#include "lora.h"
#include "timeServer.h"
#include "vcom.h"
#include "version.h"
#include "radio.h"
#include "sx1276Regs-Fsk.h"
#include "sx1276Regs-Lora.h"
#include "delay.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/*!
 * CAYENNE_LPP is myDevices Application server.
 */

/*!
 * Defines the application data transmission duty cycle. 30s, value in [ms].
 */
#define APP_TX_DUTYCYCLE 30000
/*!
 * LoRaWAN Adaptive Data Rate
 * @note Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_ON 1
/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_CONFIRMED_MSG ENABLE
/*!
 * LoRaWAN application port
 * @note do not use 224. It is reserved for certification
 */
#define LORAWAN_APP_PORT 100
/*!
 * Number of trials for the join request.
 */
#define JOINREQ_NBTRIALS 3 //will be changed to 48 in RegionCN470Verify()

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* call back when LoRa will transmit a frame*/
static void LoraTxData( lora_AppData_t *AppData, FunctionalState *IsTxConfirmed );

/* call back when LoRa has received a frame*/
static void LoraRxData( lora_AppData_t *AppData );

/* Private variables ---------------------------------------------------------*/
/* load call backs*/
static LoRaMainCallback_t LoRaMainCallbacks = {
    HW_GetBatteryLevel,
    HW_GetUniqueId,
    HW_GetRandomSeed,
    LoraTxData,
    LoraRxData
};

/**
 * Initialises the Lora Parameters
 */
static LoRaParam_t LoRaParamInit = {
    TX_ON_TIMER,
    APP_TX_DUTYCYCLE,
    CLASS_A,
    LORAWAN_ADR_ON,
    DR_0,
    LORAWAN_PUBLIC_NETWORK,
    JOINREQ_NBTRIALS
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int application_start( void )
{
    /* Configure the hardware*/
    HW_Init( );
    
    /* Configure Debug mode */
    DBG_Init( );
    
    /* USER CODE BEGIN 1 */
    /* USER CODE END 1 */

    /* Configure the Lora Stack*/
    lora_Init( &LoRaMainCallbacks, &LoRaParamInit );
    
    PRINTF("FW VERSION: %s\n\r", AT_VERSION_STRING)
    ;
    PRINTF("LoRaWan VERSION: %s\n\r", AT_VERSION_LORAWAN_STRING)
    ;
    
    /* main loop*/
    while ( 1 )
    {
        /* run the LoRa class A state machine*/
        lora_fsm( );
        
        DISABLE_IRQ();
        /* if an interrupt has occurred after DISABLE_IRQ, it is kept pending
         * and cortex will not enter low power anyway  */
        if ( lora_getDeviceState( ) == DEVICE_STATE_SLEEP )
        {
#ifndef LOW_POWER_DISABLE
            LowPower_Handler( );
#endif
        }
        ENABLE_IRQ();
        /* USER CODE BEGIN 2 */
        /* USER CODE END 2 */
    }
}

static void LoraTxData( lora_AppData_t *AppData, FunctionalState *IsTxConfirmed )
{
    /* USER CODE BEGIN 3 */
    AppData->BuffSize = sprintf( (char *) AppData->Buff, "app lora demo" );
    DBG_PRINTF( "tx: %s\n", AppData->Buff );
    
    AppData->Port = LORAWAN_APP_PORT;
    
    *IsTxConfirmed = LORAWAN_CONFIRMED_MSG;
    
    /* USER CODE END 3 */
}

static void LoraRxData( lora_AppData_t *AppData )
{
    /* USER CODE BEGIN 4 */
    AppData->Buff[AppData->BuffSize] = '\0';
    DBG_PRINTF( "rx: port = %d, len = %d, data = %s\n", AppData->Port, AppData->BuffSize, AppData->Buff );
    switch ( AppData->Port )
    {
        default:
            break;
    }
    /* USER CODE END 4 */
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

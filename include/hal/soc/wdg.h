/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef YOS_WDG_H
#define YOS_WDG_H

#pragma once
#include "common.h"
#include "board_platform.h"

/** @addtogroup MICO_PLATFORM
* @{
*/

/** @defgroup MICO_WDG MICO WDG Driver
* @brief  Hardware watch dog Functions (WDG) Functions
* @note   These Functions are used by MICO's system monitor service, If any defined system monitor 
*          cannot be reloaded for some reason, system monitor service use this hardware watch dog 
*          to perform a system reset. So the watch dog reload time should be greater than system 
*          monitor's refresh time.
* @{
*/

/******************************************************
 *                   Macros
 ******************************************************/  

#define PlatformWDGInitialize   hal_wdg_init /**< For API compatiable with older version */
#define PlatformWDGReload       hal_wdg_reload     /**< For API compatiable with older version */
#define PlatformWDGFinalize     hal_wdg_finalize   /**< For API compatiable with older version */

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                Type Definitions
 ******************************************************/

 /******************************************************
 *                    Structures
 ******************************************************/


/******************************************************
 *                     Variables
 ******************************************************/

/******************************************************
 *              Function Declarations
 ******************************************************/


/**
 * @biref This function will initialize the on board CPU hardware watch dog
 *
 * @param timeout        : Watchdag timeout, application should call hal_wdg_reload befor timeout.
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_wdg_init(uint32_t timeout);

/**
 * @biref Reload watchdog counter.
 *
 * @param     none
 * @return    none
 */
void hal_wdg_reload(void);

/**
 * @biref This function performs any platform-specific cleanup needed for hardware watch dog.
 *
 * @param     none
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_wdg_finalize(void);

/** @} */
/** @} */

#endif



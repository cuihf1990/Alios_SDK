/* MiCO Team
 * Copyright (c) 2017 MXCHIP Information Tech. Co.,Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *               Function Declarations
 ******************************************************/

/**
  * @brief  Add iperf command lines to MiCO CLI.
  * @param  none.
  * @retval kNoErr is returned on success, otherwise, kXXXErr is returned.
  */
OSStatus iperf_cli_register( void );

#ifdef __cplusplus
} /*extern "C" */
#endif

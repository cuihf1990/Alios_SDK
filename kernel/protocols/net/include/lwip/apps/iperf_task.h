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

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/* for iperf task */
#define IPERF_NAME "iperf"
#define IPERF_STACKSIZE 1536
#define IPERF_PRIO 6

#define IPERF_COMMAND_BUFFER_NUM (18)
#define IPERF_COMMAND_BUFFER_SIZE (20) // 4 bytes align


/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

void iperf_udp_run_server(char *parameters[]);
void iperf_tcp_run_server(char *parameters[]);
void iperf_udp_run_client(char *parameters[]);
void iperf_tcp_run_client(char *parameters[]);

void iperf_get_current_time(uint32_t *s, uint32_t *ms);


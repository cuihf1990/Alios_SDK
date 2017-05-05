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

#ifndef UR_CLI_SERIAL_H
#define UR_CLI_SERIAL_H

typedef void (* cli_input_t)(const uint8_t *buf, uint16_t length);

ur_error_t ur_cli_start(cli_input_t input);
ur_error_t ur_cli_output(const uint8_t *buf, uint16_t length);

#endif  /* UR_CLI_SERIAL_H */


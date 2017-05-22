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

#ifndef YOC_SYS_VERSION
#define YOC_SYS_VERSION

const char *get_yoc_product_name(void);
const char *get_yoc_factory_id(void);
const char *get_yoc_product_model(void);
const char *get_yoc_device_type(void);
const char *get_yoc_terminal_type(void);
const char *get_yoc_os_version (void);
const char *get_yoc_os_internal_version(void);
const char *get_yoc_product_internal_type(void);

#endif /* YOC_SYS_VERSION */


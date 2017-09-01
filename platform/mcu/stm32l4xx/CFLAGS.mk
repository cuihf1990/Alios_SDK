##
 # Copyright (C) 2016 YunOS Project. All rights reserved.
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #   http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
##

L_PATH := $(call cur-dir)

CSP_CFLAGS += \
    -mcpu=cortex-m4     \
    -mthumb             \
    -mfloat-abi=soft    \
    -ffunction-sections \
    -fdata-sections

CSP_CFLAGS += \
    -Iid2kernel/rhino/arch/armv7-m/gcc/m4

CSP_CFLAGS += \
    -Iid2kernel/tools/ysh/include

CSP_LDFLAGS := \
    -mcpu=cortex-m4  \
    -mthumb          \
    -mfloat-abi=soft \
    -lc


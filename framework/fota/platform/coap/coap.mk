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
include $(BUILD_MODULE)
NAME := coap_ota

UTIL_SOURCE := ../../../../utility/iotx-utils

ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
PLATFORM_COAP := linux
else ifeq ($(findstring mk3060, $(BUILD_STRING)), mk3060)
PLATFORM_COAP := rhino
endif

$(NAME)_INCLUDES :=  \
    ./src \
    ../ \
    $(UTIL_SOURCE)/misc \
    $(UTIL_SOURCE)/sdk-impl \
    $(UTIL_SOURCE)/sdk-impl/exports \
    $(UTIL_SOURCE)/sdk-impl/imports \
    $(UTIL_SOURCE)/LITE-log \
    $(UTIL_SOURCE)/LITE-utils \
    $(UTIL_SOURCE)/digest

$(NAME)_SOURCES := \
    ./src/ota.c \
    ./src/ota_lib.c \
    $(UTIL_SOURCE)/hal/$(PLATFORM_COAP)/HAL_OS_$(PLATFORM_COAP).c

$(NAME)_COMPONENTS += connectivity.coap
$(NAME)_SOURCES += ota_service_coap.c
$(NAME)_DEFINES += OTA_CH_SIGNAL_COAP

$(NAME)_COMPONENTS += utility.iotx-utils.misc
$(NAME)_COMPONENTS += utility.iotx-utils.sdk-impl
$(NAME)_CFLAGS := \
    -Werror \
    -Wno-unused-function \
    -Wno-implicit-function-declaration
#$(filter-out -Werror,$(CFLAGS))

$(NAME)_DEFINES += DEBUG
GLOBAL_DEFINES      += IOTX_DEBUG

ifeq ($(CONFIG_COAP_ONLINE), y)
$(NAME)_DEFINES += COAP_ONLINE
endif
ifeq ($(CONFIG_COAP_DTLS_SUPPORT), y)
$(NAME)_DEFINES += COAP_DTLS_SUPPORT
endif

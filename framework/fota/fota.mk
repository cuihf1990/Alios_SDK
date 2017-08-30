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

NAME := fota

$(NAME)_CFLAGS += -Wall -Werror

$(NAME)_SOURCES += \
    ota_service.c

ifeq ($(CONFIG_OTA_CH),mqtt)
$(NAME)_COMPONENTS += fota.coap_mqtt
endif
ifeq ($(CONFIG_OTA_CH),coap)
$(NAME)_COMPONENTS += fota.coap_mqtt
endif
ifeq ($(CONFIG_OTA_CH),alink)
$(NAME)_COMPONENTS += fota.alink
endif

$(NAME)_INCLUDES := \
    ./

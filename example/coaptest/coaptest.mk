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

NAME := coaptest

$(NAME)_SOURCES     := coaptest.c
GLOBAL_DEFINES      += ALIOT_DEBUG

CONFIG_COAP_DTLS_SUPPORT := y
#CONFIG_COAP_ONLINE := y

ifeq ($(CONFIG_COAP_DTLS_SUPPORT), y)
$(NAME)_DEFINES += COAP_DTLS_SUPPORT
endif
ifeq ($(CONFIG_COAP_ONLINE), y)
$(NAME)_DEFINES += COAP_ONLINE
endif


$(NAME)_COMPONENTS  := cli
#ifeq ($(LWIP),1)
#$(NAME)_COMPONENTS  += protocols.net
#endif

$(NAME)_CFLAGS += \
    -Wno-unused-function \
    -Wno-implicit-function-declaration \
    -Wno-unused-function \
#    -Werror

$(NAME)_INCLUDES    := \
    ../../framework/coap \
    ../../framework/coap/iot-coap-c/ \
    ../../utility/iotx-utils/sdk-impl \
    ../../utility/iotx-utils/sdk-impl/imports \
    ../../utility/iotx-utils/sdk-impl/exports

$(NAME)_COMPONENTS  += connectivity.coap

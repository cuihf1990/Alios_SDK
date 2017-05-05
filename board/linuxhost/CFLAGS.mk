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

CSP_LDFLAGS += -m32

CSP_CFLAGS += -O0 -gstabs -m32
CSP_CFLAGS += -Wall
ifneq ($(gcov),n)
CSP_CFLAGS += -fprofile-arcs -ftest-coverage
CSP_CFLAGS += -DENABLE_GCOV
endif
CSP_CFLAGS += -DCSP_LINUXHOST

ifeq ($(M), 1)
CSP_CFLAGS += \
    -fsanitize=address \
    -fno-omit-frame-pointer \
    -ggdb
endif

ifeq ($(shell uname), Linux)
CSP_CFLAGS += -rdynamic
endif

CSP_CFLAGS += -I$(L_PATH)/include
CSP_CFLAGS += -I$(L_PATH)/ydk/src/core/include
CSP_CFLAGS += -I$(L_PATH)/include/yoc

ifeq ($(CONFIG_YOC_NET_PROTOCOL), y)
CSP_CFLAGS += -I$(YOC_BASE_PATH)/id2kernel/netproto/include
ifeq ($(CONFIG_YOC_RHINO), y)
CSP_CFLAGS += -I$(YOC_BASE_PATH)/id2kernel/netproto/port/include/
CSP_CFLAGS += -I$(YOC_BASE_PATH)/targets/linuxhost/csp/lwip/include/
CSP_CFLAGS += -DWITH_LWIP
else
CSP_CFLAGS += -I$(L_PATH)/csp/lwip/include/
endif
CSP_CFLAGS += -DCYASSL_LWIP
endif

ifeq ($(CONFIG_YOC_URADAR_MESH), y)
CSP_CFLAGS += -I$(YOC_BASE_PATH)/id2kernel/uradar/mesh/include
endif



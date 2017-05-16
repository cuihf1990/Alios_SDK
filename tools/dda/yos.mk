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

include $(DEFINE_LOCAL)

L_MODULE := libdda

#L_EXCLUDE_TARGET := true

L_INCS := id2kernel/rhino/arch/$(CONFIG_CHIP_ARCH)
L_INCS += id2kernel/uradar/mesh/include
L_INCS += id2kernel/sys/include

L_CFLAGS := -Werror

L_SRCS += \
    eloop.c \
    agent.c \
    msg.c \
    hal.c

ifeq ($(CONFIG_YOS_DDM), y)
L_SRCS += config_parser.c
L_SRCS += master.c
L_SRCS += dgraph.c
endif

L_INCS += services/framework/include

ifeq ($(CONFIG_YOS_DDM_DG), y)
L_CFLAGS += -DCONFIG_DDA_DGRAPH_ENABLE
endif

include $(BUILD_MODULE)


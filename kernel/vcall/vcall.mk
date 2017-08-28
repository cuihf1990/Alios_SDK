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

NAME := vcall

$(NAME)_TYPE := kernel
GLOBAL_INCLUDES += ./mico/include

$(NAME)_CFLAGS += -Wall -Werror
ifeq ($(HOST_ARCH),ARM968E-S)
$(NAME)_CFLAGS += -marm
endif

ifneq ($(vcall),linux)
GLOBAL_DEFINES += VCALL_RHINO
$(NAME)_COMPONENTS += rhino

$(NAME)_SOURCES := \
    mico/mico_rhino.c

$(NAME)_SOURCES += \
    yos/yos_rhino.c
else
GLOBAL_DEFINES += VCALL_LINUX

$(NAME)_SOURCES += \
    yos/yos_linux.c
endif


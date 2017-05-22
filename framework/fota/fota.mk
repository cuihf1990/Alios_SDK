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

$(NAME)_SOURCES += \
    ota_update_packet.c \
    ota_util.c \
    ota_transport.c \
    platform/ota_transport_platform.c \
    ota_update_manifest.c \
    md5_sum.c \
    md5.c \
    version.c \
    osupdate_service.c


$(NAME)_INCLUDES := \
    ./ \
    ../../utility/cjson/include \
    ../../include/hal \
    ./platform/

ifneq ($(CONFIG_YOS_SECURITY),n)
$(NAME)_INCLUDES += \
    security/tfs/include
endif


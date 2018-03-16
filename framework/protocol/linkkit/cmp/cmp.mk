
NAME := cmp
$(NAME)_TYPE := framework
GLOBAL_INCLUDES +=  ./ ./inc
$(NAME)_INCLUDES += ../../protocol/alink-ilop/sdk-encap  ../../protocol/alink-ilop/iotkit-system

$(NAME)_SOURCES     := src/iotx_cmp_api.c src/iotx_cmp_common.c src/iotx_cmp_log.c src/iotx_cmp_mqtt_direct.c src/iotx_cmp_ota.c

#defalut gcc
ifeq ($(COMPILER),)
$(NAME)_CFLAGS  += -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
$(NAME)_CFLAGS  += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized
$(NAME)_CFLAGS  += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable
$(NAME)_CFLAGS  += -Wno-unused-value -Wno-strict-aliasing
else ifeq ($(COMPILER),gcc)
$(NAME)_CFLAGS  += -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
$(NAME)_CFLAGS  += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized
$(NAME)_CFLAGS  += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable
$(NAME)_CFLAGS  += -Wno-unused-value -Wno-strict-aliasing
endif

$(NAME)_COMPONENTS := framework.common 
GLOBAL_DEFINES += MQTT_DIRECT MQTT_COMM_ENABLED CMP_VIA_MQTT_DIRECT OTA_SIGNAL_CHANNEL=1 CMP_SUPPORT_MEMORY_MAGIC

NAME := alink-ilop
$(NAME)_TYPE := framework
GLOBAL_INCLUDES += sdk-encap \
                  sdk-encap/imports \
                  iotkit-system \
                  base/log/LITE-log  \
                  base/utils  \
                  base/utils/include \
                  layers/config/include \
                  iotkit-system \
                          

include $(ALIOS_PATH)/framework/protocol/alink-ilop/base/log/iot-log.mk
include $(ALIOS_PATH)/framework/protocol/alink-ilop/base/utils/iot-utils.mk
include $(ALIOS_PATH)/framework/protocol/alink-ilop/hal-impl/iot-hal.mk
include $(ALIOS_PATH)/framework/protocol/alink-ilop/iotkit-system/iotkit-system.mk
include $(ALIOS_PATH)/framework/protocol/alink-ilop/layers/iot-layers.mk

ifeq ($(COMPILER),gcc)
$(NAME)_CFLAGS  += -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
$(NAME)_CFLAGS  += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized
$(NAME)_CFLAGS  += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable
$(NAME)_CFLAGS  += -Wno-unused-value -Wno-strict-aliasing
endif


$(NAME)_COMPONENTS += digest_algorithm cjson base64 hashtable log yloop modules.fs.kv cloud hal mbedtls
GLOBAL_DEFINES += CONFIG_ALINK_ILOP

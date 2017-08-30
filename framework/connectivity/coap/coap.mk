NAME := coap

ROOT_DIR := ../../../utility/iotx-utils

$(NAME)_INCLUDES :=  \
    ./ \
    ./iot-coap-c \
    $(ROOT_DIR)/sdk-impl \
    $(ROOT_DIR)/sdk-impl/imports \
    $(ROOT_DIR)/sdk-impl/exports \
    $(ROOT_DIR)/LITE-log \
    $(ROOT_DIR)/LITE-utils \
    $(ROOT_DIR)/digest \
    $(ROOT_DIR)/mbedtls-lib/include
    

$(NAME)_SOURCES := iotx_ca_cert.c iotx_coap_api.c iotx_hmac.c iotx_product_linux.c\
    iot-coap-c/CoAPDeserialize.c \
    iot-coap-c/CoAPExport.c \
    iot-coap-c/CoAPMessage.c \
    iot-coap-c/CoAPNetwork.c \
    iot-coap-c/CoAPSerialize.c \
    $(ROOT_DIR)/hal/HAL_UDP_linux.c \
    $(ROOT_DIR)/hal/HAL_OS_linux.c

$(NAME)_COMPONENTS += utility.iotx-utils.LITE-utils
$(NAME)_COMPONENTS += utility.iotx-utils.digest
#ifeq ($(CONFIG_COAP_ONLINE), y)
#$(NAME)_DEFINES += COAP_ONLINE
#endif
ifeq ($(CONFIG_COAP_DTLS_SUPPORT), y)
$(NAME)_DEFINES += COAP_DTLS_SUPPORT
$(NAME)_COMPONENTS += utility.iotx-utils.mbedtls-lib
endif

# TODO: fix warnings
$(NAME)_CFLAGS := $(filter-out -Werror,$(CFLAGS))

$(NAME)_DEFINES += DEBUG
# PKG_UPDATE  := 'git@gitlab.alibaba-inc.com:iot-middleware/iot-coap-c.git'

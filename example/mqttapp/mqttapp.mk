NAME := mqttapp

$(NAME)_SOURCES     := mqtt-example.c
GLOBAL_DEFINES      += ALIOT_DEBUG IOTX_DEBUG



#$(NAME)_INCLUDES := ../../framework/protocol/alink/json/

$(NAME)_INCLUDES  += ../../framework/mqtt/platform/ \
../../framework/mqtt/utils \
../../framework/mqtt/mqtt \
../../framework/mqtt/guider \
../../framework/mqtt/system-mqtt \
../../framework/mqtt/shadow \
../../framework/mqtt/packages-mqtt \
../../framework/mqtt/sdk-impl \
../../framework/mqtt/sdk-impl/imports \
../../framework/mqtt/utils/digest \
../../framework/mqtt/utils/misc
#$(NAME)_COMPONENTS := log connectivity protocol.alink.json
$(NAME)_COMPONENTS := log
$(NAME)_COMPONENTS  += mbedtls
$(NAME)_COMPONENTS  += mqtt

LWIP := 1
ifeq ($(LWIP),1)
$(NAME)_COMPONENTS  += protocols.net
no_with_lwip := 0
endif

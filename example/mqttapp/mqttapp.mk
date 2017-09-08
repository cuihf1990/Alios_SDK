NAME := mqttapp

GLOBAL_DEFINES      += ALIOT_DEBUG IOTX_DEBUG
CONFIG_OTA_CH = mqtt
ifeq ($(findstring b_l475e, $(BUILD_STRING)), b_l475e)
$(NAME)_SOURCES     := mqtt-example-b_l475e.c
else
$(NAME)_SOURCES     := mqtt-example.c
endif

$(NAME)_COMPONENTS := connectivity.mqtt

LWIP := 0
ifeq ($(LWIP),1)
$(NAME)_COMPONENTS  += protocols.net
no_with_lwip := 0
endif

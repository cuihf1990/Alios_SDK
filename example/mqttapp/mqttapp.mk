NAME := mqttapp

GLOBAL_DEFINES      += ALIOT_DEBUG IOTX_DEBUG
CONFIG_OTA_CH = mqtt


$(NAME)_SOURCES     := mqtt-example.c

$(NAME)_COMPONENTS := connectivity.mqtt

LWIP := 0
ifeq ($(LWIP),1)
$(NAME)_COMPONENTS  += protocols.net
no_with_lwip := 0
endif

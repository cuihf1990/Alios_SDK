NAME := mqtt

GLOBAL_INCLUDES += ./
$(NAME)_SOURCES += mqtt_client.c   mqtt_instance.c
$(NAME)_INCLUDES += ../../protocol/alink-ilop/sdk-encap  ../../protocol/alink-ilop/iotkit-system

ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
PLATFORM_MQTT := linux
else 
PLATFORM_MQTT := rhino
endif

GLOBAL_DEFINES += MQTT_COMM_ENABLED  CMP_VIA_MQTT_DIRECT MQTT_DIRECT
$(NAME)_CFLAGS    += -DOTA_SIGNAL_CHANNEL=1 

$(NAME)_COMPONENTS := connectivity.mqtt.MQTTPacket
$(NAME)_COMPONENTS += mbedtls



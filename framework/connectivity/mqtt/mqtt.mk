NAME := mqtt

$(NAME)_COMPONENTS := connectivity.mqtt.MQTTPacket
$(NAME)_INCLUDES := ../../../utility/iotx-utils/digest ../../../utility/iotx-utils/guider ../../../utility/iotx-utils/hal ../../../utility/iotx-utils/LITE-log ../../../utility/iotx-utils/LITE-utils ../../../utility/iotx-utils/misc ../../../utility/iotx-utils/sdk-impl ../../../utility/iotx-utils/device
$(NAME)_SOURCES := mqtt_client.c


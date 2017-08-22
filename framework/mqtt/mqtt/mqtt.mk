NAME := mqtt_rn

$(NAME)_COMPONENTS := mqtt.mqtt.MQTTPacket
$(NAME)_INCLUDES := ../platform/ ../utils/digest/ ../utils/misc/ ../system-mqtt/ ../packages-mqtt/LITE-log/ ../packages-mqtt/LITE-utils/ ../sdk-impl/ ../guider/ ../security-mqtt/
$(NAME)_SOURCES := mqtt_client.c


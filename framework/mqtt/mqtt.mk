NAME := mqtt

$(NAME)_COMPONENTS := mqtt.mqtt
$(NAME)_COMPONENTS += mqtt.guider
$(NAME)_COMPONENTS += mqtt.security-mqtt
$(NAME)_COMPONENTS += mqtt.shadow
$(NAME)_COMPONENTS += mqtt.system-mqtt
$(NAME)_COMPONENTS += mqtt.utils
$(NAME)_COMPONENTS += mqtt.platform
$(NAME)_COMPONENTS += mqtt.packages-mqtt
$(NAME)_COMPONENTS += mqtt.sdk-impl
#$(NAME)_COMPONENTS += mqtt.platform mqtt.packages mqtt.sdk-impl
$(NAME)_INCLUDES := os/

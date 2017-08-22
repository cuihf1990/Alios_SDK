NAME := platform
$(NAME)_INCLUDES := ssl/mbedtls/mbedtls/include/ os/ network/ ssl/ ../utils ./ ../sdk-impl/imports/ ../sdk-impl/

ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
PLATFORM_MQTT := linux
NETWORK_MQTT := linuxsock
else ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
PLATFORM_MQTT := linux
NETWORK_MQTT := linuxsock
else ifeq ($(findstring mk3060, $(BUILD_STRING)), mk3060)
PLATFORM_MQTT := rhino
NETWORK_MQTT := rhinosock
endif

$(NAME)_SOURCES := os/linux/HAL_OS_linux.c
$(NAME)_SOURCES += os/linux/HAL_TCP_linux.c
#$(NAME)_SOURCES := os/linux/HAL_UDP_linux.c


#$(NAME)_SOURCES := os/$(PLATFORM_MQTT)/HAL_OS_$(PLATFORM_MQTT).c
#$(NAME)_SOURCES := os/$(PLATFORM_MQTT)/HAL_TCP_$(PLATFORM_MQTT).c
#$(NAME)_SOURCES := os/$(PLATFORM_MQTT)/HAL_UDP_$(PLATFORM_MQTT).c
#$(NAME)_SOURCES += network/$(NETWORK_MQTT)/$(NETWORK_MQTT).c


#mbedtls
$(NAME)_SOURCES += ssl/mbedtls/HAL_DTLS_mbedtls.c
$(NAME)_SOURCES += ssl/mbedtls/HAL_TLS_mbedtls.c
#$(NAME)_COMPONENTS += mqtt.platform_rn.ssl.mbedtls.mbedtls
#protocol.alink.os.platform

#ssl

#$(NAME)_SOURCES += ssl/openssl/openssl.c
#$(NAME)_COMPONENTS += mqtt.platform_rn.ssl.openssl


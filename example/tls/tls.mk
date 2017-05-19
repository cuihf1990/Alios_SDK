NAME := ssl_client

$(NAME)_SOURCES     := ssl_client.c
$(NAME)_COMPONENTS  := mbedtls

ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
$(NAME)_DEFINES     += CONFIG_LINUX_HOST
else
$(error "not find correct platform!")
endif


NAME := ssl_client

$(NAME)_SOURCES     := ssl_client.c
$(NAME)_COMPONENTS  := mbedtls

ifeq ($(findstring b_l475e, $(BUILD_STRING)), b_l475e)
$(NAME)_DEFINES     := MBEDTLS_NET_ALT_UART

endif


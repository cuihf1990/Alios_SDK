#NAME := iotkit-system

$(NAME)_SOURCES  +=  hal-impl/rhino/HAL_OS_rhino.c
$(NAME)_SOURCES  +=  hal-impl/rhino/HAL_TCP_rhino.c
$(NAME)_SOURCES  +=  hal-impl/rhino/HAL_PRODUCT_rhino.c
$(NAME)_SOURCES  +=  hal-impl/rhino/HAL_UDP_rhino.c


$(NAME)_SOURCES  += hal-impl/tls/mbedtls/HAL_DTLS_mbedtls.c
$(NAME)_SOURCES  += hal-impl/tls/mbedtls/HAL_TLS_mbedtls.c
$(NAME)_INCLUDES += ./

$(NAME)_COMPONENTS += mbedtls

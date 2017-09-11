NAME := otatest

#CONFIG_COAP_DTLS_SUPPORT := y
#CONFIG_COAP_ONLINE := y

$(NAME)_COMPONENTS  := cli
#ifeq ($(LWIP),1)
#$(NAME)_COMPONENTS  += protocols.net
#endif

CONFIG_OTA_CH := coap
$(NAME)_SOURCES     := ota_coap_test.c

$(NAME)_CFLAGS += \
    -Wno-unused-function \
    -Wno-implicit-function-declaration \
    -Wno-unused-function \
    -Werror

$(NAME)_COMPONENTS  += fota.platform.coap

NAME := platform
$(NAME)_SOURCES = linux_awss.c linux_hardware.c yos_os.c linux_ota.c yos_ssl.c linux_network.c
$(NAME)_INCLUDES := ../product/
#$(NAME)_LDFLAGS += -lssl -lcrypto

$(NAME)_COMPONENTS  := mbedtls

NAME := platform
$(NAME)_SOURCES = linux_awss.c linux_hardware.c yos_os.c linux_ota.c linux_network.c
ifneq (,$(filter mbedtls,$(COMPONENTS)))
$(NAME)_SOURCES += yos_ssl.c
else
$(NAME)_SOURCES += linux_ssl.c
endif

$(NAME)_INCLUDES := ../product/

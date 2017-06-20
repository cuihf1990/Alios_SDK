NAME := platform

ifneq (,$(filter mk108 mk3060,$(findstring mk108, $(BUILD_STRING)) $(findstring mk3060, $(BUILD_STRING))))
$(NAME)_SOURCES =  yos_awss.c yos_hardware.c yos_os.c yos_network.c yos_ssl.c
else
$(NAME)_SOURCES = yos_awss.c linux_hardware.c yos_os.c linux_ota.c linux_network.c
ifneq (,$(filter mbedtls,$(COMPONENTS)))
$(NAME)_SOURCES += yos_ssl.c
else
$(NAME)_SOURCES += linux_ssl.c
endif
endif

$(NAME)_INCLUDES := ../product/

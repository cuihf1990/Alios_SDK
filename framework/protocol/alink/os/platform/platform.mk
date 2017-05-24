NAME := platform
$(NAME)_SOURCES = linux_awss.c linux_hardware.c linux_os.c linux_ota.c linux_ssl.c linux_network.c 
$(NAME)_includes := ../product/
$(NAME)_LDFLAGS += -lssl -lcrypto

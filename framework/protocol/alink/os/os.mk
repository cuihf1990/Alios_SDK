GLOBAL_INCLUDES  += os
$(NAME)_INCLUDES += os/product/ os/platform/

$(NAME)_SOURCES  += os/os_misc.c os/os_thread.c
$(NAME)_SOURCES  += os/product/product.c

ifneq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
$(NAME)_SOURCES  += os/platform/yos_awss.c os/platform/yos_hardware.c os/platform/yos_os.c os/platform/yos_network.c os/platform/yos_ssl.c
else
$(NAME)_SOURCES  += os/platform/yos_awss.c os/platform/linux_hardware.c os/platform/yos_os.c os/platform/linux_ota.c os/platform/yos_network.c
$(NAME)_SOURCES  += os/platform/yos_ssl.c
endif                                      

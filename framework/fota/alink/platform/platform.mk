NAME := fota_platform

$(NAME)_SOURCES := ota_platform_os.c
GLOBAL_INCLUDES += ./

ifneq (,$(filter protocol.alink,$(COMPONENTS)))
$(NAME)_CFLAGS += -Wall -Werror
$(NAME)_COMPONENTS += fota.alink.platform.alink
else
ifneq (,$(filter mqtt,$(COMPONENTS)))   
+$(NAME)_COMPONENTS += fota.alink.platform.aliot 
else
$(NAME)_COMPONENTS += fota.alink.platform.common
endif
endif

NAME := netmgr

ifneq (,$(ssid))
$(NAME)_DEFINES += WIFI_SSID=\"$(ssid)\"
$(NAME)_DEFINES += WIFI_PWD=\"$(pwd)\"
endif

$(NAME)_SOURCES := netmgr.c
$(NAME)_CFLAGS += -Wall -Werror

GLOBAL_INCLUDES += include

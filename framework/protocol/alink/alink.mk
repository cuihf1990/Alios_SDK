NAME := alink

$(NAME)_SOURCES  := service_manager.c
$(NAME)_INCLUDES := os/
$(NAME)_INCLUDES += system/
$(NAME)_INCLUDES += json/

$(NAME)_INCLUDES += .

include framework/protocol/alink/accs/accs.mk
include framework/protocol/alink/json/json.mk

$(NAME)_COMPONENTS := protocol.alink.cota  protocol.alink.system
$(NAME)_COMPONENTS += protocol.alink.os
$(NAME)_COMPONENTS += protocol.alink.devmgr protocol.alink.msdp

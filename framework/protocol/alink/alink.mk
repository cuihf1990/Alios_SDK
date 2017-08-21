NAME := alink

$(NAME)_SOURCES  := service_manager.c

GLOBAL_INCLUDES += .

include framework/protocol/alink/accs/accs.mk
include framework/protocol/alink/json/json.mk
include framework/protocol/alink/system/system.mk
include framework/protocol/alink/cota/cota.mk
include framework/protocol/alink/os/os.mk


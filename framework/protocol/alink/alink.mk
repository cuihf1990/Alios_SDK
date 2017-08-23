NAME := alink

GLOBAL_INCLUDES += .
$(NAME)_SOURCES  := service_manager.c

include framework/protocol/alink/accs/accs.mk
include framework/protocol/alink/json/json.mk
include framework/protocol/alink/system/system.mk
include framework/protocol/alink/cota/cota.mk
include framework/protocol/alink/os/os.mk

$(NAME)_COMPONENTS := connectivity.wsf digest_algorithm cjson base64 hashtable log

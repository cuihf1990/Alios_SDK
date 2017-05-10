NAME := alink

$(NAME)_SOURCES := service_manager.c 
$(NAME)_COMPONENTS := protocol.alink.cota protocol.alink.accs protocol.alink.system
$(NAME)_COMPONENTS += protocol.alink.json protocol.alink.digest_algorithm
$(NAME)_COMPONENTS += protocol.alink.os
$(NAME)_INCLUDES := os/

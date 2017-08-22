NAME := msdp

GLOBAL_INCLUDES += .

$(NAME)_SOURCES := msdp.c msdp_common.c msdp_zigbee.c msdp_gateway.c

$(NAME)_CFLAGS += -DGATEWAY_SDK


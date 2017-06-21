NAME := msdp

GLOBAL_INCLUDES += ./

$(NAME)_SOURCES := msdp.c msdp_common.c msdp_zigbee.c msdp_gateway.c
$(NAME)_INCLUDES := ../ ../system/ ../json/ ../os/ ../accs/ ../devmgr/
$(NAME)_INCLUDES +=  ../../../../framework/connectivity/ ../../../../framework/connectivity/wsf/ ../../../ywss/
$(NAME)_INCLUDES += ../../../../utility/digest_algorithm/ ../../../gateway/
$(NAME)_CFLAGS += -DGATEWAY_SDK


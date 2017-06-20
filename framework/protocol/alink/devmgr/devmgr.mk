NAME := devmgr

GLOBAL_INCLUDES += ./

$(NAME)_SOURCES := devmgr_alink.c devmgr_common.c devmgr.c devmgr_cache.c devmgr_router.c
$(NAME)_INCLUDES := ../ ../system/ ../os/ ../json/ ../accs/ ../stdd/
$(NAME)_INCLUDES +=  ../../../../framework/connectivity/ ../../../../framework/connectivity/wsf/ ../../../ywss/
$(NAME)_INCLUDES += ../../../../utility/digest_algorithm/
$(NAME)_CFLAGS += -DGATEWAY_SDK


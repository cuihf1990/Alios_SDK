NAME := devmgr

GLOBAL_INCLUDES += .

$(NAME)_SOURCES := devmgr_alink.c devmgr_common.c devmgr.c devmgr_cache.c devmgr_router.c

$(NAME)_CFLAGS += -DGATEWAY_SDK


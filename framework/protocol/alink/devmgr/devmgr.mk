NAME := devmgr 

$(NAME)_SOURCES := devmgr_alink.c	devmgr.c  devmgr_cache.c  devmgr_common.c  devmgr_router.c  devmgr_ut.c 
$(NAME)_INCLUDES := ../ ../system/ ../digest_algorithm/ ../json/ ../../../../framework/connectivity/
$(NAME)_INCLUDES += ../../../../framework/connectivity/wsf/ ../ ../../../ywss/ ../os/ ../../../ywss/ 
$(NAME)_INCLUDES += ../../../../utility/digest_algorithm/ 


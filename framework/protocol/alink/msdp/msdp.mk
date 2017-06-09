NAME := msdp 

$(NAME)_SOURCES := msdp.c	msdp_common.c  msdp_gateway.c  msdp_ut.c  msdp_zigbee.c 
$(NAME)_INCLUDES := ../ ../system/ ../digest_algorithm/ ../json/ ../../../../framework/connectivity/
$(NAME)_INCLUDES += ../../../../framework/connectivity/wsf/ ../ ../../../ywss/ ../os/ ../../../ywss/ 
$(NAME)_INCLUDES += ../../../../utility/digest_algorithm/ 


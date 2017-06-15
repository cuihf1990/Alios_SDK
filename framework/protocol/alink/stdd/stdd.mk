NAME := stdd 

$(NAME)_SOURCES := attrs_profile_mgr.c  stdd.c  stdd_datatype.c  stdd_lua.c  stdd_parser.c  stdd_ut.c  stdd_zigbee.c  
$(NAME)_INCLUDES := ../ ../system/ ../digest_algorithm/ ../json/ ../../../../framework/connectivity/
$(NAME)_INCLUDES += ../../../../framework/connectivity/wsf/ ../ ../../../ywss/ ../os/ ../../../ywss/ 
$(NAME)_INCLUDES += ../../../../utility/digest_algorithm/ 


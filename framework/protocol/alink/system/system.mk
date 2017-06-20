NAME := system

$(NAME)_SOURCES := alink_export.c config.c device.c
$(NAME)_INCLUDES := ./ ../ ../accs/ ../os ../cota ../json/ ../msdp ../devmgr ../stdd/
$(NAME)_INCLUDES += ../../../../framework/connectivity/ ../../../../utility/digest_algorithm/
$(NAME)_CFLAGS += -DGATEWAY_SDK

NAME := alink_transport

$(NAME)_SOURCES := ota_transport.c
GLOBAL_INCLUDES += ./
$(NAME)_INCLUDES := ../../../protocol/alink/system/ \
                    ../../../protocol/alink/os/ \
                    ../../ \
                    ../

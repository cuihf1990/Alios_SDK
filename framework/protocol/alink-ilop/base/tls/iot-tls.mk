NAME := iot-tls

SOURCE_DIR = 
$(NAME)_SOURCES     := $(SOURCE_DIR)/

$(NAME)_CFLAGS += \
    -Wno-unused-function \
    -Wno-implicit-function-declaration \
    -Wno-unused-function \
#    -Werror

$(NAME)_INCLUDES    := \
                    ./

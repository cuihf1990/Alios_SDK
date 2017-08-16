NAME := kernel_hal

$(NAME)_CFLAGS += -Wall -Werror
$(NAME)_SOURCES     := wifi.c
$(NAME)_SOURCES     += ota.c

NAME := fota_socket

$(NAME)_TYPE := kernel
GLOBAL_INCLUDES += ./

ifeq ($(STM32_NONSTD_SOCKET), true)
$(NAME)_CFLAGS += -Wall -Werror
$(NAME)_COMPONENTS += fota.download.http.socket.stm32wifi
else
$(NAME)_CFLAGS += -Wall -Werror
$(NAME)_COMPONENTS += fota.download.http.socket.stand
endif


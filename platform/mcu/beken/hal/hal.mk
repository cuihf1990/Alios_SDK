NAME := hal

$(NAME)_SOURCES	 := flash.c \
					uart.c \
					ringbuf.c

$(NAME)_INCLUDES := . \
					../beken7231/beken378/driver/include

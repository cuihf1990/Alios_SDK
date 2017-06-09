NAME := hal

$(NAME)_SOURCES	 := uart.c \
					ringbuf.c

$(NAME)_INCLUDES := . \
					../beken7231/beken378/driver/include

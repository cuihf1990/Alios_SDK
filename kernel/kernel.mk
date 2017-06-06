
NAME = kernel

$(NAME)_COMPONENTS += platform/mcu/$(HOST_MCU_FAMILY)
# $(NAME)_COMPONENTS += kernel/rhino/test

ifeq ($(valgrind), 1)
GLOBAL_CFLAGS += $(shell [ -f /usr/include/valgrind/valgrind.h ] && echo -DHAVE_VALGRIND_VALGRIND_H)
GLOBAL_CFLAGS += $(shell [ -f /usr/include/valgrind.h ] && echo -DHAVE_VALGRIND_H)
endif


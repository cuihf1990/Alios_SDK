
NAME = kernel_kernel

$(NAME)_COMPONENTS += platform/mcu/$(HOST_MCU_FAMILY)

ifeq ($(HOST_MCU_FAMILY), linux)
$(NAME)_COMPONENTS += kernel/rhino/test
endif

$(NAME)_COMPONENTS += vcall

ifeq ($(BINS), 1)
$(NAME)_COMPONENTS += syscall usyscall
GLOBAL_CFLAGS += -DYOS_BINS
endif

ifeq ($(valgrind), 1)
GLOBAL_CFLAGS += $(shell [ -f /usr/include/valgrind/valgrind.h ] && echo -DHAVE_VALGRIND_VALGRIND_H)
GLOBAL_CFLAGS += $(shell [ -f /usr/include/valgrind.h ] && echo -DHAVE_VALGRIND_H)
endif


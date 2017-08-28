NAME = kernel

$(NAME)_TYPE := kernel
$(NAME)_COMPONENTS += platform/mcu/$(HOST_MCU_FAMILY)

$(NAME)_COMPONENTS += vcall

ifeq ($(BINS), 1)
$(NAME)_COMPONENTS += syscall usyscall
GLOBAL_CFLAGS += -DYOS_BINS
endif


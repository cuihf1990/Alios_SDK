NAME := activation

GLOBAL_INCLUDES := .

ifeq ($(HOST_ARCH),ARM968E-S)
$(NAME)_PREBUILT_LIBRARY := ./ARM968E-S/activation.a
endif

ifeq ($(HOST_ARCH),Cortex-M4)
ifeq ($(COMPILER),armcc)
$(NAME)_PREBUILT_LIBRARY := ./Cortex-M4/KEIL/activation.a
else ifeq ($(COMPILER),iar)
$(NAME)_PREBUILT_LIBRARY := ./Cortex-M4/IAR/activation.a
else
$(NAME)_PREBUILT_LIBRARY := ./Cortex-M4/activation.a
endif
endif

ifeq ($(HOST_ARCH),linux)
$(NAME)_PREBUILT_LIBRARY := ./linux/activation.a
endif

ifeq ($(HOST_ARCH),xtensa)
$(NAME)_PREBUILT_LIBRARY := ./xtensa/activation.a
endif


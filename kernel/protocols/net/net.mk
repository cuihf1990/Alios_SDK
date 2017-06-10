NAME := net

include kernel/protocols/net/Filelists.mk
GLOBAL_INCLUDES += include port/include
GLOBAL_DEFINES += WITH_LWIP

$(NAME)_INCLUDES += port/include

$(NAME)_SOURCES := $(COREFILES)
$(NAME)_SOURCES += $(CORE4FILES)
$(NAME)_SOURCES += $(CORE6FILES)
$(NAME)_SOURCES += $(APIFILES)
$(NAME)_SOURCES += $(NETIFFILES)

ifneq (,$(filter linuxhost,$(COMPONENTS)))
$(NAME)_SOURCES += port/sys_arch.c
endif

ifneq (,$(filter armhflinux,$(COMPONENTS)))
$(NAME)_SOURCES += port/sys_arch.c
endif

ifneq (,$(filter mk108,$(COMPONENTS)))
$(NAME)_SOURCES += port/sys_arch.c
endif

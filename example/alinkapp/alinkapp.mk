NAME := alinkapp

$(NAME)_SOURCES := alink_sample.c
$(NAME)_COMPONENTS := base64 hashtable log connectivity protocol.alink modules.kv cli

ifneq ($(ywss),0)
$(NAME)_COMPONENTS += ywss
endif

ifneq (,$(filter linuxhost,$(COMPONENTS)))
gateway ?= 0
else
gateway ?= 1
endif

ifeq ($(gateway),1)

$(NAME)_COMPONENTS += gateway
ifneq (,$(filter linuxhost,$(COMPONENTS)))
DDA ?= 1
endif

ifneq (,$(filter armhflinux,$(COMPONENTS)))
DDA ?= 1
endif

endif

ifeq ($(LWIP),1)
$(NAME)_COMPONENTS  += protocols.net
no_with_lwip := 0
endif

$(NAME)_INCLUDES := ../../framework/protocol/alink/system/ ../../framework/protocol/alink/json/
$(NAME)_INCLUDES += ../../kernel/modules/kv/include
$(NAME)_INCLUDES += ../../framework/gateway/
GLOBAL_CFLAGS  += -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
GLOBAL_CFLAGS  += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized 
GLOBAL_CFLAGS  += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable 
GLOBAL_CFLAGS  += -Wno-unused-value -Wno-strict-aliasing

ifeq ($(DDA),1)
GLOBAL_LDFLAGS += -lreadline -lncurses
$(NAME)_COMPONENTS  += dda
endif

ifeq ($(sds),1)
GLOBAL_DEFINES += CONFIG_SDS
endif

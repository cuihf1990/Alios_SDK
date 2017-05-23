NAME := alinkapp

$(NAME)_SOURCES := alink_sample.c
$(NAME)_COMPONENTS  := base64 hashtable log connectivity ywss protocol.alink modules.kv netmgr
$(NAME)_INCLUDES := ../../framework/protocol/alink/system/ ../../framework/protocol/alink/json/
$(NAME)_INCLUDES += ../../kernel/modules/kv/include  
GLOBAL_LDFLAGS += -lssl
GLOBAL_DEFINES += DEBUG 
GLOBAL_CFLAGS  += -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
GLOBAL_CFLAGS  += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized 
GLOBAL_CFLAGS  += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable 
GLOBAL_CFLAGS  += -Wno-unused-value -Wno-strict-aliasing -Wno-unused-result

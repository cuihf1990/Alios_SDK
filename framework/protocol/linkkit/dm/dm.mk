
NAME := dm
$(NAME)_TYPE := framework
GLOBAL_INCLUDES += ./include ./include/interface
$(NAME)_INCLUDES += ../../protocol/alink-ilop/sdk-encap  ../../protocol/alink-ilop/iotkit-system ../../protocol/cmp/

#$(NAME)_SOURCES     := src/dm_cJSON.c src/cmp_abstract_impl.c src/cmp_message_info.c src/dm_impl.c src/dm_thing.c src/dm_thing_manager.c src/logger.c src/new.c src/single_list.c
$(NAME)_SOURCES     := src/cmp_abstract_impl.c src/cmp_message_info.c src/dm_impl.c src/dm_thing.c src/dm_thing_manager.c src/logger.c src/new.c src/single_list.c

#defalut gcc
ifeq ($(COMPILER),)
$(NAME)_CFLAGS  += -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
$(NAME)_CFLAGS  += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized
$(NAME)_CFLAGS  += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable
$(NAME)_CFLAGS  += -Wno-unused-value -Wno-strict-aliasing
else ifeq ($(COMPILER),armcc)
$(NAME)_CFLAGS  += -D__LONG_LONG_MAX__=LLONG_MAX
else ifeq ($(COMPILER),iar)
$(NAME)_CFLAGS  += -D__LONG_LONG_MAX__=LLONG_MAX
else ifeq ($(COMPILER),gcc)
$(NAME)_CFLAGS  += -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
$(NAME)_CFLAGS  += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized
$(NAME)_CFLAGS  += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable
$(NAME)_CFLAGS  += -Wno-unused-value -Wno-strict-aliasing
endif

$(NAME)_COMPONENTS := framework.common 
GLOBAL_DEFINES += USING_UTILS_JSON LITE_THING_MODEL NDEBUG

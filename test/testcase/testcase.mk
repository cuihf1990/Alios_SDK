NAME := testcase

GLOBAL_INCLUDES     += include

$(NAME)_SOURCES     := yts_main.c
$(NAME)_SOURCES     += basic_test.c
$(NAME)_SOURCES     += hal_test.c
$(NAME)_SOURCES     += framework/yloop_test.c 
$(NAME)_SOURCES     += utility/cjson_test.c
$(NAME)_SOURCES     += tfs/tfs_test.c
$(NAME)_SOURCES     += kernel/rhino/rhino_test.c
$(NAME)_SOURCES     += kernel/rhino/arch/linux/port_test.c
$(NAME)_COMPONENTS  := yunit cjson

ifneq (,$(filter protocols.mesh,$(COMPONENTS)))
$(NAME)_INCLUDES := ../../kernel/protocols/mesh/include
$(NAME)_INCLUDES += ../../tools/dda
include test/testcase/kernel/protocols/mesh/filelists.mk
$(NAME)_SOURCES += $(MESHYTSFILE)
endif

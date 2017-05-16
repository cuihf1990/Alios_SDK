NAME := testcase

$(NAME)_SOURCES     := yts_main.c
$(NAME)_SOURCES     += basic_test.c
$(NAME)_SOURCES     += hal_test.c
$(NAME)_SOURCES     += framework/yloop_test.c 
$(NAME)_SOURCES     += utility/cjson_test.c
$(NAME)_SOURCES     += kernel/rhino/rhino_test.c
$(NAME)_SOURCES     += kernel/rhino/arch/linux/port_test.c
$(NAME)_COMPONENTS  := yunit cjson

GLOBAL_INCLUDES     += include


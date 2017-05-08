NAME := testcase

$(NAME)_SOURCES     := yts_main.c
$(NAME)_SOURCES     += basic_test.c
$(NAME)_SOURCES     += framework/yloop_test.c 

$(NAME)_COMPONENTS  := yunit cjson

GLOBAL_INCLUDES     += include


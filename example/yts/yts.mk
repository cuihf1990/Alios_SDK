NAME := yts

$(NAME)_SOURCES := main.c
$(NAME)_COMPONENTS := testcase

GLOBAL_CFLAGS  += -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
GLOBAL_CFLAGS  += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized 
GLOBAL_CFLAGS  += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable 
GLOBAL_CFLAGS  +=  -Wno-unused-value -Wno-strict-aliasing
GLOBAL_DEFINES += CONFIG_USE_DEF_AP

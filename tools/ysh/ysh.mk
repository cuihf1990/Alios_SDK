NAME := ysh

$(NAME)_SOURCES     := ysh.c
$(NAME)_SOURCES     += ysh_history.c
$(NAME)_SOURCES     += ysh_register.c
$(NAME)_SOURCES     += cmd/ysh_help.c
$(NAME)_SOURCES     += cmd/ysh_dumpsys.c

$(NAME)_INCLUDES     += cmd/
GLOBAL_INCLUDES     += include


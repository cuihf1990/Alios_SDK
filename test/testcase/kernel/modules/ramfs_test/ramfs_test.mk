NAME := ramfs_test

$(NAME)_COMPONENTS  += modules.fs.ramfs

$(NAME)_SOURCES     += ramfs_test.c

$(NAME)_CFLAGS      += -Wall -Werror


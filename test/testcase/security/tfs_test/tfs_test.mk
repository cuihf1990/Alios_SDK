NAME := tfs_test

$(NAME)_COMPONENTS  += tfs libid2 libkm plat_gen 

$(NAME)_SOURCES     += tfs_test.c

$(NAME)_CFLAGS      += -Wall -Werror -Wno-pointer-sign


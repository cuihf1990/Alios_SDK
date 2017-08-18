NAME := vfs

$(NAME)_TYPE 	    := kernel
$(NAME)_SOURCES     := vfs.c
$(NAME)_SOURCES     += device.c
$(NAME)_SOURCES     += vfs_inode.c
$(NAME)_SOURCES     += vfs_driver.c

$(NAME)_DEFINES     += IO_NEED_TRAP

GLOBAL_INCLUDES     += include

NAME := vfs

$(NAME)_SOURCES     := vfs.c
$(NAME)_SOURCES     += device.c
$(NAME)_SOURCES     += vfs_inode.c
$(NAME)_SOURCES     += vfs_driver.c

GLOBAL_INCLUDES     += include

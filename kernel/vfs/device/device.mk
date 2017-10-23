NAME := vfs_device

$(NAME)_SOURCES     += vfs_adc.c

$(NAME)_INCLUDES += ../include/device/ \
                   ../include/  \
                   ../../hal/soc/
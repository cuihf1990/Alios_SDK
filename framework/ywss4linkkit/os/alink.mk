HDR_REFS := utility

LIBA_TARGET := libos.a

LIB_HEADERS := \
    product/product.h \
    platform/platform.h \

LIB_SRCS := $(wildcard *.c)

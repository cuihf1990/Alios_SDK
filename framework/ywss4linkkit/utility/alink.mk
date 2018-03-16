HDR_REFS := os service

LIBA_TARGET := libutility.a
LIB_SRCS := \
    $(wildcard *.c) \
    $(wildcard digest_algorithm/digest_algorithm.c) \

LIB_HEADERS := \
    json_parser.h \

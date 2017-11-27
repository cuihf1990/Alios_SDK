NAME := salapp

vcall ?= posix

$(NAME)_SOURCES := salapp.c

$(NAME)_COMPONENTS += netmgr sal atparser cli

sal := 1

GLOBAL_DEFINES += DEBUG

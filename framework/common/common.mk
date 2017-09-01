NAME := framework

$(NAME)_SOURCES    := main.c version.c

$(NAME)_COMPONENTS := yloop vfs log fota modules.fs.kv cloud

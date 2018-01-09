NAME := spiffs

$(NAME)_TYPE        := kernel

$(NAME)_SOURCES     += spiffs/spiffs_cache.c
$(NAME)_SOURCES     += spiffs/spiffs_check.c
$(NAME)_SOURCES     += spiffs/spiffs_gc.c
$(NAME)_SOURCES     += spiffs/spiffs_hydrogen.c
$(NAME)_SOURCES     += spiffs/spiffs_nucleus.c

#default gcc
ifeq ($(COMPILER),)
$(NAME)_CFLAGS      += -Wall -Werror
else ifeq ($(COMPILER),gcc)
$(NAME)_CFLAGS      += -Wall -Werror
endif

GLOBAL_INCLUDES     += include spiffs/include
GLOBAL_DEFINES      += AOS_SPIFFS

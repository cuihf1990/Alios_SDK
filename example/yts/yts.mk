NAME := yts

$(NAME)_SOURCES := main.c
$(NAME)_COMPONENTS := protocols.net protocols.mesh dda testcase  rhino.test

$(NAME)_CFLAGS += -Wall -Werror

GLOBAL_LDFLAGS += -lreadline -lncurses
GLOBAL_DEFINES += CONFIG_AOS_MESHYTS DEBUG

ifneq (,${BINS})
GLOBAL_CFLAGS += -DSYSINFO_OS_BINS
endif
CURRENT_TIME = $(shell /bin/date +%Y%m%d.%H%M)
CONFIG_SYSINFO_APP_VERSION = APP-1.0.0-$(CURRENT_TIME)
$(info app_version:${CONFIG_SYSINFO_APP_VERSION})
GLOBAL_CFLAGS += -DSYSINFO_APP_VERSION=\"$(CONFIG_SYSINFO_APP_VERSION)\"

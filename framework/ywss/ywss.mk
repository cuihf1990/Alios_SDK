NAME := ywss

GLOBAL_INCLUDES += .
$(NAME)_SOURCES := awss.c enrollee.c sha256.c zconfig_utils.c zconfig_ieee80211.c wifimgr.c ywss_utils.c
$(NAME)_SOURCES += zconfig_ut_test.c registrar.c zconfig_protocol.c zconfig_vendor_common.c

$(NAME)_INCLUDES += ../gateway/

$(NAME)_DEFINES += DEBUG

GLOBAL_DEFINES += CONFIG_YWSS

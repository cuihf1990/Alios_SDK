NAME := ywss
$(NAME)_SOURCES := awss.c enrollee.c sha256.c zconfig_utils.c zconfig_ieee80211.c
$(NAME)_SOURCES += zconfig_ut_test.c registrar.c zconfig_protocol.c zconfig_vendor_common.c
$(NAME)_INCLUDES := ../protocol/alink/os/ ../protocol/alink/ ../../utility/base64/ ../protocol/alink/accs/
$(NAME)_INCLUDES += ../protocol/alink/json/ ../connectivity/wsf/ ../../utility/digest_algorithm/

GLOBAL_DEFINES += CONFIG_YWSS

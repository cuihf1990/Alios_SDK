NAME := ywss4linkkit

$(NAME)_DEFINES := USE_LPTHREAD

$(NAME)_COMPONENTS := digest_algorithm protocol.alink-ilop connectivity.link-coap connectivity.mqtt

$(NAME)_TYPE := framework
$(NAME)_INCLUDES += . ./os/ ./os/product/ ./os/platform/ ./utility/
$(NAME)_SOURCES :=
$(NAME)_SOURCES += awss.c           awss_cmp_mqtt.c  awss_report.c   HAL_AWSS_rhino.c     registrar.c          zconfig_protocol.c
$(NAME)_SOURCES += awss_cmp.c       awss_main.c      awss_wifimgr.c  sha256.c             zconfig_utils.c
$(NAME)_SOURCES += awss_cmp_coap.c  awss_notify.c    enrollee.c      passwd.c             zconfig_ieee80211.c  zconfig_vendor_common.c
$(NAME)_SOURCES +=  os/os_misc.c utility/log.c
$(NAME)_SOURCES += CoAPServer.c
#DIGEST_DIR := utility/digest_algorithm
#$(NAME)_SOURCES += $(DIGEST_DIR)/digest_algorithm.c $(DIGEST_DIR)/hmac.c  $(DIGEST_DIR)/md5.c  $(DIGEST_DIR)/sha2.c

ifeq ($(awss_ble),1)
$(NAME)_SOURCES += awss_blefi.c blefi_config.c
GLOBAL_DEFINES += CONFIG_AWSS_BLE
endif

$(NAME)_DEFINES += DEBUG

ifeq ($(COMPILER),)
$(NAME)_CFLAGS  += -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
$(NAME)_CFLAGS  += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized
$(NAME)_CFLAGS  += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable
$(NAME)_CFLAGS  += -Wno-unused-value -Wno-strict-aliasing
else ifeq ($(COMPILER),gcc)
$(NAME)_CFLAGS  += -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
$(NAME)_CFLAGS  += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized
$(NAME)_CFLAGS  += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable
$(NAME)_CFLAGS  += -Wno-unused-value -Wno-strict-aliasing
endif

GLOBAL_DEFINES += CONFIG_YWSS

ifeq ($(simulate_netmgr),1)
GLOBAL_DEFINES += SIMULATE_NETMGR
GLOBAL_CFLAGS += -DNETMGR_SSID=\"Yuemewifi-3766\" -DNETMGR_PW=\"aos12345\"
endif

ifeq ($(auto_netmgr),0)
GLOBAL_DEFINES += AUTO_NETMGR_DISABLE
endif

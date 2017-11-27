NAME := testcase

GLOBAL_INCLUDES     += include

$(NAME)_COMPONENTS  := yunit testcase.basic_test

# utility testcase
$(NAME)_COMPONENTS  += testcase.utility.cjson_test
$(NAME)_COMPONENTS  += testcase.utility.digest_algorithm_test
$(NAME)_COMPONENTS  += testcase.utility.hashtable_test

# kernel testcase
$(NAME)_COMPONENTS  += testcase.kernel.vfs_test
$(NAME)_COMPONENTS  += testcase.kernel.yloop_test
$(NAME)_COMPONENTS  += testcase.kernel.modules.kv_test
$(NAME)_COMPONENTS  += testcase.kernel.rhino_test
$(NAME)_COMPONENTS  += testcase.kernel.vcall_test

$(NAME)_COMPONENTS  += testcase.framework.wifi_hal_test

ifneq (,$(findstring linux, $(BUILD_STRING)))
$(NAME)_COMPONENTS  += testcase.kernel.modules.fatfs_test
$(NAME)_COMPONENTS  += testcase.kernel.deviceIO_test

# framework testcase
$(NAME)_COMPONENTS  += testcase.framework.alink_test
$(NAME)_COMPONENTS  += testcase.framework.coap_test
$(NAME)_COMPONENTS  += testcase.framework.fota_test
$(NAME)_COMPONENTS  += testcase.framework.mqtt_test
$(NAME)_COMPONENTS  += testcase.framework.netmgr_test
$(NAME)_COMPONENTS  += testcase.kernel.protocols.mesh_test

# security testcase
$(NAME)_COMPONENTS  += testcase.security.tfs_test
$(NAME)_COMPONENTS  += testcase.security.tls_test
$(NAME)_COMPONENTS  += testcase.security.alicrypto_test

# aosapi testcase
$(NAME)_COMPONENTS  += testcase.aosapi.api_test
endif

$(NAME)_SOURCES     := yts_main.c

$(NAME)_CFLAGS      += -Wall -Werror


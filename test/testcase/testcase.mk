NAME := testcase

GLOBAL_INCLUDES     += include

$(NAME)_SOURCES     := yts_main.c
$(NAME)_COMPONENTS  := yunit cjson

#YTS_COAP
ifeq ($(yts_connectivity), coap)
$(NAME)_DEFINES += YTS_COAP
$(NAME)_SOURCES     += framework/coap_test.c

CONFIG_COAP_DTLS_SUPPORT := y
#CONFIG_COAP_ONLINE := y

ifeq ($(CONFIG_COAP_DTLS_SUPPORT), y)
$(NAME)_DEFINES += COAP_DTLS_SUPPORT
endif
ifeq ($(CONFIG_COAP_ONLINE), y)
$(NAME)_DEFINES += COAP_ONLINE
endif

$(NAME)_COMPONENTS  += connectivity.coap

#YTS_COAP
else
#YTS_MQTT
ifeq ($(yts_connectivity), mqtt)
$(NAME)_DEFINES += YTS_MQTT
$(NAME)_SOURCES     += framework/mqtt_test.c
$(NAME)_COMPONENTS  += connectivity.mqtt
else
$(NAME)_SOURCES     += basic_test.c
$(NAME)_SOURCES     += framework/hal/hal_test.c
$(NAME)_SOURCES     += framework/yloop_test.c
$(NAME)_SOURCES     += framework/fota_test.c
$(NAME)_SOURCES     += framework/vfs_test.c
$(NAME)_SOURCES     += framework/netmgr_test.c 
$(NAME)_SOURCES     += utility/cjson_test.c
$(NAME)_SOURCES     += utility/hashtable_test.c
$(NAME)_SOURCES     += utility/digest_algorithm.c
$(NAME)_SOURCES     += tls/tls_test.c
$(NAME)_SOURCES     += tfs/tfs_test.c
$(NAME)_SOURCES     += kernel/rhino/rhino_test.c
$(NAME)_SOURCES     += kernel/rhino/arch/linux/port_test.c
$(NAME)_SOURCES     += kernel/module/kv_test.c
$(NAME)_SOURCES     += kernel/module/fatfs_test.c
$(NAME)_SOURCES     += kernel/vcall/vcall_test.c
$(NAME)_SOURCES     += devices/vflash/vflash_test.c

$(NAME)_SOURCES    += aosapi/yos/kernel/test_kernel.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_sys_test.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_task_test.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_mm_test.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_mutex_test.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_queue_test.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_sem_test.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_timer_test.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_workqueue_test.c

ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
# only for for linux host now
$(NAME)_SOURCES    += alicrypto/alicrypto_test.c
endif

ifeq ($(findstring armhflinux, $(BUILD_STRING)), armhflinux)
# only for for linux host now
$(NAME)_SOURCES    += alicrypto/alicrypto_test.c
endif

$(NAME)_COMPONENTS  += mbedtls

$(NAME)_COMPONENTS  += base64 hashtable log connectivity.wsf ywss protocol.alink modules.fs.kv modules.fs.fatfs

ifneq (,$(filter protocols.mesh,$(COMPONENTS)))
$(NAME)_INCLUDES := ../../kernel/protocols/mesh/include
$(NAME)_INCLUDES += ../../tools/dda
include test/testcase/kernel/protocols/mesh/filelists.mk
$(NAME)_SOURCES += $(MESHYTSFILE)
endif

#YTS_MQTT
endif
#YTS_COAP
endif

$(NAME)_CFLAGS  += -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
$(NAME)_CFLAGS  += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized
$(NAME)_CFLAGS  += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable
$(NAME)_CFLAGS  += -Wno-unused-value -Wno-strict-aliasing

$(NAME)_INCLUDES += ../../framework/protocol/alink/system/
$(NAME)_INCLUDES += ../../framework/fota/platform/alink/

$(NAME)_INCLUDES += ../../utility/iotx-utils/sdk-impl

NAME := testcase

GLOBAL_INCLUDES     += include

$(NAME)_SOURCES     := yts_main.c
$(NAME)_SOURCES     += basic_test.c
$(NAME)_SOURCES     += framework/hal/hal_test.c
$(NAME)_SOURCES     += framework/yloop_test.c
$(NAME)_SOURCES     += framework/fota_test.c
$(NAME)_SOURCES     += framework/vfs_test.c
$(NAME)_SOURCES     += framework/netmgr_test.c 
$(NAME)_SOURCES     += utility/cjson_test.c
$(NAME)_SOURCES     += utility/hashtable_test.c
$(NAME)_SOURCES     += tfs/tfs_test.c
$(NAME)_SOURCES     += kernel/rhino/rhino_test.c
$(NAME)_SOURCES     += kernel/rhino/arch/linux/port_test.c
$(NAME)_SOURCES     += kernel/module/kv_test.c
$(NAME)_SOURCES     += kernel/vcall/vcall_test.c

$(NAME)_SOURCES    += aosapi/yos/kernel/test_kernel.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_sys_test.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_task_test.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_mm_test.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_mutex_test.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_queue_test.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_sem_test.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_timer_test.c
$(NAME)_SOURCES    += aosapi/yos/kernel/yos_workqueue_test.c

$(NAME)_COMPONENTS  := yunit cjson

$(NAME)_COMPONENTS  += base64 hashtable log connectivity ywss protocol.alink modules.kv

ifneq (,$(filter protocols.mesh,$(COMPONENTS)))
$(NAME)_INCLUDES := ../../kernel/protocols/mesh/include
$(NAME)_INCLUDES += ../../tools/dda
include test/testcase/kernel/protocols/mesh/filelists.mk
$(NAME)_SOURCES += $(MESHYTSFILE)
endif
$(NAME)_INCLUDES += ../../framework/protocol/alink/system/
GLOBAL_LDFLAGS += -lssl

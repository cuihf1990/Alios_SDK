MiCO_SDK_MAKEFILES           		+= ././kernel/rhino/test/test.mk ././platform/mcu/linux/linux.mk ./kernel/vcall/vcall.mk ./test/yunit/yunit.mk ./tools/ysh/ysh.mk ./utility/log/log.mk ./framework/hal/hal.mk ./framework/wrapper/wrapper.mk ./framework/vfs/vfs.mk ./framework/yloop/yloop.mk ././kernel/kernel.mk ./kernel/rhino/rhino.mk ./board/linuxhost/linuxhost.mk ./example/linuxapp/linuxapp.mk
TOOLCHAIN_NAME            		:= HOSTGCC
MiCO_SDK_LDFLAGS             		+= -lpthread -lm -lcrypto
RESOURCE_CFLAGS					+= -g -c -W -Os -fno-builtin
MiCO_SDK_LINK_SCRIPT         		+= 
MiCO_SDK_LINK_SCRIPT_CMD    	 	+= 
MiCO_SDK_PREBUILT_LIBRARIES 	 	+= 
MiCO_SDK_CERTIFICATES       	 	+= 
MiCO_SDK_PRE_APP_BUILDS      		+= 
MiCO_SDK_LINK_FILES          		+=                            
MiCO_SDK_INCLUDES           	 	+=           -I././kernel/rhino/test/./ -I././platform/mcu/linux/. -I./kernel/vcall/./mico/include -I./test/yunit/include -I./tools/ysh/include -I./framework/vfs/include -I./kernel/rhino/core/include -I./board/linuxhost/include -I./include -I./example/linuxapp
MiCO_SDK_DEFINES             		+=       -DRTOS_rhino=1 -Drhino_VERSION_MAJOR= -Drhino_VERSION_MINOR= -Drhino_VERSION_REVISION= -DHAVE_RHINO_KERNEL -DCONFIG_YOC_RHINO_MMREGION
COMPONENTS                		:= linuxapp board_linuxhost rhino kernel yloop vfs wrapper hal log ysh yunit vcall linuximpl test
BUS                       		:= 
IMAGE_TYPE                		:= ram
NETWORK_FULL              		:= 
RTOS_FULL                 		:= rhino vcall vcall rhino
PLATFORM_DIRECTORY        		:= linuxhost
APP_FULL                  		:= linuxapp
NETWORK                   		:= 
RTOS                      		:= rhino
PLATFORM                  		:= linuxhost
HOST_MCU_FAMILY                  	:= linux
USB                       		:= 
APP                       		:= linuxapp
HOST_OPENOCD              		:= linux
JTAG              		        := jlink_swd
HOST_ARCH                 		:= linux
NO_BUILD_BOOTLOADER           	:= 
NO_BOOTLOADER_REQUIRED         	:= 
linuxapp_LOCATION         := ./example/linuxapp/
board_linuxhost_LOCATION         := ./board/linuxhost/
rhino_LOCATION         := ./kernel/rhino/
kernel_LOCATION         := ././kernel/
yloop_LOCATION         := ./framework/yloop/
vfs_LOCATION         := ./framework/vfs/
wrapper_LOCATION         := ./framework/wrapper/
hal_LOCATION         := ./framework/hal/
log_LOCATION         := ./utility/log/
ysh_LOCATION         := ./tools/ysh/
yunit_LOCATION         := ./test/yunit/
vcall_LOCATION         := ./kernel/vcall/
linuximpl_LOCATION         := ././platform/mcu/linux/
test_LOCATION         := ././kernel/rhino/test/
linuxapp_SOURCES          += main.c
board_linuxhost_SOURCES          += main/arg_options.c main/main.c main/hw.c main/sensor.c main/crypto_impl.c csp/csp_rhino.c soc/soc_impl.c soc/hook_impl.c soc/ysh_impl.c
rhino_SOURCES          += core/k_err.c core/k_mm_bestfit.c core/k_mm_region.c core/k_obj_set.c core/k_ringbuf.c core/k_stats.c core/k_task_sem.c core/k_timer.c core/k_buf_queue.c core/k_event.c core/k_mm_blk.c core/k_mutex.c core/k_pend.c core/k_sched.c core/k_sys.c core/k_tick.c core/k_workqueue.c core/k_dyn_mem_proc.c core/k_idle.c core/k_mm_firstfit.c core/k_obj.c core/k_queue.c core/k_sem.c core/k_task.c core/k_time.c
kernel_SOURCES          += 
yloop_SOURCES          += yloop.c local_event.c
vfs_SOURCES          += vfs.c device.c vfs_inode.c vfs_driver.c
wrapper_SOURCES          += csp.c api.c
hal_SOURCES          += hal.c crypto.c flash.c mesh.c random.c sensor.c time.c wifi.c
log_SOURCES          += log.c
ysh_SOURCES          += ysh.c ysh_history.c ysh_register.c cmd/ysh_help.c
yunit_SOURCES          += yunit.c
vcall_SOURCES          += mico/rtos_rhino.c
linuximpl_SOURCES          += cpu_impl.c cpu_longjmp_32.S
test_SOURCES          += test_fw.c test_self_entry.c core/buf_queue/buf_queue_del.c core/buf_queue/buf_queue_dyn_create.c core/buf_queue/buf_queue_flush.c core/buf_queue/buf_queue_info_get.c core/buf_queue/buf_queue_recv.c core/buf_queue/buf_queue_test.c core/event/event_break.c core/event/event_opr.c core/event/event_param.c core/event/event_reinit.c core/event/event_test.c core/mm_blk/mm_blk_break.c core/mm_blk/mm_blk_fragment.c core/mm_blk/mm_blk_opr.c core/mm_blk/mm_blk_param.c core/mm_blk/mm_blk_reinit.c core/mm_blk/mm_blk_test.c core/mutex/mutex_opr.c core/mutex/mutex_param.c core/mutex/mutex_reinit.c core/mutex/mutex_test.c core/queue/queue_all_send.c core/queue/queue_back_send.c core/queue/queue_del.c core/queue/queue_flush.c core/queue/queue_front_send.c core/queue/queue_info_get.c core/queue/queue_is_full.c core/queue/queue_nowait_recv.c core/queue/queue_notify_set.c core/queue/queue_test.c core/sem/sem_break.c core/sem/sem_count.c core/sem/sem_param.c core/sem/sem_reinit.c core/sem/sem_test.c core/sem/sem_opr.c core/sys/sys_opr.c core/sys/sys_test.c core/task/task_sleep.c core/task/task_del.c core/task/task_suspend_test.c core/task/task_test.c core/task/task_yield_test.c core/task/task_misc_test.c core/task_sem/tasksem_count.c core/task_sem/tasksem_opr.c core/task_sem/tasksem_param.c core/task_sem/tasksem_test.c core/time/time_opr.c core/time/time_test.c core/timer/timer_change.c core/timer/timer_create_del.c core/timer/timer_dyn_create_del.c core/timer/timer_start_stop.c core/timer/timer_test.c core/workqueue/workqueue_test.c core/workqueue/workqueue_interface.c core/mm_region/mm_region_break.c core/mm_region/mm_region_test.c core/ringbuf/ringbuf_break.c core/ringbuf/ringbuf_test.c core/combination/comb_test.c core/combination/sem_event.c core/combination/sem_queue_buf.c core/combination/sem_mutex.c core/combination/comb_all.c core/combination/mutex_queue_buf.c core/combination/sem_queue.c core/combination/mutex_queue.c
linuxapp_CHECK_HEADERS    += 
board_linuxhost_CHECK_HEADERS    += 
rhino_CHECK_HEADERS    += 
kernel_CHECK_HEADERS    += 
yloop_CHECK_HEADERS    += 
vfs_CHECK_HEADERS    += 
wrapper_CHECK_HEADERS    += 
hal_CHECK_HEADERS    += 
log_CHECK_HEADERS    += 
ysh_CHECK_HEADERS    += 
yunit_CHECK_HEADERS    += 
vcall_CHECK_HEADERS    += 
linuximpl_CHECK_HEADERS    += 
test_CHECK_HEADERS    += 
linuxapp_INCLUDES         := 
board_linuxhost_INCLUDES         := -I./board/linuxhost/. -I./board/linuxhost/.
rhino_INCLUDES         := 
kernel_INCLUDES         := 
yloop_INCLUDES         := 
vfs_INCLUDES         := 
wrapper_INCLUDES         := 
hal_INCLUDES         := 
log_INCLUDES         := 
ysh_INCLUDES         := -I./tools/ysh/cmd/
yunit_INCLUDES         := 
vcall_INCLUDES         := 
linuximpl_INCLUDES         := 
test_INCLUDES         := 
linuxapp_DEFINES          := 
board_linuxhost_DEFINES          := 
rhino_DEFINES          := 
kernel_DEFINES          := 
yloop_DEFINES          := 
vfs_DEFINES          := 
wrapper_DEFINES          := 
hal_DEFINES          := 
log_DEFINES          := 
ysh_DEFINES          := 
yunit_DEFINES          := 
vcall_DEFINES          := 
linuximpl_DEFINES          := 
test_DEFINES          := 
linuxapp_CFLAGS           :=            -g -c -W -Os -fno-builtin    -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
board_linuxhost_CFLAGS           :=            -g -c -W -Os -fno-builtin    -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
rhino_CFLAGS           :=            -g -c -W -Os -fno-builtin    -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
kernel_CFLAGS           :=            -g -c -W -Os -fno-builtin    -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
yloop_CFLAGS           :=            -g -c -W -Os -fno-builtin    -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" -std=c99
vfs_CFLAGS           :=            -g -c -W -Os -fno-builtin    -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
wrapper_CFLAGS           :=            -g -c -W -Os -fno-builtin    -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
hal_CFLAGS           :=            -g -c -W -Os -fno-builtin    -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
log_CFLAGS           :=            -g -c -W -Os -fno-builtin    -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
ysh_CFLAGS           :=            -g -c -W -Os -fno-builtin    -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
yunit_CFLAGS           :=            -g -c -W -Os -fno-builtin    -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
vcall_CFLAGS           :=            -g -c -W -Os -fno-builtin    -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
linuximpl_CFLAGS           :=            -g -c -W -Os -fno-builtin    -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
test_CFLAGS           :=            -g -c -W -Os -fno-builtin    -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
linuxapp_CXXFLAGS         :=                -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
board_linuxhost_CXXFLAGS         :=                -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
rhino_CXXFLAGS         :=                -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
kernel_CXXFLAGS         :=                -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
yloop_CXXFLAGS         :=                -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
vfs_CXXFLAGS         :=                -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
wrapper_CXXFLAGS         :=                -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
hal_CXXFLAGS         :=                -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
log_CXXFLAGS         :=                -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
ysh_CXXFLAGS         :=                -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
yunit_CXXFLAGS         :=                -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
vcall_CXXFLAGS         :=                -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
linuximpl_CXXFLAGS         :=                -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
test_CXXFLAGS         :=                -DMiCO_SDK_VERSION_MAJOR=3 -DMiCO_SDK_VERSION_MINOR=2 -DMiCO_SDK_VERSION_REVISION=3 -DBUS=\"\" -Iout/linuxapp@linuxhost/resources/ -DPLATFORM=\"linuxhost\" 
linuxapp_ASMFLAGS         :=               
board_linuxhost_ASMFLAGS         :=               
rhino_ASMFLAGS         :=               
kernel_ASMFLAGS         :=               
yloop_ASMFLAGS         :=               
vfs_ASMFLAGS         :=               
wrapper_ASMFLAGS         :=               
hal_ASMFLAGS         :=               
log_ASMFLAGS         :=               
ysh_ASMFLAGS         :=               
yunit_ASMFLAGS         :=               
vcall_ASMFLAGS         :=               
linuximpl_ASMFLAGS         :=               
test_ASMFLAGS         :=               
linuxapp_RESOURCES        := 
board_linuxhost_RESOURCES        := 
rhino_RESOURCES        := 
kernel_RESOURCES        := 
yloop_RESOURCES        := 
vfs_RESOURCES        := 
wrapper_RESOURCES        := 
hal_RESOURCES        := 
log_RESOURCES        := 
ysh_RESOURCES        := 
yunit_RESOURCES        := 
vcall_RESOURCES        := 
linuximpl_RESOURCES        := 
test_RESOURCES        := 
linuxapp_MAKEFILE         := ./example/linuxapp/linuxapp.mk
board_linuxhost_MAKEFILE         := ./board/linuxhost/linuxhost.mk
rhino_MAKEFILE         := ./kernel/rhino/rhino.mk
kernel_MAKEFILE         := ././kernel/kernel.mk
yloop_MAKEFILE         := ./framework/yloop/yloop.mk
vfs_MAKEFILE         := ./framework/vfs/vfs.mk
wrapper_MAKEFILE         := ./framework/wrapper/wrapper.mk
hal_MAKEFILE         := ./framework/hal/hal.mk
log_MAKEFILE         := ./utility/log/log.mk
ysh_MAKEFILE         := ./tools/ysh/ysh.mk
yunit_MAKEFILE         := ./test/yunit/yunit.mk
vcall_MAKEFILE         := ./kernel/vcall/vcall.mk
linuximpl_MAKEFILE         := ././platform/mcu/linux/linux.mk
test_MAKEFILE         := ././kernel/rhino/test/test.mk
linuxapp_PRE_BUILD_TARGETS:= 
board_linuxhost_PRE_BUILD_TARGETS:= 
rhino_PRE_BUILD_TARGETS:= 
kernel_PRE_BUILD_TARGETS:= 
yloop_PRE_BUILD_TARGETS:= 
vfs_PRE_BUILD_TARGETS:= 
wrapper_PRE_BUILD_TARGETS:= 
hal_PRE_BUILD_TARGETS:= 
log_PRE_BUILD_TARGETS:= 
ysh_PRE_BUILD_TARGETS:= 
yunit_PRE_BUILD_TARGETS:= 
vcall_PRE_BUILD_TARGETS:= 
linuximpl_PRE_BUILD_TARGETS:= 
test_PRE_BUILD_TARGETS:= 
linuxapp_PREBUILT_LIBRARY := 
board_linuxhost_PREBUILT_LIBRARY := 
rhino_PREBUILT_LIBRARY := 
kernel_PREBUILT_LIBRARY := 
yloop_PREBUILT_LIBRARY := 
vfs_PREBUILT_LIBRARY := 
wrapper_PREBUILT_LIBRARY := 
hal_PREBUILT_LIBRARY := 
log_PREBUILT_LIBRARY := 
ysh_PREBUILT_LIBRARY := 
yunit_PREBUILT_LIBRARY := 
vcall_PREBUILT_LIBRARY := 
linuximpl_PREBUILT_LIBRARY := 
test_PREBUILT_LIBRARY := 
MiCO_SDK_UNIT_TEST_SOURCES   		:=                            
ALL_RESOURCES             		:= 
INTERNAL_MEMORY_RESOURCES 		:= 
EXTRA_TARGET_MAKEFILES 			:= 
APPS_START_SECTOR 				:=  
BOOTLOADER_FIRMWARE				:=  
ATE_FIRMWARE				        :=  
APPLICATION_FIRMWARE				:=  
PARAMETER_1_IMAGE					:=  
PARAMETER_2_IMAGE					:=  
FILESYSTEM_IMAGE					:=  
WIFI_FIRMWARE						:=  
BT_PATCH_FIRMWARE					:=  
MiCO_ROM_SYMBOL_LIST_FILE 		:= 
MiCO_SDK_CHIP_SPECIFIC_SCRIPT		:=              
MiCO_SDK_CONVERTER_OUTPUT_FILE	:=              
MiCO_SDK_FINAL_OUTPUT_FILE 		:=              
MiCO_RAM_STUB_LIST_FILE 			:= 
MOC_KERNEL_BIN_FILE 				:= 
MOC_APP_OFFSET 				:= 

##
 # Copyright (C) 2016 YunOS Project. All rights reserved.
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #   http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
##

CC := gcc -g -c -m32 -Wall -Werror -Wno-error=format \
      -DCONFIG_YOC_UT=1 -DHAVE_RHINO_KERNEL -DCONFIG_YSH_NO_TEST
LD := gcc -m32

SRCS := \
    core/k_buf_queue.c \
    core/k_dyn_mem_proc.c \
    core/k_err.c \
    core/k_event.c \
    core/k_idle.c \
    core/k_mm_blk.c \
    core/k_mm_region.c \
    core/k_mm_firstfit.c \
    core/k_mm_bestfit.c \
    core/k_mutex.c \
    core/k_obj.c \
    core/k_pend.c \
    core/k_queue.c \
    core/k_sched.c \
    core/k_sem.c \
    core/k_stats.c \
    core/k_sys.c \
    core/k_task.c \
    core/k_task_sem.c \
    core/k_tick.c \
    core/k_time.c \
    core/k_timer.c \
    core/k_workqueue.c

SRCS += \
    arch/linux/cpu_impl.c \

ASM_SRCS := \
    arch/linux/cpu_longjmp_32.S

SRCS += \
    targets/host/linux/main.c  \
    targets/host/linux/yoc_impl.c \
    targets/host/linux/hook_impl.c \
    targets/host/linux/ysh_impl.c\
    targets/host/linux/trace_impl.c

SRCS +=            \
    tools/ysh/ysh.c \
    tools/ysh/cmd/ysh_dumpsys.c \
    tools/ysh/cmd/ysh_bt.c \
    tools/mm_leak/yunos_mm_leak.c

SRCS += \
    test/test_fw.c \
    test/test_self_entry.c

SRCS += \
    test/core/buf_queue/buf_queue_del.c \
    test/core/buf_queue/buf_queue_dyn_create.c \
    test/core/buf_queue/buf_queue_flush.c \
    test/core/buf_queue/buf_queue_info_get.c \
    test/core/buf_queue/buf_queue_recv.c \
    test/core/buf_queue/buf_queue_test.c \
    test/core/event/event_break.c \
    test/core/event/event_opr.c \
    test/core/event/event_param.c \
    test/core/event/event_reinit.c \
    test/core/event/event_test.c \
    test/core/mm/mm_break.c \
    test/core/mm/mm_opr.c \
    test/core/mm/mm_param.c \
    test/core/mm/mm_reinit.c \
    test/core/mm/mm_test.c \
    test/core/mm_blk/mm_blk_break.c \
    test/core/mm_blk/mm_blk_fragment.c \
    test/core/mm_blk/mm_blk_opr.c \
    test/core/mm_blk/mm_blk_param.c \
    test/core/mm_blk/mm_blk_reinit.c \
    test/core/mm_blk/mm_blk_test.c \
    test/core/mutex/mutex_opr.c \
    test/core/mutex/mutex_param.c \
    test/core/mutex/mutex_reinit.c \
    test/core/mutex/mutex_test.c \
    test/core/queue/queue_all_send.c \
    test/core/queue/queue_back_send.c \
    test/core/queue/queue_del.c \
    test/core/queue/queue_flush.c \
    test/core/queue/queue_front_send.c \
    test/core/queue/queue_info_get.c \
    test/core/queue/queue_is_full.c \
    test/core/queue/queue_notify_set.c \
    test/core/queue/queue_nowait_recv.c \
    test/core/queue/queue_test.c \
    test/core/sem/sem_break.c \
    test/core/sem/sem_count.c \
    test/core/sem/sem_opr.c \
    test/core/sem/sem_param.c \
    test/core/sem/sem_reinit.c \
    test/core/sem/sem_test.c \
    test/core/sys/sys_opr.c \
    test/core/sys/sys_test.c \
    test/core/task/task_del.c \
    test/core/task/task_sleep.c \
    test/core/task/task_suspend_test.c \
    test/core/task/task_test.c \
    test/core/task/task_yield_test.c \
    test/core/task/task_misc_test.c \
    test/core/task_sem/tasksem_count.c \
    test/core/task_sem/tasksem_opr.c \
    test/core/task_sem/tasksem_param.c \
    test/core/task_sem/tasksem_test.c \
    test/core/time/time_opr.c \
    test/core/time/time_test.c \
    test/core/timer/timer_change.c \
    test/core/timer/timer_create_del.c \
    test/core/timer/timer_dyn_create_del.c \
    test/core/timer/timer_start_stop.c \
    test/core/timer/timer_test.c \
    test/core/workqueue/workqueue_test.c \
    test/core/workqueue/workqueue_interface.c

SRCS += \
    test/tools/ysh_unit_test.c

SRCS += \
    test/core/combination/comb_test.c \
    test/core/combination/sem_event.c \
    test/core/combination/sem_queue.c \
    test/core/combination/sem_queue_buf.c \
    test/core/combination/sem_mutex.c \
    test/core/combination/comb_all.c  \
    test/core/combination/mutex_queue.c \
    test/core/combination/mutex_queue_buf.c

SRCS += \
    test/core/mm_region/mm_region_test.c \
    test/core/mm_region/mm_region_break.c

INCS := \
    core/include \
    arch/linux

INCS += \
    test

INCS += \
    tools/ysh/include \
    tools/mm_leak/include

OBJS := $(SRCS:%.c=out/%.o)

ASM_OBJS := $(ASM_SRCS:%.S=out/%.o)

LIBS := pthread rt

TARGET := out/id2kernel

$(OBJS): out/%.o : %.c
	@mkdir -p $(dir $@)
	@ $(CC) -g $(INCS:%=-I%) -o $@ $<
	@echo CC: $@

$(ASM_OBJS): out/%.o : %.S
	@mkdir -p $(dir $@)
	@ $(CC) -g $(INCS:%=-I%) -o $@ $<
	@echo CC: $@

$(TARGET): $(OBJS) $(ASM_OBJS)
	@$(LD) -o $@ $^ $(LIBS:%=-l%)
	@echo LD: $@

all : $(TARGET)

clean:
	@rm -rf out


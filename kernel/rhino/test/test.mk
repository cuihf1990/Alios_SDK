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

NAME := test

GLOBAL_INCLUDES += ./

$(NAME)_SOURCES := \
    test_fw.c \
    test_self_entry.c \
    core/buf_queue/buf_queue_del.c \
    core/buf_queue/buf_queue_dyn_create.c \
    core/buf_queue/buf_queue_flush.c \
    core/buf_queue/buf_queue_info_get.c \
    core/buf_queue/buf_queue_recv.c \
    core/buf_queue/buf_queue_test.c \
    core/event/event_break.c \
    core/event/event_opr.c \
    core/event/event_param.c \
    core/event/event_reinit.c \
    core/event/event_test.c \
    core/mm_blk/mm_blk_break.c \
    core/mm_blk/mm_blk_fragment.c \
    core/mm_blk/mm_blk_opr.c \
    core/mm_blk/mm_blk_param.c \
    core/mm_blk/mm_blk_reinit.c \
    core/mm_blk/mm_blk_test.c \
    core/mutex/mutex_opr.c \
    core/mutex/mutex_param.c \
    core/mutex/mutex_reinit.c \
    core/mutex/mutex_test.c \
    core/queue/queue_all_send.c \
    core/queue/queue_back_send.c \
    core/queue/queue_del.c \
    core/queue/queue_flush.c \
    core/queue/queue_front_send.c \
    core/queue/queue_info_get.c \
    core/queue/queue_is_full.c \
    core/queue/queue_nowait_recv.c \
    core/queue/queue_notify_set.c \
    core/queue/queue_test.c \
    core/sem/sem_break.c \
    core/sem/sem_count.c \
    core/sem/sem_param.c \
    core/sem/sem_reinit.c \
    core/sem/sem_test.c \
    core/sem/sem_opr.c \
    core/sys/sys_opr.c \
    core/sys/sys_test.c \
    core/task/task_sleep.c \
    core/task/task_del.c  \
    core/task/task_suspend_test.c \
    core/task/task_test.c \
    core/task/task_yield_test.c \
    core/task/task_misc_test.c \
    core/task_sem/tasksem_count.c \
    core/task_sem/tasksem_opr.c \
    core/task_sem/tasksem_param.c \
    core/task_sem/tasksem_test.c \
    core/time/time_opr.c \
    core/time/time_test.c \
    core/timer/timer_change.c \
    core/timer/timer_create_del.c \
    core/timer/timer_dyn_create_del.c \
    core/timer/timer_start_stop.c \
    core/timer/timer_test.c \
    core/workqueue/workqueue_test.c \
    core/workqueue/workqueue_interface.c \
    core/mm_region/mm_region_break.c \
    core/mm_region/mm_region_test.c \
    core/ringbuf/ringbuf_break.c \
    core/ringbuf/ringbuf_test.c \
    core/combination/comb_test.c \
    core/combination/sem_event.c \
    core/combination/sem_queue_buf.c \
    core/combination/sem_mutex.c \
    core/combination/comb_all.c  \
    core/combination/mutex_queue_buf.c \
    core/combination/sem_queue.c\
    core/combination/mutex_queue.c \
    core/trace/trace_test.c


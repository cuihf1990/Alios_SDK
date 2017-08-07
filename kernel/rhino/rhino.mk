#
#  UNPUBLISHED PROPRIETARY SOURCE CODE
#  Copyright (c) 2016 MXCHIP Inc.
#
#  The contents of this file may not be disclosed to third parties, copied or
#  duplicated in any form, in whole or in part, without the prior written
#  permission of MXCHIP Corporation.
#

NAME := rhino

VERSION_MAJOR    = $(word 1, $(subst ., ,$(VERSION)))
VERSION_MINOR    = $(word 1, $(subst ., ,$(VERSION)))
VERSION_REVISION = $(word 1, $(subst ., ,$(VERSION)))

$(NAME)_COMPONENTS += rhino

# Define some macros to allow for some network-specific checks
GLOBAL_DEFINES += RTOS_$(NAME)=1
GLOBAL_DEFINES += $(NAME)_VERSION_MAJOR=$(VERSION_MAJOR)
GLOBAL_DEFINES += $(NAME)_VERSION_MINOR=$(VERSION_MINOR)
GLOBAL_DEFINES += $(NAME)_VERSION_REVISION=$(VERSION_REVISION)

GLOBAL_INCLUDES += core/include

$(NAME)_CFLAGS += -Wall -Werror
$(NAME)_SOURCES := core/k_err.c          \
                   core/k_mm.c           \
                   core/k_mm_region.c    \
                   core/k_mm_debug.c     \
                   core/k_obj_set.c      \
                   core/k_ringbuf.c      \
                   core/k_stats.c        \
                   core/k_task_sem.c     \
                   core/k_timer.c        \
                   core/k_buf_queue.c    \
                   core/k_event.c        \
                   core/k_mm_blk.c       \
                   core/k_mutex.c        \
                   core/k_pend.c         \
                   core/k_sched.c        \
                   core/k_sys.c          \
                   core/k_tick.c         \
                   core/k_workqueue.c    \
                   core/k_dyn_mem_proc.c \
                   core/k_idle.c         \
                   core/k_obj.c          \
                   core/k_queue.c        \
                   core/k_sem.c          \
                   core/k_task.c         \
                   core/k_time.c


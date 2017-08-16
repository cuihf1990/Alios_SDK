#
#  UNPUBLISHED PROPRIETARY SOURCE CODE
#  Copyright (c) 2016 MXCHIP Inc.
#
#  The contents of this file may not be disclosed to third parties, copied or
#  duplicated in any form, in whole or in part, without the prior written
#  permission of MXCHIP Corporation.
#

NAME := entry

GLOBAL_INCLUDES += .

$(NAME)_CFLAGS += -marm

$(NAME)_INCLUDES := ../../app      \
				   ../../app/config \
                   ../../func/include \
                   ../../os/include \
                   ../../ip/lmac/src/rwnx \
                   ../../ip/ke \
                   ../../ip/mac \
                   ../../../../yos 
					
                   
$(NAME)_SOURCES	 += arch_main.c \
                    boot_handlers.S \
                    boot_vectors.S \
                    ll.S \
                    ../intc/intc.c 
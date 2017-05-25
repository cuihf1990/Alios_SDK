# for linux host

LOCAL_PATH := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))

CC ?= gcc
AR ?= ar
RM := rm

SRCS := \
    $(wildcard core/*.c) \
    $(wildcard core/id2/src/*.c) \
    $(wildcard core/aes/src/*.c) \
    $(wildcard common/src/*.c) \
    $(wildcard network/src/*.c)

INCLUDES := \
           -I./core/id2/include/ \
           -I./core/aes/include/ \
           -I./common/include/ \
           -I./network/include \
           -I./../platform

CFLAGS := -g -Wall -O0
ifeq ($(MCU), ck802)
CFLAGS += -g -Wpointer-arith -Wundef -Wall -Wl,-EL -fno-inline-functions -nostdlib -ffunction-sections -fdata-sections -fno-builtin -mcpu=ck802 -mistack -fno-strict-aliasing -fno-strength-reduce
endif

ifeq ($(PLATFORM), linux)
    SRCS += $(wildcard ../platform/${PLATFORM}/*.c)
    CFLAGS += -DTFS_EMULATE
    SRCS += $(wildcard emulator/src/*.c)
    INCLUDES += -I./emulator/include/
    CFLAGS += -DTFS_OPENSSL
#    INCLUDES += -I../external/lib/openssl/include
else
    SRCS += ./hal/src/cmd.c
    SRCS += ./hal/src/hal_id2.c
    SRCS += ./hal/src/hal_aes.c
    ifeq ($(CRYPTO), rsa)
        SRCS += ./hal/src/hal_rsa.c
    else
        ifeq ($(CRYPTO), 3des)
            SRCS += ./hal/src/hal_3des.c
        endif
    endif
    INCLUDES += -I./hal/include
endif

ifeq ($(DEST), lib-debug)
    SRCS += ../demo/tfs_test.c
    INCLUDES += -I../include
endif

#$(warning $(SRCS))

OBJS := $(patsubst %.c,%.o,$(SRCS))

#$(warning $(OBJS))

CFLAGS += -DTFS_DEBUG
ifeq ($(HW), tee)
    CFLAGS += -DTFS_TEE
endif

ifeq ($(CRYPTO), 3des)
    CFLAGS += -DTFS_ID2_3DES
else
    ifeq ($(CRYPTO), rsa)
        CFLAGS += -DTFS_ID2_RSA
    endif
endif

CFLAGS += -DTFS_ONLINE

#CFLAGS += -DTFS_MBEDTLS
#LIBS += -lmbedtls -lmbedcrypto
#INCLUDES += -I/usr/local/include/

../libtfs.a : $(OBJS)
	$(AR) -rcs $@ $(OBJS)

%.o : %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $(LIBS) $< -o $*.o

clean:
	$(RM) -f $(OBJS)
	$(RM) -f ../libtfs.a

.PHONY:clean

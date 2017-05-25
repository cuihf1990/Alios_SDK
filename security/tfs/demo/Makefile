OUT_DIR := ../out

SRCS := \
    ./tfs_main.c \
    ./tfs_test.c

OBJS := $(patsubst %.c,%.o,$(SRCS))

CC := gcc
RM := rm
BIN := $(OUT_DIR)/tfs_demo

INCLUDES := -I../include -I../external/lib/openssl/include/ -I../platform -I../src/common/include


CFLAGS := -g -Wall -O0

LIBS += -L.. -ltfs

$(shell if [ ! -d $(OUT_DIR) ]; then mkdir $(OUT_DIR); fi;)

LIBS += -L../external/lib/openssl -lssl -lcrypto

ifeq ($(CRYPTO), 3des)
    CFLAGS += -DTFS_ID2_3DES
else
    ifeq ($(CRYPTO), rsa)
        CFLAGS += -DTFS_ID2_RSA
    endif
endif

$(BIN) : $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(INCLUDES) $(LIBS)

%.o : %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $*.o

clean:
	$(RM) -f $(OBJS)
	$(RM) -f $(BIN)
	$(shell if [ -z "`ls $(OUT_DIR)`" ]; then rmdir $(OUT_DIR); fi;)

.PHONY: clean

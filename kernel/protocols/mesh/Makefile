SDK := 1
GLOBAL_DEFINES += MESH_SDK
GLOBAL_INCLUDES += src/platform
GLOBAL_INCLUDES += ../../../security/alicrypto/inc

LWIP := 0
include mesh.mk

CFLAGS += -m32
CFLAGS += $(addprefix -D,$(GLOBAL_DEFINES) $($(NAME)_DEFINES))
CFLAGS += $(addprefix -I,$(GLOBAL_INCLUDES) $($(NAME)_INCLUDES))
CFLAGS += $($(NAME)_CFLAGS)

srcs := $($(NAME)_SOURCES)
objs := $(srcs:.c=.o)
objs := $(addprefix build/,$(objs))

all	: build/libumesh.a

build/libumesh.a : $(objs)
	@$(AR) rcs $@ $^

build/%.o : %.c
	@-mkdir -p $(dir $@)
	@echo CC $^
	@$(CC) -o $@ -c $^ $(CFLAGS) $($@_CFLAGS)

clean:
	rm -rf build


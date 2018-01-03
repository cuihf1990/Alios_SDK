NAME := bluetooth

$(NAME)_TYPE := kernel
GLOBAL_INCLUDES += include \
                   port/include

$(NAME)_INCLUDES += include \
                    port/include \
                    core/include \
                    core/tinycrypt/include \
                    include/drivers \
                    profile

$(NAME)_COMPONENTS += yloop

$(NAME)_SOURCES := core/atomic_c.c \
                   core/buf.c \
                   host/uuid.c \
                   host/hci_core.c \
                   host/log.c \
                   core/tinycrypt/source/utils.c \
                   core/tinycrypt/source/sha256.c \
                   core/tinycrypt/source/hmac.c \
                   core/tinycrypt/source/hmac_prng.c \
                   host/conn.c \
                   host/l2cap.c \
                   host/att.c \
                   host/gatt.c \
                   host/smp_null.c \
                   profile/dis.c \
                   profile/gap.c \
                   profile/bas.c \
                   profile/cts.c \
                   profile/hrs.c \
                   profile/hog.c \

$(NAME)_SOURCES := core/work.c \
                   port/rhino_port.c
$(NAME)_INCLUDES += ../../rhino/core/include

ifeq ($(COMPILER),)
$(NAME)_CFLAGS      += -Wall
else ifeq ($(COMPILER),gcc)
$(NAME)_CFLAGS      += -Wall
endif

GLOBAL_DEFINES += CONFIG_AOS_BLUETOOTH
GLOBAL_DEFINES += CONFIG_BLUETOOTH
GLOBAL_DEFINES += CONFIG_AOS_RHINO
GLOBAL_DEFINES += CONFIG_BLUETOOTH_PERIPHERAL

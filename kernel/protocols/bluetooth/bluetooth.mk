NAME := bluetooth

$(NAME)_TYPE := kernel
GLOBAL_INCLUDES += include \
                   include/drivers \
                   port/include

$(NAME)_INCLUDES += core/tinycrypt/include \
                    core/include \
                    profile \
                    ../../rhino/core/include

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
                   host/smp.c \
                   host/keys.c \
                   core/tinycrypt/source/cmac_mode.c \
                   core/tinycrypt/source/aes_encrypt.c \
                   core/work.c \
                   port/rhino_port.c

ifeq ($(COMPILER),)
$(NAME)_CFLAGS      += -Wall
else ifeq ($(COMPILER),gcc)
$(NAME)_CFLAGS      += -Wall
endif

GLOBAL_DEFINES += CONFIG_AOS_BLUETOOTH
GLOBAL_DEFINES += CONFIG_BLUETOOTH
GLOBAL_DEFINES += CONFIG_AOS_RHINO
GLOBAL_DEFINES += CONFIG_BLUETOOTH_PERIPHERAL

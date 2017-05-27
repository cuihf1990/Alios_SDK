
NAME := board_armhflinux

MODULE              := 1062
HOST_ARCH           := armhflinux
HOST_MCU_FAMILY     := linux

$(NAME)_COMPONENTS  := yloop vfs hal log vcall ysh alicrypto modules.kv netmgr

ifeq ($(gcov),1)
GLOBAL_CFLAGS  += -fprofile-arcs -ftest-coverage
GLOBAL_LDFLAGS += --coverage
endif

GLOBAL_CFLAGS += -I$(SOURCE_ROOT)/board/armhflinux/include
GLOBAL_LDFLAGS += -L$(SOURCE_ROOT)/board/armhflinux/lib

CONFIG_LIB_TFS := y
CONFIG_TFS_ID2_RSA := y
CONFIG_TFS_ID2_3DES := n
CONFIG_TFS_EMULATE := n
CONFIG_TFS_ON_LINE := n
CONFIG_TFS_OPENSSL := n
CONFIG_TFS_MBEDTLS := n
CONFIG_TFS_DEBUG := n
CONFIG_TFS_VENDOR_FACTORY := csky
CONFIG_TFS_VENDOR_VERSION := tee
CONFIG_TFS_SE_LIB := libtfshal.a
CONFIG_TFS_TEE := n
CONFIG_TFS_SW := y
CONFIG_TFS_TEST := n

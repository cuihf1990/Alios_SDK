
NAME := board_linuxhost

MODULE              := 1062
HOST_ARCH           := linux
HOST_MCU_FAMILY     := linux

$(NAME)_COMPONENTS  := yloop vfs hal log vcall ysh alicrypto modules.kv tfs netmgr

CONFIG_LIB_TFS := y
CONFIG_TFS_ID2_RSA := y
CONFIG_TFS_ID2_3DES := n
CONFIG_TFS_EMULATE := y
CONFIG_TFS_ON_LINE := y
CONFIG_TFS_OPENSSL := y
CONFIG_TFS_MBEDTLS := n
CONFIG_TFS_DEBUG := n
CONFIG_TFS_VENDOR_FACTORY := csky
CONFIG_TFS_VENDOR_VERSION := tee
CONFIG_TFS_SE_LIB := libtfshal.a
CONFIG_TFS_TEE := n
CONFIG_TFS_TEST := y

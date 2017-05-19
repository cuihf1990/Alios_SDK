# for linux host

NAME := tfs

$(NAME)_SOURCES := src/core/aes/src/aes128.c \
				   src/core/id2/src/device_info.c \
				   src/core/id2/src/id2.c \
				   src/core/tfs_aes.c \
				   src/core/tfs_id2.c \
				   src/network/src/http.c

$(NAME)_INCLUDES := src/core/id2/include/ \
					src/core/aes/include/ \
					src/common/include/ \
					src/network/include/ \
					platform

GLOBAL_INCLUDES += include/

ifeq ($(CONFIG_TFS_TEST), y)
    GLOBAL_INCLUDES += demo/
    $(NAME)_SOURCES += demo/tfs_test.c
endif

ifeq ($(CONFIG_TFS_EMULATE), y)
    $(NAME)_DEFINES += TFS_EMULATE
	$(NAME)_SOURCES += \
				   src/emulator/src/emu_id2.c \
				   src/emulator/src/emu_aes.c

    $(NAME)_INCLUDES += src/emulator/include/
else
    $(NAME)_SOURCES += src/hal/src/hal_id2.c \
					   src/hal/src/hal_aes.c

    $(NAME)_INCLUDES += src/hal/include/
endif

ifeq ($(CONFIG_TFS_ON_LINE), y)
	$(NAME)_DEFINES += TFS_ONLINE
endif

#$(NAME)_CFLAGS += -g -Wall -O0

ifeq ($(CONFIG_TFS_OPENSSL), y)
	$(NAME)_DEFINES += TFS_OPENSSL
    GLOBAL_LDFLAGS += -lcrypto
endif

ifeq ($(CONFIG_TFS_MBEDTLS), y)
	$(NAME)_DEFINES += TFS_MBEDTLS
    GLOBAL_LDFLAGS += -lmbedcrypto
endif

ifeq ($(CONFIG_TFS_ID2_3DES), y)
    $(NAME)_DEFINES += TFS_ID2_3DES
    ifeq ($(CONFIG_TFS_EMULATE), y)
        $(NAME)_SOURCES += src/emulator/src/emu_3des.c
	else
	    $(NAME)_SOURCES += src/hal/src/hal_3des.c
	endif
endif

ifeq ($(CONFIG_TFS_ID2_RSA), y)
    $(NAME)_DEFINES += TFS_ID2_RSA
    ifeq ($(CONFIG_TFS_EMULATE), y)
        $(NAME)_SOURCES += src/emulator/src/emu_rsa.c
	else
	    $(NAME)_SOURCES += src/hal/src/hal_rsa.c
	endif
endif

ifeq ($(CONFIG_TFS_DEBUG), y)
    $(NAME)_DEFINES += TFS_DEBUG
endif

ifeq ($(CONFIG_TFS_TEE), y)
    $(NAME)_DEFINES += TFS_TEE
endif

ifeq ($(HOST_ARCH), linux)
    $(NAME)_SOURCES += \
				   platform/linux/base64.c \
				   platform/linux/cJSON.c \
				   platform/linux/md5.c \
				   platform/linux/pal_platform_base64.c \
				   platform/linux/pal_platform_json.c \
				   platform/linux/pal_platform_md5.c \
				   platform/linux/pal_platform_memory.c \
				   platform/linux/pal_platform_network.c \
				   platform/linux/pal_platform_product.c \
				   platform/linux/pal_platform_random.c \
				   platform/linux/pal_platform_storage.c \
				   platform/linux/pal_platform_time.c
else
    $(NAME)_SOURCES += \
				   platform/aos/base64.c \
				   platform/aos/cJSON.c \
				   platform/aos/md5.c \
				   platform/aos/pal_platform_base64.c \
				   platform/aos/pal_platform_json.c \
				   platform/aos/pal_platform_md5.c \
				   platform/aos/pal_platform_memory.c \
				   platform/aos/pal_platform_network.c \
				   platform/aos/pal_platform_product.c \
				   platform/aos/pal_platform_random.c \
				   platform/aos/pal_platform_storage.c \
				   platform/aos/pal_platform_time.c
endif

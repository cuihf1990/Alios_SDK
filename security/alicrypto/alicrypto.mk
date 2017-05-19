
NAME := alicrypto
ALICRYPTO_TEST := no

$(NAME)_SOURCES :=
$(NAME)_COMPONENTS :=

GLOBAL_INCLUDES     += inc
GLOBAL_LDFLAGS      +=
GLOBAL_DEFINES      += CONFIG_ALICRYPTO
GLOBAL_CFLAGS       +=

ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)

$(NAME)_PREBUILT_LIBRARY := lib/linuxhost/libmbedcrypto.a  \
		lib/linuxhost/libalicrypto.a

ifeq ($(ALICRYPTO_TEST), yes)
GLOBAL_INCLUDES     += test
GLOBAL_LDFLAGS      +=
$(NAME)_SOURCES += \
				test/ali_crypto_test.c \
				test/ali_crypto_test_comm.c \
				test/ali_crypto_test_aes.c \
				test/ali_crypto_test_hash.c \
				test/ali_crypto_test_rand.c \
				test/ali_crypto_test_rsa.c \
				
endif

else

endif # !linuxapp@linuxhost

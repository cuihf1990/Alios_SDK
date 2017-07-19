
NAME := alicrypto
ALICRYPTO_TEST := yes

$(NAME)_SOURCES :=
$(NAME)_COMPONENTS :=

GLOBAL_INCLUDES     += inc
GLOBAL_LDFLAGS      +=
GLOBAL_DEFINES      += CONFIG_ALICRYPTO
GLOBAL_CFLAGS       +=

#$(NAME)_SOURCES += test/ali_crypto_test_weak.c
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
				test/ali_crypto_test_hmac.c \
				
endif

else ifeq ($(findstring armhflinux, $(BUILD_STRING)), armhflinux)
$(NAME)_PREBUILT_LIBRARY := lib/armhflinux/libmbedcrypto.a  \
		lib/armhflinux/libalicrypto.a

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
				test/ali_crypto_test_hmac.c \
				
endif # end ALICRYPTO_TEST=yes

else ifeq ($(findstring mk108, $(BUILD_STRING)), mk108)
$(NAME)_PREBUILT_LIBRARY := lib/mk108/libmbedcrypto.a  \
		lib/mk108/libalicrypto.a

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
				test/ali_crypto_test_hmac.c \
				
endif # end ALICRYPTO_TEST=yes

else ifeq ($(findstring mk3060, $(BUILD_STRING)), mk3060)
$(NAME)_PREBUILT_LIBRARY := lib/mk108/libmbedcrypto.a  \
		lib/mk108/libalicrypto.a

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
				test/ali_crypto_test_hmac.c \
				
endif # end ALICRYPTO_TEST=yes

endif # !linuxapp@linuxhost

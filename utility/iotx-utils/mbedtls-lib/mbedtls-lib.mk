NAME := libmbedtls-2.5.0

$(NAME)_INCLUDES :=  \
    ./include \
    ../sdk-impl \
    ../sdk-impl/imports
    
ifeq ($(CONFIG_COAP_DTLS_SUPPORT), y)
$(NAME)_DEFINES += COAP_DTLS_SUPPORT
endif

$(NAME)_DEFINES += FORCE_SSL_VERIFY

$(NAME)_SOURCES := \
    HAL_DTLS_mbedtls.c \
    HAL_TLS_mbedtls.c

#ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
#$(NAME)_PREBUILT_LIBRARY := lib/linuxhost/mbedtls-2.5.0.a
#endif


SOURCE_PATH := library
$(NAME)_SOURCES += \
    $(SOURCE_PATH)/aes.c \
    $(SOURCE_PATH)/aesni.c \
    $(SOURCE_PATH)/arc4.c \
    $(SOURCE_PATH)/asn1parse.c \
    $(SOURCE_PATH)/asn1write.c \
    $(SOURCE_PATH)/base64.c \
    $(SOURCE_PATH)/bignum.c \
    $(SOURCE_PATH)/blowfish.c \
    $(SOURCE_PATH)/camellia.c \
    $(SOURCE_PATH)/ccm.c \
    $(SOURCE_PATH)/certs.c \
    $(SOURCE_PATH)/cipher.c \
    $(SOURCE_PATH)/cipher_wrap.c \
    $(SOURCE_PATH)/cmac.c \
    $(SOURCE_PATH)/ctr_drbg.c \
    $(SOURCE_PATH)/debug.c \
    $(SOURCE_PATH)/des.c \
    $(SOURCE_PATH)/dhm.c \
    $(SOURCE_PATH)/ecdh.c \
    $(SOURCE_PATH)/ecdsa.c \
    $(SOURCE_PATH)/ecjpake.c \
    $(SOURCE_PATH)/ecp.c \
    $(SOURCE_PATH)/ecp_curves.c \
    $(SOURCE_PATH)/entropy.c \
    $(SOURCE_PATH)/entropy_poll.c \
    $(SOURCE_PATH)/error.c \
    $(SOURCE_PATH)/gcm.c \
    $(SOURCE_PATH)/havege.c \
    $(SOURCE_PATH)/hmac_drbg.c \
    $(SOURCE_PATH)/md2.c \
    $(SOURCE_PATH)/md4.c \
    $(SOURCE_PATH)/md5.c \
    $(SOURCE_PATH)/md.c \
    $(SOURCE_PATH)/md_wrap.c \
    $(SOURCE_PATH)/memory_buffer_alloc.c \
    $(SOURCE_PATH)/net_sockets.c \
    $(SOURCE_PATH)/oid.c \
    $(SOURCE_PATH)/padlock.c \
    $(SOURCE_PATH)/pem.c \
    $(SOURCE_PATH)/pk.c \
    $(SOURCE_PATH)/pkcs11.c \
    $(SOURCE_PATH)/pkcs12.c \
    $(SOURCE_PATH)/pkcs5.c \
    $(SOURCE_PATH)/pkparse.c \
    $(SOURCE_PATH)/pk_wrap.c \
    $(SOURCE_PATH)/pkwrite.c \
    $(SOURCE_PATH)/platform.c \
    $(SOURCE_PATH)/ripemd160.c \
    $(SOURCE_PATH)/rsa.c \
    $(SOURCE_PATH)/sha1.c \
    $(SOURCE_PATH)/sha256.c \
    $(SOURCE_PATH)/sha512.c \
    $(SOURCE_PATH)/ssl_cache.c \
    $(SOURCE_PATH)/ssl_ciphersuites.c \
    $(SOURCE_PATH)/ssl_cli.c \
    $(SOURCE_PATH)/ssl_cookie.c \
    $(SOURCE_PATH)/ssl_srv.c \
    $(SOURCE_PATH)/ssl_ticket.c \
    $(SOURCE_PATH)/ssl_tls.c \
    $(SOURCE_PATH)/threading.c \
    $(SOURCE_PATH)/timing.c \
    $(SOURCE_PATH)/version.c \
    $(SOURCE_PATH)/version_features.c \
    $(SOURCE_PATH)/x509.c \
    $(SOURCE_PATH)/x509_create.c \
    $(SOURCE_PATH)/x509_crl.c \
    $(SOURCE_PATH)/x509_crt.c \
    $(SOURCE_PATH)/x509_csr.c \
    $(SOURCE_PATH)/x509write_crt.c \
    $(SOURCE_PATH)/x509write_csr.c \
    $(SOURCE_PATH)/xtea.c


$(NAME)_CFLAGS := $(filter-out -Werror,$(CFLAGS))

$(NAME)_DEFINES += DEBUG

/*
 * Copyright (C) 2017 YunOS IoT Project. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "mbedtls/config.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/net_sockets.h"

#if defined(MBEDTLS_THREADING_ALT)
#include "mbedtls/threading.h"
#endif

//#define CONFIG_SSL_DEBUG

#define SSL_DEBUG_LEVEL    1

typedef struct _ssl_param_t {
    mbedtls_net_context net;
    mbedtls_x509_crt ca_cert;
    mbedtls_ssl_config conf;
    mbedtls_ssl_context ssl;
} ssl_param_t;

static int ssl_random(void *prng, unsigned char *output, size_t output_len)
{

    size_t i;

    (void)prng;

    for (i = 0; i < output_len; i++) {
        output[i] = rand() & 0xff;
    }

    return 0;
}

static void ssl_debug(void *ctx, int level,
                      const char *file, int line, const char *str)
{
    (void)ctx;
    (void) level;

    platform_printf("%s, line: %d: %s", file, line, str);

    return;
}

void *platform_ssl_connect(_IN_ void *tcp_fd, _IN_ const char *server_cert, _IN_ int server_cert_len)
{
    int ret;
    unsigned int result;
    ssl_param_t *ssl_param = NULL;

    if (tcp_fd == NULL ||
        server_cert == NULL || server_cert_len <= 0) {
        platform_printf("ssl_connect: invalid input args!\n");
        return NULL;
    }

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold(SSL_DEBUG_LEVEL);
#endif

#if defined(MBEDTLS_THREADING_ALT)
    mbedtls_threading_set_alt(threading_mutex_init,
                              threading_mutex_free,
                              threading_mutex_lock,
                              threading_mutex_unlock);
#endif

    ssl_param = platform_malloc(sizeof(ssl_param_t));
    if (NULL == ssl_param) {
        platform_printf("ssl_connect: platform_malloc(%d) fail\n", sizeof(ssl_param_t));
        return NULL;
    } else {
        mbedtls_net_init(&ssl_param->net);
        mbedtls_x509_crt_init(&ssl_param->ca_cert);
        mbedtls_ssl_config_init(&ssl_param->conf);
        mbedtls_ssl_init(&ssl_param->ssl);
    }

    /* 
     * Initialize the connection
     */
    ssl_param->net.fd = (int)tcp_fd;

    /* 
     * Initialize certificates
     */
#if defined(CONFIG_SSL_DEBUG)
    platform_printf("...... Loading the CA root certificate ... ");   
#endif

    ret = mbedtls_x509_crt_parse(&ssl_param->ca_cert,
              (unsigned char *)server_cert, (size_t)server_cert_len + 1);
    if (ret < 0) {
        platform_printf("ssl_connect: x509 parse failed- 0x%x\n", -ret);
        goto _err;
    }

#if defined(CONFIG_SSL_DEBUG)
        platform_printf("ok (%d skipped)\n", ret);
#endif

    /* 
     * setup stuff
     */
#if defined(CONFIG_SSL_DEBUG)
    platform_printf("...... Setting up the SSL/TLS structure ... ");
#endif

    ret = mbedtls_ssl_config_defaults(
                    &ssl_param->conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) {
        platform_printf("ssl_connect: set ssl config failed - %d\n", ret);
        goto _err;
    }

#if defined(CONFIG_SSL_DEBUG)
    platform_printf("ok\n");
#endif

    mbedtls_ssl_conf_authmode(&ssl_param->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&ssl_param->conf, &ssl_param->ca_cert, NULL);
    mbedtls_ssl_conf_rng(&ssl_param->conf, ssl_random, NULL);
    mbedtls_ssl_conf_dbg(&ssl_param->conf, ssl_debug, NULL);

    ret = mbedtls_ssl_setup(&ssl_param->ssl, &ssl_param->conf);
    if (ret != 0) {
        platform_printf("ssl_connect: mbedtls_ssl_setup returned - %d\n", ret);
        goto _err;
    }

    mbedtls_ssl_set_bio(&ssl_param->ssl, &ssl_param->net, mbedtls_net_send, mbedtls_net_recv, NULL);

    /* 
     * handshake
     */
#if defined(CONFIG_SSL_DEBUG)
    platform_printf("...... Performing the SSL/TLS handshake ... ");
#endif

    while ((ret = mbedtls_ssl_handshake(&ssl_param->ssl)) != 0) {
        if ((ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE)) {
            platform_printf("ssl_connect: mbedtls_ssl_handshake returned -0x%04x", -ret);
            goto _err;
        }
    }

#if defined(CONFIG_SSL_DEBUG)
    platform_printf("ok\n");
#endif

    /* 
     * verify the server certificate
     */
#if defined(CONFIG_SSL_DEBUG)
    platform_printf("...... Verifying peer X.509 certificate ... ");
#endif

    result = mbedtls_ssl_get_verify_result(&ssl_param->ssl);
    if (result != 0) {
        platform_printf("ssl_connect: verify result not confirmed - %d\n", result);
        goto _err;
    }

#if defined(CONFIG_SSL_DEBUG)
    platform_printf("ok\n");
#endif

    return (void *)ssl_param;

_err:
    if (ssl_param != NULL) {
        mbedtls_net_free( &ssl_param->net);
        mbedtls_x509_crt_free(&ssl_param->ca_cert);
        mbedtls_ssl_free(&ssl_param->ssl);
        mbedtls_ssl_config_free(&ssl_param->conf);
        platform_free(ssl_param);
    }

    return NULL;
}

int platform_ssl_send(_IN_ void *ssl, _IN_ const char *buffer, _IN_ int length)
{
    int ret;
    ssl_param_t *ssl_param;

    if (ssl == NULL || buffer == NULL || length <= 0) {
        platform_printf("ssl_send: invalid input args\n");
        return -1;
    }

#if defined(CONFIG_SSL_DEBUG)
    platform_printf("...... > Send to server: ");
#endif

    ssl_param = (ssl_param_t *)ssl;
    ret = mbedtls_ssl_write(&ssl_param->ssl, buffer, length);
    if (ret < 0) {
        platform_printf("ssl_send: mbedtls_ssl_write failed - %d\n", ret);
        return -1;
    }

#if defined(CONFIG_SSL_DEBUG)
    platform_printf("%d bytes sent\n", ret);
#endif

    return ret;
}

int platform_ssl_recv(_IN_ void *ssl, _IN_ char *buffer, _IN_ int length)
{
    int ret;
    int total_len = 0;
    ssl_param_t *ssl_param;

    if (ssl == NULL || buffer == NULL || length <= 0) {
        platform_printf("ssl_recv: invalid input args\n");
        return -1;
    } else {
        ssl_param = (ssl_param_t *)ssl;
    }

#if defined(CONFIG_SSL_DEBUG)
    platform_printf("...... < Read from server: ");
#endif

    do {
        ret = mbedtls_ssl_read(&ssl_param->ssl, buffer, length - total_len);
        if (ret > 0) {
            total_len += ret;
            buffer += ret;
        } else if (ret == 0) {
            /* EOF */
            break;
        } else {
            if (ret == MBEDTLS_ERR_SSL_WANT_READ ||
                ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
                /* 
                 * indicate that read already complete,
                 * if call read again, it will return 0(EOF))
                 */
                break;
            }

            platform_printf("ssl_recv: mbedtls_ssl_read failed - 0x%x\n", -ret);

            return -1;
        }
    } while(1);

#if defined(CONFIG_SSL_DEBUG)
    platform_printf("%d bytes read\n", total_len);
#endif

    return total_len;
}

int platform_ssl_close(_IN_ void *ssl)
{
    ssl_param_t *ssl_param;

    if (ssl == NULL) {
        return 0;
    }

    ssl_param = (ssl_param_t *)ssl;
    mbedtls_ssl_close_notify(&ssl_param->ssl);

    mbedtls_net_free( &ssl_param->net);
    mbedtls_x509_crt_free(&ssl_param->ca_cert);
    mbedtls_ssl_free(&ssl_param->ssl);
    mbedtls_ssl_config_free(&ssl_param->conf);
    platform_free(ssl_param);

#if defined(MBEDTLS_THREADING_ALT)
    mbedtls_threading_free_alt();
#endif

    return 0;
}

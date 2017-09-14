/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef AOS_MBEDTLS_SSL_H
#define AOS_MBEDTLS_SSL_H

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include <stdlib.h>

void *mbedtls_ssl_connect(void *tcp_fd, const char *ca_cert, int ca_cert_len);
int mbedtls_ssl_send(void *ssl, const char *buffer, int length);
int mbedtls_ssl_recv(void *ssl, char *buffer, int length);
int mbedtls_ssl_close(void *ssl);

#endif /* AOS_MBEDTLS_SSL_H */


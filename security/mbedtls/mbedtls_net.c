/*
 * Copyright (C) 2017 YunOS IoT Project. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#if defined(MBEDTLS_NET_ALT_UART)

#include "stm32_wifi.h"
#include "mbedtls/net_sockets.h"

#define WIFI_WRITE_TIMEOUT   200
#define WIFI_READ_TIMEOUT    200
#define WIFI_PAYLOAD_SIZE    ES_WIFI_PAYLOAD_SIZE

void mbedtls_net_init(mbedtls_net_context *ctx)
{
    ctx->fd = -1;
}

int mbedtls_net_connect(mbedtls_net_context *ctx, const char *host, const char *port, int proto)
{
    WIFI_Status_t ret;
    WIFI_Protocol_t type;
    uint8_t ip_addr[4];

    ret = WIFI_GetHostAddress((char *)host, ip_addr);
    if (ret != WIFI_STATUS_OK) {
        printf("net_connect: get host addr fail - %d\n", ret);
        return MBEDTLS_ERR_NET_UNKNOWN_HOST;
    }

    type = proto == MBEDTLS_NET_PROTO_UDP ?
                    WIFI_UDP_PROTOCOL : WIFI_TCP_PROTOCOL;
    ret = WIFI_OpenClientConnection(0, type, "", ip_addr, atoi(port), 0);
    if (ret != WIFI_STATUS_OK) {
        printf("net_connect: open client fail - %d\n", ret);
        return MBEDTLS_ERR_NET_CONNECT_FAILED;
    }

    ctx->fd = 0;

    return ret;
}

int mbedtls_net_send(void *ctx, const unsigned char *buf, size_t len)
{
    WIFI_Status_t ret;
    uint16_t send_size;
    int fd = ((mbedtls_net_context *) ctx)->fd;

    if (fd < 0) {
        printf("net_send: invalid socket fd\n");
        return MBEDTLS_ERR_NET_INVALID_CONTEXT;
    }

    ret = WIFI_SendData((uint8_t)fd,
                        (uint8_t *)buf, len,
                        &send_size, WIFI_WRITE_TIMEOUT);
    if (ret != WIFI_STATUS_OK) {
        printf("net_send: send data fail - %d\n", ret);
        return MBEDTLS_ERR_NET_SEND_FAILED;
    }

    return send_size;
}

int mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len)
{
    WIFI_Status_t ret;
    uint16_t recv_size;
    int fd = ((mbedtls_net_context *) ctx)->fd;

    if (fd < 0) {
        printf("net_recv: invalid socket fd\n");
        return(MBEDTLS_ERR_NET_INVALID_CONTEXT);
    }

    if (len > WIFI_PAYLOAD_SIZE) {
        len = WIFI_PAYLOAD_SIZE;
    }

    ret = WIFI_ReceiveData((uint8_t)fd,
                            buf, (uint16_t)len,
                            &recv_size, WIFI_READ_TIMEOUT);
    if (ret != WIFI_STATUS_OK) {
        printf("net_recv: receive data fail - %d\n", ret);
        return MBEDTLS_ERR_NET_RECV_FAILED;
    }

    //TODO, how to identify the connection is shutdown?
    if (recv_size == 0) {
        return MBEDTLS_ERR_SSL_WANT_READ;
    }

    return recv_size;
}

int mbedtls_net_recv_timeout(void *ctx, unsigned char *buf, size_t len,
                      uint32_t timeout )
{
    WIFI_Status_t ret;
    uint16_t recv_size;
    int fd = ((mbedtls_net_context *) ctx)->fd;

    if (fd < 0) {
        printf("net_recv_timeout: invalid socket fd\n");
        return(MBEDTLS_ERR_NET_INVALID_CONTEXT);
    }

    if (len > WIFI_PAYLOAD_SIZE) {
        len = WIFI_PAYLOAD_SIZE;
    }

    // TODO: STM32 WiFi module can't set mqtt default timeout 60000, will return error, need to check with WiFi module, ignore param "timeout"
    ret = WIFI_ReceiveData((uint8_t)fd,
                            buf, (uint16_t)len,
                            &recv_size, WIFI_READ_TIMEOUT);
    if (ret != WIFI_STATUS_OK) {
        printf("net_recv_timeout: receive data fail - %d\n", ret);
        return MBEDTLS_ERR_NET_RECV_FAILED;
    }

    //TODO, how to identify the connection is shutdown?
    if (recv_size == 0) {
        return MBEDTLS_ERR_SSL_WANT_READ;
    }

    return recv_size;
}

void mbedtls_net_free(mbedtls_net_context *ctx)
{
    WIFI_Status_t ret;

    if (ctx->fd == -1)
        return;

    ret = WIFI_CloseClientConnection((uint32_t)ctx->fd);
    if (ret != WIFI_STATUS_OK) {
        printf("net_free: close client fail - %d\n", ret);
        return;
    }

    ctx->fd = -1;
}

#endif /* MBEDTLS_NET_ALT_UART */

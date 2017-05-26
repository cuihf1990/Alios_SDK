/*
 *  SSL client demonstration program
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yos/network.h>

#include "mbedtls/mbedtls_ssl.h"

const char *mbedtls_test_ca_pem = "-----BEGIN CERTIFICATE-----\n"   \
"MIIDtzCCAp+gAwIBAgIJAOxbLdldR1+SMA0GCSqGSIb3DQEBBQUAMHIxCzAJBgNV\n"\
"BAYTAkNOMREwDwYDVQQIDAh6aGVqaWFuZzERMA8GA1UEBwwIaGFuZ3pob3UxEDAO\n"\
"BgNVBAoMB2FsaWJhYmExDjAMBgNVBAsMBXl1bm9zMRswGQYDVQQDDBIqLnNtYXJ0\n"\
"LmFsaXl1bi5jb20wHhcNMTQwOTE3MDI0OTM0WhcNMjQwOTE0MDI0OTM0WjByMQsw\n"\
"CQYDVQQGEwJDTjERMA8GA1UECAwIemhlamlhbmcxETAPBgNVBAcMCGhhbmd6aG91\n"\
"MRAwDgYDVQQKDAdhbGliYWJhMQ4wDAYDVQQLDAV5dW5vczEbMBkGA1UEAwwSKi5z\n"\
"bWFydC5hbGl5dW4uY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA\n"\
"pwFEj4onz7YZ0ESpG7BNZmuK4KJgGMlFHnEL3AT1YtvB7DGePTNsW9hX3WYsaqy7\n"\
"+4PyzJgfNJY3WQr6uPv/EMwqlYMO0F8kg9AmFepuicHh5JvCeTJciG7OH/8qHhEB\n"\
"3b3w35un5YxUXuffw3SiFTj+vnFdc3Yj9pBv++0nsDvl6l8TSkJMnRLY8lRzzi1T\n"\
"rbdsDeNXLnfeThElMPFeI1h+s7amt2ktBGnv6HAg7a9OehUI8uTpFZ7559Yf8Dpm\n"\
"MDijYc6LLLSE6OO5C7im0pg8IRu6oZo1F5raK5gbRU/QI7K58IuIo+k4+clcvtko\n"\
"Ck4RkwdvC8cc0u5mJ8mXJwIDAQABo1AwTjAdBgNVHQ4EFgQUw6RWDo81JEoy+Vnf\n"\
"vMTvRsLkZ30wHwYDVR0jBBgwFoAUw6RWDo81JEoy+VnfvMTvRsLkZ30wDAYDVR0T\n"\
"BAUwAwEB/zANBgkqhkiG9w0BAQUFAAOCAQEAS2UQtfguKHnlxH9jpfJUyGKFoaTT\n"\
"8/XhPW3CW9c++zDrgNq920iQ42Yl9zB58iJ/v0w6n9quTtIta6QD72ssEJtWc2v2\n"\
"rwu14WJB5tGhBRagMvtF7p/56KYxib0p3fqjaE3Neer5r8Mgb6oI13tHbf0WT4JM\n"\
"GJCLsDoz4ZwvemLISeonZVSVIezs0BDU/TeEK2kIeUDB14FR6fY/U4ovS/v+han8\n"\
"NLhWorEpB1p2sgnSPgSVc6ZPHHyjIQOcWdn56vnOf41rLF/zqjD0Sn7YgceAd5OT\n"\
"rJ/t7PbiC/sn8SR7+0ATOMh0vRSA9HuuvoDz0adMhoFnba2FwiENfsLlhw==\n"    \
"-----END CERTIFICATE-----\n";

const char *server_name = "alink.tcp.aliyun.com";
const int server_port = 443;

unsigned char alink_req_data[] = {
    0x57, 0x53, 0x46, 0x31, 0x00, 0x00, 0x00, 0x11,
    0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03
};

void *network_socket_create(const char *net_addr, int port)
{
    int ret;
    int tcp_fd;
    struct hostent *host;
    struct sockaddr_in saddr;
    int opt_val = 1;

    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd < 0) {
        printf("creat socket fail\n");
        return NULL;
    }

    host = gethostbyname(net_addr);
    if (host == NULL) {
        printf("get host by name fail\n");
        goto _err;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = *(unsigned long *)(host->h_addr);

    setsockopt(tcp_fd, SOL_SOCKET, SO_RCVTIMEO, &opt_val, sizeof(opt_val));

    ret = connect(tcp_fd, (struct sockaddr*)&saddr, sizeof(saddr));
    if (ret < 0) {
        printf("socket connect fail\n");
        goto _err;
    }

    return (void *)tcp_fd;

_err:
    close(tcp_fd);

    return NULL;
}

void network_socket_destroy(void *tcp_fd)
{
    if (tcp_fd == NULL) {
        return;
    }

    close((int)tcp_fd);

    return;
}

int application_start(int argc, char **argv)
{
    int ret = 0;
    char buf[128];
    void *tcp_fd = NULL;
    void *ssl = NULL;

    tcp_fd = network_socket_create(server_name, server_port);
    if (tcp_fd == NULL) {
        printf("http client connect fail\n");
        return -1;
    } else {
        printf("network socket create ok\n");
    }

    ssl = mbedtls_ssl_connect(tcp_fd,
              mbedtls_test_ca_pem, strlen(mbedtls_test_ca_pem));
    if (ssl == NULL) {
        printf("ssl connect fail\n");
        ret = -1;
        goto _out;
    } else {
        printf("mbedtls ssl connect ok\n");
    }

    ret = mbedtls_ssl_send(ssl, (char *)alink_req_data,
                          (int)sizeof(alink_req_data));
    if (ret < 0) {
        printf("ssl send fail\n");
        goto _out;
    } else {
        printf("mbedtls ssl send ok\n");
    }

    ret = mbedtls_ssl_recv(ssl, buf, 128);
    if (ret < 0) {
        printf("ssl recv fail\n");
        goto _out;
    } else {
        printf("mbedtls ssl recv ok\n");
    }

    ret = 0;

_out:
    if (ssl != NULL) {
        mbedtls_ssl_close(ssl);
    }

    if (tcp_fd != NULL) {
        network_socket_destroy(tcp_fd);
    }

    return ret;
}


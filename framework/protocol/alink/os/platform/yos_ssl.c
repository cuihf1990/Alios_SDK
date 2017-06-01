/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "mbedtls/mbedtls_ssl.h"

void *platform_ssl_connect(_IN_ void *tcp_fd, _IN_ const char *server_cert, _IN_ int server_cert_len)
{
    return mbedtls_ssl_connect(tcp_fd, server_cert, server_cert_len);;
}

int platform_ssl_send(_IN_ void *ssl, _IN_ const char *buffer, _IN_ int length)
{
    return mbedtls_ssl_send(ssl, buffer, length);
}

int platform_ssl_recv(_IN_ void *ssl, _IN_ char *buffer, _IN_ int length)
{
    return mbedtls_ssl_recv(ssl, buffer, length);
}

int platform_ssl_close(_IN_ void *ssl)
{
    return mbedtls_ssl_close(ssl);
}

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

#ifndef __NET_WORK__
#define __NET_WORK__

/************************************ SSL interface ************************************/

/** @defgroup groupyos___ssl ssl
 *  @{
 */

/**
 * @brief Establish a ssl connection.
 *
 * @param[in] tcp_fd @n The network connection handle.
 * @param[in] server_cert @n Specify the sever certificate which is PEM format, and
 *          both root cert(CA) and user cert should be supported
 * @param[in] server_cert_len @n Length of sever certificate, in bytes.
 * @return SSL handle.
 * @see None.
 * @note None.
 */
void *yos__ssl_connect( void *tcp_fd,  const char *server_cert,  int server_cert_len);



/**
 * @brief Sends data on a ssl connection.
 *
 * @param[in] ssl @n A descriptor identifying a ssl connection.
 * @param[in] buffer @n A pointer to a buffer containing the data to be transmitted.
 * @param[in] length @n The length, in bytes, of the data pointed to by the buffer parameter.
 * @return
   @verbatim
   >  0: The total number of bytes sent, which can be less than the number indicated by length.
   <  0: Error occur.
   @endverbatim
 * @see None.
 * @note Blocking API.
 */
int yos__ssl_send( void *ssl,  const char *buffer,  int length);


/**
 * @brief Receives data from a ssl connection.
 *
 * @param[in] ssl @n A descriptor identifying a ssl connection.
 * @param[out] buffer @n A pointer to a buffer to receive incoming data.
 * @param[in] length @n The length, in bytes, of the data pointed to by the buffer parameter.
 * @return
   @verbatim
   >  0: The total number of bytes received, which can be less than the number indicated by length.
   <  0: Error occur.
   @endverbatim
 *
 * @see None.
 * @note blocking API.
 */
int yos__ssl_recv( void *ssl,  char *buffer,  int length);


/**
 * @brief Closes an existing ssl connection.
 *
 * @param[in] ssl: @n the specified connection.
 * @return None.
 * @see None.
 * @note None.
 */
int yos__ssl_close( void *ssl);

#endif

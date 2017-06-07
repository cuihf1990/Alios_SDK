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

#define _SOCKET_MAXNUMS         (10)
#define _WAIT_INFINITE          (~0)
#define _INVALID_FD             ((void *)-1)

#define YOS_WAIT_INFINITE          (~0)
#define CONFIG_MAX_MESSAGE_LENGTH 4096
/** @defgroup group_network network
 *  @{
 */

/**
 * @brief this is a network address structure, including host(ip or host name) and port.
 */
typedef struct {
    char *host; /**< host ip(dotted-decimal notation) or host name(string) */
    uint16_t port; /**< udp port or tcp port */
}  netaddr_t, *p_netaddr_t;

#define PLATFORM_SOCKET_MAXNUMS         (10)


/**
 * @brief Create a udp server with the specified port.
 *
 * @param[in] port @n The specified udp sever listen port.
 * @return Server handle.
   @verbatim
   =  NULL: fail.
   != NULL: success.
   @endverbatim
 * @see None.
 * @note It is recommended to add handle value by 1, if 0(NULL) is a valid handle value in your platform.
 */
void *yos_udp_server_create( uint16_t port);



/**
 * @brief Create a udp client.
 *
 * @param None
 * @return Client handle.
   @verbatim
   =  NULL: fail.
   != NULL: success.
   @endverbatim
 * @see None.
 * @note None.
 */
void *udp_client_create(void);



/**
 * @brief Add this host to the specified udp multicast group.
 *
 * @param[in] netaddr @n Specify multicast address.
 * @return Multicast handle.
   @verbatim
   =  NULL: fail.
   != NULL: success.
   @endverbatim
 * @see None.
 * @note None.
 *
 */
void *yos_udp_multicast_server_create( netaddr_t *netaddr);

/**
 * @brief Closes an existing udp connection.
 *
 * @param[in] handle @n the specified connection.
 * @return None.
 * @see None.
 * @note None.
 */
void  yos_udp_close(void *handle);



/**
 * @brief Sends data to a specific destination.
 *
 * @param[in] handle @n A descriptor identifying a connection.
 * @param[in] buffer @n A pointer to a buffer containing the data to be transmitted.
 * @param[in] length @n The length, in bytes, of the data pointed to by the buffer parameter.
 * @param[in] netaddr @n A pointer to a netaddr structure that contains the address of the target.
 *
 * @return
   @verbatim
   > 0: the total number of bytes sent, which can be less than the number indicated by length.
   = -1: error occur.
   @endverbatim
 * @see None.
 * @note blocking API.
 */
int  yos_udp_sendto(
    void *handle,
    const char *buffer,
    uint32_t length,
    netaddr_t *netaddr);


/**
 * @brief Receives data from a udp connection.
 *
 * @param[in] handle @n A descriptor identifying a connection.
 * @param[out] buffer @n A pointer to a buffer to receive incoming data.
 * @param[in] length @n The length, in bytes, of the data pointed to by the buffer parameter.
 * @param[out] netaddr @n A pointer to a netaddr structure that contains the address of the source.
 * @return
   @verbatim
   >  0: The total number of bytes received, which can be less than the number indicated by length.
   <  0: Error occur.
   @endverbatim
 *
 * @see None.
 * @note blocking API.
 */
int  yos_udp_recvfrom(
    void *handle,
    char *buffer,
    uint32_t length,
    netaddr_t *netaddr);



/**
 * @brief Create a tcp server with the specified port.
 *
 * @param[in] port @n The specified tcp sever listen port.
 * @return Server handle.
   @verbatim
   =  NULL: fail.
   != NULL: success.
   @endverbatim
 * @see None.
 * @note None.
 */
void *yos_tcp_server_create( uint16_t port);



/**
 * @brief Permits an incoming connection attempt on a tcp server.
 *
 * @param[in] server @n The specified tcp sever.
 * @return Connection handle.
 * @see None.
 * @note None.
 */
void *yos_tcp_server_accept( void *server);



/**
 * @brief Establish a connection.
 *
 * @param[in] netaddr @n The destination address.
 * @return Connection handle
   @verbatim
   =  NULL: fail.
   != NULL: success.
   @endverbatim
 * @see None.
 * @note the func will block until tcp connect success or fail.
 */
void *yos_tcp_client_connect(  netaddr_t *netaddr);




/**
 * @brief Sends data on a connection.
 *
 * @param[in] handle @n A descriptor identifying a connection.
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
int  yos_tcp_send( void *handle,  const char *buffer,  uint32_t length);



/**
 * @brief Receives data from a tcp connection.
 *
 * @param[in] handle @n A descriptor identifying a connection.
 * @param[out] buffer @n A pointer to a buffer to receive incoming data.
 * @param[in] length @n The length, in bytes, of the data pointed to by the buffer parameter.
 * @return
   @verbatim
   >  0: The total number of bytes received, which can be less than the number indicated by length.
   <  0: Error occur.
   @endverbatim
 *
 * @see None.
 * @note Blocking API.
 */
int  yos_tcp_recv( void *handle,  char *buffer,  uint32_t length);



/**
 * @brief Closes an existing tcp connection.
 *
 * @param[in] handle @n the specified connection.
 * @return None.
 * @see None.
 * @note None.
 */
void  yos_tcp_close( void *handle);




/**
 * @brief Determines the status of one or more connection, waiting if necessary, to perform synchronous I/O.
 *
 * @param[in,out] handle_read @n
   @verbatim
   [in]: An optional pointer to a set of connection to be checked for readability.
         handle_read[n] > 0, care the connection, and the value is handle of the careful connection.
         handle_read[n] = NULL, uncare.
   [out]: handle_read[n] = NULL, the connection unreadable; != NULL, the connection readable.
   @endverbatim
 * @param[in,out] handle_write: @n
   @verbatim
   [in]: An optional pointer to a set of connection to be checked for writability.
         handle_write[n] > 0, care the connection, and the value is handle of the careful connection.
         handle_write[n] = NULL, uncare.
   [out]: handle_write[n] = NULL, the connection unwritable; != NULL, the connection wirteable.
   @endverbatim
 * @param[in] timeout_ms: @n Timeout interval in millisecond.
 * @return
   @verbatim
   =  0: The timeout interval elapsed.
   >  0: The total number of connection handles that are ready.
   <  0: A connection error occur.
   @endverbatim
 * @see None.
 * @note None.
 */
int  yos_select(
    void *read_fds[PLATFORM_SOCKET_MAXNUMS],
    void *write_fds[PLATFORM_SOCKET_MAXNUMS],
    int timeout_ms);

#endif


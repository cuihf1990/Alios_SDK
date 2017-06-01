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

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "network.h"

static int __create_socket( p_netaddr_t netaddr, int type, struct sockaddr_in *paddr, long *psock)
{
	struct hostent *hp;
	struct in_addr in;
	uint32_t ip;
	int opt_val = 1;

    assert(paddr && psock);

    if (NULL == netaddr->host)
    {
    	ip = htonl(INADDR_ANY);
    }
    else
    {
        /*
         * in some platform gethostbyname() will return bad result
         * if host is "255.255.255.255", so calling inet_aton first
         */
        if (inet_aton(netaddr->host, &in)) {
            ip = *(uint32_t *)&in;
        } else {
            hp = gethostbyname(netaddr->host);
            if (!hp) {
                printf("can't resolute the host address \n");
                return -1;
            }
            ip = *(uint32_t *)(hp->h_addr);
		}
    }

    *psock = socket(AF_INET, type, 0);
    if (*psock < 0)
    {
        perror("socket");
    	return -1;
    }

    memset(paddr, 0, sizeof(struct sockaddr_in));

    if (0 != setsockopt(*psock, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)))
    {
        perror("setsockopt");
        close((int)*psock);
        return -1;
    }
    
    if (type == SOCK_DGRAM) {
        if (0 != setsockopt(*psock, SOL_SOCKET, SO_BROADCAST, &opt_val, sizeof(opt_val)))
        {
            perror("setsockopt");
            close((int)*psock);
            return -1;
        }
    }

    paddr->sin_addr.s_addr = ip;
	paddr->sin_family = AF_INET;
	paddr->sin_port = htons( netaddr->port );

	return 0;
}

void *yos_udp_server_create( uint16_t port)
{
	struct sockaddr_in addr;
	long server_socket;
	netaddr_t netaddr = {NULL, port};

	if (0 != __create_socket(&netaddr, SOCK_DGRAM, &addr, &server_socket))
	{
		return  _INVALID_FD;
	}

    if (-1 == bind(server_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)))
    {
        udp_close((void *)server_socket);
    	return  _INVALID_FD;
    }

    return (void *)server_socket;
}

void *yos_udp_client_create(void)
{
	struct sockaddr_in addr;
	long sock;
	netaddr_t netaddr = {NULL, 0};

	if (0 != __create_socket(&netaddr, SOCK_DGRAM, &addr, &sock))
	{
		return  _INVALID_FD;
	}

	return (void *)sock;
}



void *yos_udp_multicast_server_create(p_netaddr_t netaddr)
{
	int option = 1;
	struct sockaddr_in addr;
	long sock;
	struct ip_mreq mreq;
    /* Note: ignore host right now */
	netaddr_t netaddr_client = {NULL, netaddr->port};

	memset(&addr, 0, sizeof(addr));
	memset(&mreq, 0, sizeof(mreq));

	if (0 != __create_socket(&netaddr_client, SOCK_DGRAM, &addr, &sock))
	{
		return  _INVALID_FD;
	}

	/* allow multiple sockets to use the same PORT number */
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option)) < 0)
	{
        perror("setsockopt");

		udp_close((void *)sock);
		//do something.
		return  _INVALID_FD;
    }

	if (-1 == bind(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)))
	{
        perror("bind");
		udp_close((void *)sock);
		return  _INVALID_FD;
	}

	mreq.imr_multiaddr.s_addr = inet_addr(netaddr->host);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &mreq, sizeof(mreq)) < 0)
	{
		printf("setsockopt error \n");
		udp_close((void *)sock);
		return  _INVALID_FD;
	}

	return (void *)sock;
}



void yos_udp_close(void *handle)
{
	close((long)handle);
}



int yos_udp_sendto(
		 void *handle,
		 const char *buffer,
		 uint32_t length,
		 p_netaddr_t netaddr)
{
	int ret;
	uint32_t ip;
    struct in_addr in;
	struct hostent *hp;
	struct sockaddr_in addr;

    /*
     * in some gethostbyname() will return bad result
     * if host is "255.255.255.255", so calling inet_aton first
     */
    if (inet_aton(netaddr->host, &in)) {
        ip = *(uint32_t *)&in;
    } else {
        hp = gethostbyname(netaddr->host);
        if (!hp) {
            printf("can't resolute the host address \n");
            return -1;
        }
        ip = *(uint32_t *)(hp->h_addr);
    }

	addr.sin_addr.s_addr = ip;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(netaddr->port);

	ret = sendto((long)handle,
					   buffer,
					   length,
					   0,
					   (struct sockaddr *)&addr,
					   sizeof(struct sockaddr_in));

    if (ret < 0)
        perror("sendto");

	return (ret) > 0 ? ret : -1;
}


int yos_udp_recvfrom(
		 void *handle,
		 char *buffer,
		 uint32_t length,
		 p_netaddr_t netaddr)
{
	int ret;
	struct sockaddr_in addr;
	int addr_len = sizeof(addr);

	ret = recvfrom((long)handle, buffer, length, 0, (struct sockaddr *)&addr, &addr_len);
	if (ret > 0)
	{
		if (NULL != netaddr)
		{
            netaddr->port = ntohs(addr.sin_port);
            
            if (NULL != netaddr->host)
            {
                strcpy(netaddr->host, inet_ntoa(addr.sin_addr));
            }
		}

		return ret;
	}

	return -1;
}



void *yos_tcp_server_create( uint16_t port)
{
	struct sockaddr_in addr;
	long server_socket;
	 netaddr_t netaddr = {NULL, port};

	if (0 != __create_socket(&netaddr, SOCK_STREAM, &addr, &server_socket))
	{
		return  _INVALID_FD;
	}

    if (-1 == bind(server_socket, (struct sockaddr *)&addr, sizeof(addr)))
    {
    	 tcp_close((void *)server_socket);
    	return  _INVALID_FD;
    }

    if (0 != listen(server_socket, SOMAXCONN))
	{
    	 tcp_close((void *)server_socket);
		return  _INVALID_FD;
	}

    return (void *)server_socket;
}




void * yos_tcp_server_accept( void *server)
{
	struct sockaddr_in addr;
	int addr_length = sizeof(addr);
	long new_client;

	if ((new_client = accept((long)server,(struct sockaddr*)&addr, &addr_length)) <= 0)
	{
		return  _INVALID_FD;
	}

	return (void *)new_client;
}




void * yos_tcp_client_connect( p_netaddr_t netaddr)
{
	struct sockaddr_in addr;
	long sock;

	if (0 != __create_socket(netaddr, SOCK_STREAM, &addr, &sock))
	{
		return  _INVALID_FD;
	}

	if (-1 == connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)))
	{
		 tcp_close((void *)sock);
		return  _INVALID_FD;
	}

	return (void *)sock;
}



int  yos_tcp_send( void *handle,  const char *buffer,  uint32_t length)
{
	int bytes_sent;

	bytes_sent = send((long)handle, buffer, length, 0);
	return bytes_sent > 0 ? bytes_sent : -1;
}



int  yos_tcp_recv( void *handle,  char *buffer,  uint32_t length)
{
	int bytes_received;

	bytes_received = recv((long)handle, buffer, length, 0);

	return bytes_received > 0 ? bytes_received : -1;
}




void  yos_tcp_close( void *handle)
{
	close((long)handle);
}



int  yos_select(void *read_fds[ _SOCKET_MAXNUMS],
		void *write_fds[ _SOCKET_MAXNUMS],
		int timeout_ms)
{
	int i, ret = -1;
	struct timeval timeout_value;
	struct timeval *ptimeval = &timeout_value;
	fd_set r_set, w_set;

	if ( _WAIT_INFINITE == timeout_ms)
		ptimeval = NULL;
	else {
		ptimeval->tv_sec = timeout_ms / 1000;
		ptimeval->tv_usec = (timeout_ms % 1000) * 1000;
	}

    FD_ZERO(&r_set);
    FD_ZERO(&w_set);

    if (read_fds) {
        for (i = 0; i <  _SOCKET_MAXNUMS; ++i) {
            if ( _INVALID_FD != read_fds[i])
                FD_SET((long)read_fds[i], &r_set);
        }
    }

    if (write_fds) {
        for (i = 0; i <  _SOCKET_MAXNUMS; ++i) {
            if (  _INVALID_FD != write_fds[i] )
                FD_SET((long)write_fds[i], &w_set);
        }
    }

	ret = select(FD_SETSIZE, &r_set, &w_set, NULL, ptimeval);

	if (ret > 0) {
		if (read_fds) {
			for (i = 0; i <  _SOCKET_MAXNUMS; ++i) {
				if ( _INVALID_FD != read_fds[i]
                        && !FD_ISSET((long)read_fds[i], &r_set))
					read_fds[i] =  _INVALID_FD;
			}
		}

		if (write_fds) {
			for (i = 0; i <  _SOCKET_MAXNUMS; ++i) {
				if ( _INVALID_FD != write_fds[i]
                        && !FD_ISSET((long)write_fds[i], &w_set))
					write_fds[i] =  _INVALID_FD;
			}
		}
	} else {/* clear all fd */
		if (read_fds) {
			for (i = 0; i <  _SOCKET_MAXNUMS; ++i)
					read_fds[i] =  _INVALID_FD;
		}

		if (write_fds) {
			for (i = 0; i <  _SOCKET_MAXNUMS; ++i)
					write_fds[i] =  _INVALID_FD;
		}
    }

	return (ret >= 0) ? ret : -1;
}



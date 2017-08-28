#ifndef YOS_NETWORK_API_H
#define YOS_NETWORK_API_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <errno.h>
#include <fcntl.h>

/* network */
#ifndef WITH_LWIP
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/eventfd.h>
#else
#include <lwip/def.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#endif

#ifdef __cplusplus
}
#endif

#endif /* YOS_NETWORK_API_H */

#ifndef _SAL_SOCKETS_INTERNAL_H_
#define _SAL_SOCKETS_INTERNAL_H_

#define AF_UNSPEC       0
#define AF_INET         2
#define AF_INET6        10
#define PF_INET         AF_INET
#define PF_INET6        AF_INET6
#define PF_UNSPEC       AF_UNSPEC

/* Socket protocol types (TCP/UDP/RAW) */
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3

/* Define generic types used in sal */
#if !SAL_NO_STDINT_H
#include <stdint.h>
typedef uint8_t   u8_t;
typedef int8_t    s8_t;
typedef uint16_t  u16_t;
typedef int16_t   s16_t;
typedef uint32_t  u32_t;
typedef int32_t   s32_t;
#endif

/* If your port already typedef's sa_family_t, define SA_FAMILY_T_DEFINED
   to prevent this code from redefining it. */
#if !defined(sa_family_t) && !defined(SA_FAMILY_T_DEFINED)
typedef u8_t sa_family_t;
#endif
/* If your port already typedef's in_port_t, define IN_PORT_T_DEFINED
   to prevent this code from redefining it. */
#if !defined(in_port_t) && !defined(IN_PORT_T_DEFINED)
typedef u16_t in_port_t;
#endif

#define DNS_MAX_NAME_LENGTH 256

struct sockaddr {
  u8_t        sa_len;
  sa_family_t sa_family;
  char        sa_data[14];
};

/* If your port already typedef's in_addr_t, define IN_ADDR_T_DEFINED
   to prevent this code from redefining it. */
#if !defined(in_addr_t) && !defined(IN_ADDR_T_DEFINED)
typedef u32_t in_addr_t;
#endif

struct in_addr {
  in_addr_t s_addr;
};

struct in6_addr {
  union {
    u32_t u32_addr[4];
    u8_t  u8_addr[16];
  } un;
#define s6_addr  un.u8_addr
};

/* members are in network byte order */
struct sockaddr_in {
  u8_t            sin_len;
  sa_family_t     sin_family;
  in_port_t       sin_port;
  struct in_addr  sin_addr;
#define SIN_ZERO_LEN 8
  char            sin_zero[SIN_ZERO_LEN];
};

struct sockaddr_in6 {
  u8_t            sin6_len;      /* length of this structure    */
  sa_family_t     sin6_family;   /* AF_INET6                    */
  in_port_t       sin6_port;     /* Transport layer port #      */
  u32_t           sin6_flowinfo; /* IPv6 flow information       */
  struct in6_addr sin6_addr;     /* IPv6 address                */
  u32_t           sin6_scope_id; /* Set of interfaces for scope */
};

struct hostent {
    char  *h_name;      /* Official name of the host. */
    char **h_aliases;   /* A pointer to an array of pointers to alternative
                           host names, terminated by a null pointer. */
    int    h_addrtype;  /* Address type. */
    int    h_length;    /* The length, in bytes, of the address. */
    char **h_addr_list; /* A pointer to an array of pointers to network
                           addresses (in network byte order) for the host,
                           terminated by a null pointer. */
#define h_addr h_addr_list[0] /* for backward compatibility */
};

enum sal_ip_addr_type {
  /** IPv4 */
  IPADDR_TYPE_V4 =   0U,
  /** IPv6 */
  IPADDR_TYPE_V6 =   6U,
  /** IPv4+IPv6 ("dual-stack") */
  IPADDR_TYPE_ANY = 46U
};

typedef struct ip4_addr {
  u32_t addr;
} ip4_addr_t;

typedef struct ip6_addr {
  u32_t addr[4];
} ip6_addr_t;

typedef struct _ip_addr {
  union {
    ip6_addr_t ip6;
    ip4_addr_t ip4;
  } u_addr;
  /** @ref sal_ip_addr_type */
  u8_t type;
} ip_addr_t;

#define  SOL_SOCKET  0xfff    /* options for socket level */

/*
 * Option flags per-socket. These must match the SOF_ flags in ip.h (checked in init.c)
 */
#define SO_REUSEADDR   0x0004 /* Allow local address reuse */
#define SO_KEEPALIVE   0x0008 /* keep connections alive */
#define SO_BROADCAST   0x0020 /* permit to send and to receive broadcast messages (see IP_SOF_BROADCAST option) */

/*
 * Additional options, not kept in so_options.
 */
#define SO_DEBUG       0x0001 /* Unimplemented: turn on debugging info recording */
#define SO_ACCEPTCONN  0x0002 /* socket has had listen() */
#define SO_DONTROUTE   0x0010 /* Unimplemented: just use interface addresses */
#define SO_USELOOPBACK 0x0040 /* Unimplemented: bypass hardware when possible */
#define SO_LINGER      0x0080 /* linger on close if data present */
#define SO_DONTLINGER  ((int)(~SO_LINGER))
#define SO_OOBINLINE   0x0100 /* Unimplemented: leave received OOB data in line */
#define SO_REUSEPORT   0x0200 /* Unimplemented: allow local address & port reuse */
#define SO_SNDBUF      0x1001 /* Unimplemented: send buffer size */
#define SO_RCVBUF      0x1002 /* receive buffer size */
#define SO_SNDLOWAT    0x1003 /* Unimplemented: send low-water mark */
#define SO_RCVLOWAT    0x1004 /* Unimplemented: receive low-water mark */
#define SO_SNDTIMEO    0x1005 /* send timeout */
#define SO_RCVTIMEO    0x1006 /* receive timeout */
#define SO_ERROR       0x1007 /* get error status and clear */
#define SO_TYPE        0x1008 /* get socket type */
#define SO_CONTIMEO    0x1009 /* Unimplemented: connect timeout */
#define SO_NO_CHECK    0x100a /* don't create UDP checksum */

int sal_select(int maxfdp1, fd_set *readset, fd_set *writeset,
               fd_set *exceptset, struct timeval *timeout);

int sal_socket(int domain, int type, int protocol);

int sal_write(int s, const void *data, size_t size);

int sal_connect(int s, const struct sockaddr *name, socklen_t namelen);

int sal_eventfd(unsigned int initval, int flags);

int sal_setsockopt(int s, int level, int optname,
                   const void *optval, socklen_t optlen);

int sal_getsockopt(int s, int level, int optname,
                   void *optval, socklen_t *optlen);

struct hostent* sal_gethostbyname(const char *name);

int sal_close(int s);

int sal_init();

int sal_send(int s, const void *data, size_t size, int flags);

#define select(maxfdp1,readset,writeset,exceptset,timeout) \
        sal_select(maxfdp1,readset,writeset,exceptset,timeout)

#define write(s,data,size) \
        sal_write(s,data,size)

#define socket(domain,type,protocol) \
        sal_socket(domain,type,protocol)

#define connect(s,name,namelen) \
        sal_connect(s,name,namelen)

#define eventfd(initval,flags) \
        sal_eventfd(initval,flags)

#define setsockopt(s,level,optname,optval,optlen) \
        sal_setsockopt(s,level,optname,optval,optlen)

#define getsockopt(s,level,optname,optval,optlen) \
        sal_getsockopt(s,level,optname,optval,optlen)

#define gethostbyname(name) \
        sal_gethostbyname(name)

#define close(s) \
        sal_close(s)

#define send(s,data,size,flags) \
        sal_send(s,data,size,flags)

#endif

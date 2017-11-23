#ifndef AOS_SAL_SOCKET_H
#define AOS_SAL_SOCKET_H

#include <sys/time.h>
#include <stdlib.h>
#include <aos/aos.h>
#include "vfs_conf.h"
#include "sal_arch.h"
#include "sal_pcb.h"
#include "sal.h"

#ifdef __cplusplus
extern "C" {
#endif
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
typedef uintptr_t mem_ptr_t;
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
    char **h_aliases;   /* A pointer to an array of pointers to alternative host names,
                           terminated by a null pointer. */
    int    h_addrtype;  /* Address type. */
    int    h_length;    /* The length, in bytes, of the address. */
    char **h_addr_list; /* A pointer to an array of pointers to network addresses (in
                           network byte order) for the host, terminated by a null pointer. */
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

int sal_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
               struct timeval *timeout);

int sal_socket(int domain, int type, int protocol);

int sal_write(int s, const void *data, size_t size);

int sal_connect(int s, const struct sockaddr *name, socklen_t namelen);

#define select(maxfdp1,readset,writeset,exceptset,timeout)     sal_select(maxfdp1,readset,writeset,exceptset,timeout)

#define write(s, data, size)                                   sal_write(s,data,size)

#define socket(domain,type,protocol)                           sal_socket( domain,type,protocol)

#define connect(s, name, namelen)                              sal_connect(s, name, namelen)

#define MEMP_NUM_NETCONN     5//(MAX_SOCKETS_TCP + MAX_LISTENING_SOCKETS_TCP + MAX_SOCKETS_UDP)

#define SAL_TAG  "sal"


#define SAL_DEBUG(format, ...)  LOGD(SAL_TAG, format, ##__VA_ARGS__)
#define SAL_ERROR(format, ...)  LOGE(SAL_TAG, format, ##__VA_ARGS__)

/* Helpers to process several netconn_types by the same code */
#define NETCONNTYPE_GROUP(t)         ((t)&0xF0)
#define NETCONNTYPE_DATAGRAM(t)      ((t)&0xE0)

#if SAL_NETCONN_SEM_PER_THREAD
#define SELECT_SEM_T        sal_sem_t*
#define SELECT_SEM_PTR(sem) (sem)
#else /* SAL_NETCONN_SEM_PER_THREAD */
#define SELECT_SEM_T        sal_sem_t
#define SELECT_SEM_PTR(sem) (&(sem))
#endif /* SAL_NETCONN_SEM_PER_THREAD */

/* FD_SET used for event_select */
#ifndef FD_SET
#undef  FD_SETSIZE
/* Make FD_SETSIZE match NUM_SOCKETS in socket.c */
#define FD_SETSIZE    MEMP_NUM_NETCONN
#define FDSETSAFESET(n, code) do { \
  if (((n) - SAL_SOCKET_OFFSET < MEMP_NUM_NETCONN) && (((int)(n) - SAL_SOCKET_OFFSET) >= 0)) { \
  code; }} while(0)
#define FDSETSAFEGET(n, code) (((n) - SAL_SOCKET_OFFSET < MEMP_NUM_NETCONN) && (((int)(n) - SAL_SOCKET_OFFSET) >= 0) ?\
  (code) : 0)
//#define FD_SET(n, p)  FDSETSAFESET(n, (p)->fd_bits[((n)-SAL_SOCKET_OFFSET)/8] |=  (1 << (((n)-SAL_SOCKET_OFFSET) & 7)))
#define FD_CLR(n, p)  FDSETSAFESET(n, (p)->fd_bits[((n)-SAL_SOCKET_OFFSET)/8] &= ~(1 << (((n)-SAL_SOCKET_OFFSET) & 7)))
#define FD_ISSET(n,p) FDSETSAFEGET(n, (p)->fd_bits[((n)-SAL_SOCKET_OFFSET)/8] &   (1 << (((n)-SAL_SOCKET_OFFSET) & 7)))
#define FD_ZERO(p)    memset((void*)(p), 0, sizeof(*(p)))

typedef struct fd_set {
    unsigned char fd_bits [(FD_SETSIZE * 2 + 7) / 8];
} fd_set;

#elif SAL_SOCKET_OFFSET
#error SAL_SOCKET_OFFSET does not work with external FD_SET!
#else
#include <fcntl.h>
#endif /* FD_SET */

#if defined(AOS_CONFIG_VFS_DEV_NODES)
#define SAL_SOCKET_OFFSET              AOS_CONFIG_VFS_DEV_NODES
#endif

/** Description for a task waiting in select */
struct sal_select_cb {
    /** Pointer to the next waiting task */
    struct sal_select_cb *next;
    /** Pointer to the previous waiting task */
    struct sal_select_cb *prev;
    /** readset passed to select */
    fd_set *readset;
    /** writeset passed to select */
    fd_set *writeset;
    /** unimplemented: exceptset passed to select */
    fd_set *exceptset;
    /** don't signal the same semaphore twice: set to 1 when signalled */
    int sem_signalled;
    /** semaphore to wake up a task waiting for select */
    SELECT_SEM_T sem;
};

/** Current state of the netconn. Non-TCP netconns are always
 * in state NETCONN_NONE! */
enum netconn_state {
    NETCONN_NONE,
    NETCONN_WRITE,
    NETCONN_LISTEN,
    NETCONN_CONNECT,
    NETCONN_CLOSE
};

/** @ingroup netconn_common
 * Protocol family and type of the netconn
 */
enum netconn_type {
  NETCONN_INVALID     = 0,
  /** TCP IPv4 */
  NETCONN_TCP         = 0x10,
#if SAL_IPV6
  /** TCP IPv6 */
  NETCONN_TCP_IPV6    = NETCONN_TCP | NETCONN_TYPE_IPV6 /* 0x18 */,
#endif /* SAL_IPV6 */
  /** UDP IPv4 */
  NETCONN_UDP         = 0x20,
  /** UDP IPv4 lite */
  NETCONN_UDPLITE     = 0x21,
  /** UDP IPv4 no checksum */
  NETCONN_UDPNOCHKSUM = 0x22,

#if SAL_IPV6
  /** UDP IPv6 (dual-stack by default, unless you call @ref netconn_set_ipv6only) */
  NETCONN_UDP_IPV6         = NETCONN_UDP | NETCONN_TYPE_IPV6 /* 0x28 */,
  /** UDP IPv6 lite (dual-stack by default, unless you call @ref netconn_set_ipv6only) */
  NETCONN_UDPLITE_IPV6     = NETCONN_UDPLITE | NETCONN_TYPE_IPV6 /* 0x29 */,
  /** UDP IPv6 no checksum (dual-stack by default, unless you call @ref netconn_set_ipv6only) */
  NETCONN_UDPNOCHKSUM_IPV6 = NETCONN_UDPNOCHKSUM | NETCONN_TYPE_IPV6 /* 0x2a */,
#endif /* SAL_IPV6 */

  /** Raw connection IPv4 */
  NETCONN_RAW         = 0x40
#if SAL_IPV6
  /** Raw connection IPv6 (dual-stack by default, unless you call @ref netconn_set_ipv6only) */
  , NETCONN_RAW_IPV6    = NETCONN_RAW | NETCONN_TYPE_IPV6 /* 0x48 */
#endif /* SAL_IPV6 */
};

struct sal_netconn;
/** A callback prototype to inform about events for a netconn */
typedef void (* netconn_callback)(struct sal_netconn *conn, enum netconn_evt, u16_t len);

/** A netconn descriptor */
typedef struct sal_netconn {
  int socket;
    /** type of the netconn (TCP, UDP or RAW) */
  enum netconn_type type;
  /** current state of the netconn */
  enum netconn_state state;

    /** the SAL internal protocol control block */
  union {
    struct ip_pcb  *ip;
    struct tcp_pcb *tcp;
    struct udp_pcb *udp;
    struct raw_pcb *raw;
  } pcb;
  /** the last error this netconn had */
  err_t last_err;
  /** flags holding more netconn-internal state, see NETCONN_FLAG_* defines */
  u8_t flags;
#if SAL_SNDTIMEO
  /** timeout to wait for sending data (which means enqueueing data for sending
      in internal buffers) in milliseconds */
  s32_t send_timeout;
#endif /* SAL_SNDTIMEO */
#if SAL_RCVTIMEO
  /** timeout in milliseconds to wait for new data to be received
      (or connections to arrive for listening netconns) */
  int recv_timeout;
#endif /* SAL_RCVTIMEO */
#if SAL_RCVBUF
  /** maximum amount of bytes queued in recvmbox
      not used for TCP: adjust TCP_WND instead! */
  int recv_bufsize;
  /** number of bytes currently in recvmbox to be received,
      tested against recv_bufsize to limit bytes on recvmbox
      for UDP and RAW, used for FIONREAD */
  int recv_avail;
#endif /* SAL_RCVBUF */
  /** A callback function that is informed about events for this netconn */
  netconn_callback callback;
}sal_netconn_t;

#define AF_UNSPEC       0
#define AF_INET         2
#define AF_INET6        10
#define PF_INET         AF_INET
#define PF_INET6        AF_INET6
#define PF_UNSPEC       AF_UNSPEC

void sal_deal_event(int s, enum netconn_evt evt);

#define DNS_MAX_NAME_LENGTH 256

#define API_EVENT_SIMPLE(s,e) sal_deal_event(s,e)

#ifdef __cplusplus
}
#endif

#endif /* __AOS_EVENTFD_H__ */

#ifndef AOS_SAL_SOCKET_H
#define AOS_SAL_SOCKET_H

#include <sys/time.h>
#include <stdlib.h>
#include <aos/aos.h>
#include "vfs_conf.h"
#include "sal_arch.h"
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

#define AF_UNSPEC       0
#define AF_INET         2
#define AF_INET6        10
#define PF_INET         AF_INET
#define PF_INET6        AF_INET6
#define PF_UNSPEC       AF_UNSPEC

void sal_deal_event(int s, enum netconn_evt evt);

#define API_EVENT_SIMPLE(s,e) sal_deal_event(s,e)

#ifdef __cplusplus
}
#endif

#endif /* __AOS_EVENTFD_H__ */

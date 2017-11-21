#ifndef AOS_SAL_SOCKET_H
#define AOS_SAL_SOCKET_H

#include <sys/time.h>
#include <stdlib.h>
#include <aos/aos.h>
#include "vfs_conf.h"
#include "sal.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Socket protocol types (TCP/UDP/RAW) */
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3

int sal_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
               struct timeval *timeout);

int sal_socket(int domain, int type, int protocol);

int sal_write(int s, const void *data, size_t size);

#define select(maxfdp1,readset,writeset,exceptset,timeout)     sal_select(maxfdp1,readset,writeset,exceptset,timeout)

#define write(s, data, size)                                   sal_write(s,data,size)

#define socket(domain,type,protocol)                           sal_socket( domain,type,protocol)

#define MEMP_NUM_NETCONN     5//(MAX_SOCKETS_TCP + MAX_LISTENING_SOCKETS_TCP + MAX_SOCKETS_UDP)
#define SAL_DEBUG(format, ...)  LOGD(LOG_TAG, format"\r\n",##__VA_ARGS__)

#if SAL_NETCONN_SEM_PER_THREAD
#define SELECT_SEM_T        sys_sem_t*
#define SELECT_SEM_PTR(sem) (sem)
#else /* SAL_NETCONN_SEM_PER_THREAD */
#define SELECT_SEM_T        sys_sem_t
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
typedef int8_t err_t;
typedef aos_sem_t sys_sem_t;

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


/** Definitions for error constants. */
typedef enum {
    /** No error, everything OK. */
    ERR_OK         = 0,
    /** Out of memory error.     */
    ERR_MEM        = -1,
    /** Buffer error.            */
    ERR_BUF        = -2,
    /** Timeout.                 */
    ERR_TIMEOUT    = -3,
    /** Routing problem.         */
    ERR_RTE        = -4,
    /** Operation in progress    */
    ERR_INPROGRESS = -5,
    /** Illegal value.           */
    ERR_VAL        = -6,
    /** Operation would block.   */
    ERR_WOULDBLOCK = -7,
    /** Address in use.          */
    ERR_USE        = -8,
    /** Already connecting.      */
    ERR_ALREADY    = -9,
    /** Conn already established.*/
    ERR_ISCONN     = -10,
    /** Not connected.           */
    ERR_CONN       = -11,
    /** Low-level netif error    */
    ERR_IF         = -12,
    /** Connection aborted.      */
    ERR_ABRT       = -13,
    /** Connection reset.        */
    ERR_RST        = -14,
    /** Connection closed.       */
    ERR_CLSD       = -15,
    /** Illegal argument.        */
    ERR_ARG        = -16
} err_enum_t;

enum netconn_evt {
    NETCONN_EVT_RCVPLUS,
    NETCONN_EVT_RCVMINUS,
    NETCONN_EVT_SENDPLUS,
    NETCONN_EVT_SENDMINUS,
    NETCONN_EVT_ERROR
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


void sal_deal_event(int s, enum netconn_evt evt);


#define API_EVENT_SIMPLE(s,e) sal_deal_event(s,e)

#ifdef __cplusplus
}
#endif


#endif /* __AOS_EVENTFD_H__ */

#include <aos/aos.h>
#include <stdio.h>
#include <string.h>
#include "sal_sockets.h"
#include "sal_arch.h"
#include "err.h"

#define TAG  "sal_socket"

#define NUM_SOCKETS MEMP_NUM_NETCONN
#define NUM_EVENTS  MEMP_NUM_NETCONN

#define SAL_ASSERT(message, assertion)
static int  sal_selscan(int maxfdp1, fd_set *readset_in, fd_set *writeset_in, fd_set *exceptset_in,
                        fd_set *readset_out, fd_set *writeset_out, fd_set *exceptset_out);

static struct sal_sock *tryget_socket(int s);

static struct sal_event *tryget_event(int s);

#define SAL_EVENT_OFFSET (NUM_SOCKETS + SAL_SOCKET_OFFSET)
struct sal_event {
    uint64_t counts;
    int used;
    int reads;
    int writes;
    /** semaphore to wake up a task waiting for select */
    sal_sem_t *psem;
};

#ifndef SELWAIT_T
#define SELWAIT_T uint8_t
#endif

/** Contains all internal pointers and states used for a socket */
struct sal_sock {
    /** sockets currently are built on netconns, each socket has one netconn */
    /*conn may delete in sal*/
     at_conn_t *conn;
    /** data that was left from the previous read */
    void *lastdata;
    /** offset in the data that was left from the previous read */
    uint16_t lastoffset;
    /** number of times data was received, set by event_callback(),
        tested by the receive and select functions */
    int16_t rcvevent;
    /** number of times data was ACKed (free send buffer), set by event_callback(),
        tested by select */
    uint16_t sendevent;
    /** error happened for this socket, set by event_callback(), tested by select */
    uint16_t errevent;
    /** last error that occurred on this socket (in fact, all our errnos fit into an uint8_t) */
    uint8_t err;
    /** counter of how many threads are waiting for this socket using select */
    SELWAIT_T select_waiting;
};

#define IS_SOCK_ADDR_LEN_VALID(namelen)  ((namelen) == sizeof(struct sockaddr_in))

#ifndef set_errno
#define set_errno(err) do { if (err) { errno = (err); } } while(0)
#endif

#define sock_set_errno(sk, e) do { \
  const int sockerr = (e); \
  sk->err = (u8_t)sockerr; \
  set_errno(sockerr); \
} while (0)

/** The global array of available sockets */
static struct sal_sock sockets[NUM_SOCKETS];
/** The global array of available events */
static struct sal_event events[NUM_EVENTS];
/** The global list of tasks waiting for select */
static struct sal_select_cb *select_cb_list;
/** This counter is increased from sal_select when the list is changed
    and checked in event_callback to see if it has changed. */
static volatile int select_cb_ctr;

int sal_eventfd(unsigned int initval, int flags)
{
    int i;
    SAL_ARCH_DECL_PROTECT(lev);

    /* allocate a new socket identifier */
    for (i = 0; i < NUM_EVENTS; ++i) {
        /* Protect socket array */
        SAL_ARCH_PROTECT(lev);
        if (!events[i].used) {
            events[i].used = 1;
            events[i].counts = 0;
            events[i].reads = 0;
            events[i].writes = 0;
            events[i].psem = NULL;
            SAL_ARCH_UNPROTECT(lev);
            return i + SAL_EVENT_OFFSET;
        }
        SAL_ARCH_UNPROTECT(lev);
    }

    return -1;
}



int sal_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
               struct timeval *timeout)
{
    uint32_t waitres = 0;
    int nready;
    fd_set lreadset, lwriteset, lexceptset;
    uint32_t msectimeout;
    struct sal_select_cb select_cb;
    int i;
    int maxfdp2;
#if SAL_NETCONN_SEM_PER_THREAD
    int waited = 0;
#endif
    SAL_ARCH_DECL_PROTECT(lev);

    SAL_DEBUG(("sal_select(%d, %p, %p, %p, tvsec=%d tvusec=%d)\n",
                    maxfdp1, (void *)readset, (void *) writeset, (void *) exceptset,
                    timeout ? (int32_t)timeout->tv_sec : (int32_t) - 1,
                    timeout ? (int32_t)timeout->tv_usec : (int32_t) - 1));

    /* Go through each socket in each list to count number of sockets which
       currently match */
    nready = sal_selscan(maxfdp1, readset, writeset, exceptset, &lreadset, &lwriteset, &lexceptset);

    /* If we don't have any current events, then suspend if we are supposed to */
    if (!nready) {
        if (timeout && timeout->tv_sec == 0 && timeout->tv_usec == 0) {
            SAL_DEBUG(("sal_select: no timeout, returning 0\n"));
            /* This is OK as the local fdsets are empty and nready is zero,
               or we would have returned earlier. */
            goto return_copy_fdsets;
        }

        /* None ready: add our semaphore to list:
           We don't actually need any dynamic memory. Our entry on the
           list is only valid while we are in this function, so it's ok
           to use local variables. */

        select_cb.next = NULL;
        select_cb.prev = NULL;
        select_cb.readset = readset;
        select_cb.writeset = writeset;
        select_cb.exceptset = exceptset;
        select_cb.sem_signalled = 0;
#if SAL_NETCONN_SEM_PER_THREAD
        select_cb.sem = SAL_NETCONN_THREAD_SEM_GET();
#else /* SAL_NETCONN_SEM_PER_THREAD */
        if (sal_sem_new(&select_cb.sem, 0) != ERR_OK) {
            /* failed to create semaphore */
            set_errno(ENOMEM);
            return -1;
        }
#endif /* SAL_NETCONN_SEM_PER_THREAD */

        /* Protect the select_cb_list */
        SAL_ARCH_PROTECT(lev);

        /* Put this select_cb on top of list */
        select_cb.next = select_cb_list;
        if (select_cb_list != NULL) {
            select_cb_list->prev = &select_cb;
        }
        select_cb_list = &select_cb;
        /* Increasing this counter tells event_callback that the list has changed. */
        select_cb_ctr++;

        /* Now we can safely unprotect */
        SAL_ARCH_UNPROTECT(lev);

        /* Increase select_waiting for each socket we are interested in */
        maxfdp2 = maxfdp1;
        for (i = SAL_SOCKET_OFFSET; i < maxfdp1; i++) {
            if ((readset && FD_ISSET(i, readset)) ||
                (writeset && FD_ISSET(i, writeset)) ||
                (exceptset && FD_ISSET(i, exceptset))) {
                struct sal_sock *sock;
                struct sal_event *event;
                SAL_ARCH_PROTECT(lev);
                sock = tryget_socket(i);
                event = tryget_event(i);
                if (sock != NULL) {
                    sock->select_waiting++;
                    SAL_ASSERT("sock->select_waiting > 0", sock->select_waiting > 0);
                } else if (event != NULL) {
                    event->psem = SELECT_SEM_PTR(select_cb.sem);
                } else {
                    /* Not a valid socket */
                    nready = -1;
                    maxfdp2 = i;
                    SAL_ARCH_UNPROTECT(lev);
                    break;
                }
                SAL_ARCH_UNPROTECT(lev);
            }
        }

        if (nready >= 0) {
            /* Call sal_selscan again: there could have been events between
               the last scan (without us on the list) and putting us on the list! */
            nready = sal_selscan(maxfdp1, readset, writeset, exceptset, &lreadset, &lwriteset, &lexceptset);
            if (!nready) {
                /* Still none ready, just wait to be woken */
                if (timeout == 0) {
                    /* Wait forever */
                    msectimeout = 0;
                } else {
                    msectimeout =  ((timeout->tv_sec * 1000) + ((timeout->tv_usec + 500) / 1000));
                    if (msectimeout == 0) {
                        /* Wait 1ms at least (0 means wait forever) */
                        msectimeout = 1;
                    }
                }

                waitres = sal_arch_sem_wait(SELECT_SEM_PTR(select_cb.sem), msectimeout);
#if SAL_NETCONN_SEM_PER_THREAD
                waited = 1;
#endif
            }
        }

        /* Decrease select_waiting for each socket we are interested in */
        for (i = SAL_SOCKET_OFFSET; i < maxfdp2; i++) {
            if ((readset && FD_ISSET(i, readset)) ||
                (writeset && FD_ISSET(i, writeset)) ||
                (exceptset && FD_ISSET(i, exceptset))) {
                struct sal_sock *sock;
                struct sal_event *event;
                SAL_ARCH_PROTECT(lev);
                sock = tryget_socket(i);
                event = tryget_event(i);
                if (sock != NULL) {
                    /* @todo: what if this is a new socket (reallocated?) in this case,
                       select_waiting-- would be wrong (a global 'sockalloc' counter,
                       stored per socket could help) */
                    SAL_ASSERT("sock->select_waiting > 0", sock->select_waiting > 0);
                    if (sock->select_waiting > 0) {
                        sock->select_waiting--;
                    }
                } else if (event != NULL) {
                    event->psem = NULL;
                } else {
                    /* Not a valid socket */
                    nready = -1;
                }
                SAL_ARCH_UNPROTECT(lev);
            }
        }
        /* Take us off the list */
        SAL_ARCH_PROTECT(lev);
        if (select_cb.next != NULL) {
            select_cb.next->prev = select_cb.prev;
        }
        if (select_cb_list == &select_cb) {
            SAL_ASSERT("select_cb.prev == NULL", select_cb.prev == NULL);
            select_cb_list = select_cb.next;
        } else {
            SAL_ASSERT("select_cb.prev != NULL", select_cb.prev != NULL);
            select_cb.prev->next = select_cb.next;
        }
        /* Increasing this counter tells event_callback that the list has changed. */
        select_cb_ctr++;
        SAL_ARCH_UNPROTECT(lev);

#if SAL_NETCONN_SEM_PER_THREAD
        if (select_cb.sem_signalled && (!waited || (waitres == SAL_ARCH_TIMEOUT))) {
            /* don't leave the thread-local semaphore signalled */
            sal_arch_sem_wait(select_cb.sem, 1);
        }
#else /* SAL_NETCONN_SEM_PER_THREAD */
        sal_sem_free(&select_cb.sem);
#endif /* SAL_NETCONN_SEM_PER_THREAD */

        if (nready < 0) {
            /* This happens when a socket got closed while waiting */
            set_errno(EBADF);
            return -1;
        }

        if (waitres == SAL_ARCH_TIMEOUT) {
            /* Timeout */
            SAL_DEBUG(("sal_select: timeout expired\n"));
            /* This is OK as the local fdsets are empty and nready is zero,
               or we would have returned earlier. */
            goto return_copy_fdsets;
        }

        /* See what's set */
        nready = sal_selscan(maxfdp1, readset, writeset, exceptset, &lreadset, &lwriteset, &lexceptset);
    }

    SAL_DEBUG(("sal_select: nready=%d\n", nready));
return_copy_fdsets:
    set_errno(0);
    if (readset) {
        *readset = lreadset;
    }
    if (writeset) {
        *writeset = lwriteset;
    }
    if (exceptset) {
        *exceptset = lexceptset;
    }
    return nready;
}

//把有事件的标出来
static int sal_selscan(int maxfdp1, fd_set *readset_in, fd_set *writeset_in, fd_set *exceptset_in,
                       fd_set *readset_out, fd_set *writeset_out, fd_set *exceptset_out)
{
    int i, nready = 0;
    fd_set lreadset, lwriteset, lexceptset;
    struct sal_sock *sock;
    struct sal_event *event;
    SAL_ARCH_DECL_PROTECT(lev);

    FD_ZERO(&lreadset);
    FD_ZERO(&lwriteset);
    FD_ZERO(&lexceptset);

    /* Go through each socket in each list to count number of sockets which
       currently match */
    for (i = SAL_SOCKET_OFFSET; i < maxfdp1; i++) {
        /* if this FD is not in the set, continue */
        if (!(readset_in && FD_ISSET(i, readset_in)) &&
            !(writeset_in && FD_ISSET(i, writeset_in)) &&
            !(exceptset_in && FD_ISSET(i, exceptset_in))) {
            continue;
        }
        /* First get the socket's status (protected)... */
        SAL_ARCH_PROTECT(lev);
        sock = tryget_socket(i);
        event = tryget_event(i);
        if (sock != NULL || event != NULL) {
            void *lastdata = sock ? sock->lastdata : NULL;
            int16_t rcvevent = sock ? sock->rcvevent : event->reads;
            uint16_t sendevent = sock ? sock->sendevent : event->writes;
            uint16_t errevent = sock ? sock->errevent : 0;
            SAL_ARCH_UNPROTECT(lev);
            /* See if netconn of this socket is ready for read */
            if (readset_in && FD_ISSET(i, readset_in) && ((lastdata != NULL) || (rcvevent > 0))) {
                FD_SET(i, &lreadset);
                SAL_DEBUG(("sal_selscan: fd=%d ready for reading\n", i));
                nready++;
            }
            /* See if netconn of this socket is ready for write */
            if (writeset_in && FD_ISSET(i, writeset_in) && (sendevent != 0)) {
                FD_SET(i, &lwriteset);
                SAL_DEBUG(("sal_selscan: fd=%d ready for writing\n", i));
                nready++;
            }
            /* See if netconn of this socket had an error */
            if (exceptset_in && FD_ISSET(i, exceptset_in) && (errevent != 0)) {
                FD_SET(i, &lexceptset);
                SAL_DEBUG(("sal_selscan: fd=%d ready for exception\n", i));
                nready++;
            }
        } else {
            SAL_ARCH_UNPROTECT(lev);
            /* continue on to next FD in list */
        }
    }
    /* copy local sets to the ones provided as arguments */
    *readset_out = lreadset;
    *writeset_out = lwriteset;
    *exceptset_out = lexceptset;

    SAL_ASSERT("nready >= 0", nready >= 0);
    return nready;
}

static struct sal_event *
tryget_event(int s)
{
    s -= SAL_EVENT_OFFSET;
    if ((s < 0) || (s >= NUM_EVENTS)) {
        return NULL;
    }
    if (!events[s].used) {
        return NULL;
    }
    return &events[s];
}

/**
 * Same as get_socket but doesn't set errno
 *
 * @param s externally used socket index
 * @return struct sal_sock for the socket or NULL if not found
 */
static struct sal_sock *tryget_socket(int s)
{
    s -= SAL_SOCKET_OFFSET;
    if ((s < 0) || (s >= NUM_SOCKETS)) {
        return NULL;
    }
    if (!sockets[s].conn) {
        return NULL;
    }
    return &sockets[s];
}

static struct sal_sock *get_socket(int s)
{
    struct sal_sock *sock;

    s -= SAL_SOCKET_OFFSET;

    if ((s < 0) || (s >= NUM_SOCKETS)) {
        SAL_DEBUG(("get_socket(%d): invalid\n", s + SAL_SOCKET_OFFSET));
        set_errno(EBADF);
        return NULL;
    }

    sock = &sockets[s];

    if (!sock->conn) {
        SAL_DEBUG(("get_socket(%d): not active\n", s + SAL_SOCKET_OFFSET));
        set_errno(EBADF);
        return NULL;
    }

    return sock;
}

int sal_send(int s, const void *data, size_t size, int flags)
{
    return 0;
}

int sal_write(int s, const void *data, size_t size)
{
    s -= SAL_SOCKET_OFFSET;
    struct sal_event *event = tryget_event(s);
    if (event) {
        SAL_ARCH_DECL_PROTECT(lev);

        if (size != sizeof(uint64_t)) {
            return -1;
        }

        SAL_ARCH_PROTECT(lev);
        event->counts += *(uint64_t *)data;
        if (event->counts) {
            event->reads = event->counts;
            sal_sem_signal(event->psem);
        }
        SAL_ARCH_UNPROTECT(lev);
        return size;
    }
    return sal_send(s, data, size, 0);
}


void sal_deal_event(int s, enum netconn_evt evt)
{
    struct sal_select_cb *scb;
    int last_select_cb_ctr;
    struct sal_sock *sock = tryget_socket(s);
    if (!sock) {
        return;
    }
    SAL_ARCH_DECL_PROTECT(lev);
    SAL_ARCH_PROTECT(lev);
    /* Set event as required */
    switch (evt) {
        case NETCONN_EVT_RCVPLUS:
            sock->rcvevent++;
            break;
        case NETCONN_EVT_RCVMINUS:
            sock->rcvevent--;
            break;
        case NETCONN_EVT_SENDPLUS:
            sock->sendevent = 1;
            break;
        case NETCONN_EVT_SENDMINUS:
            sock->sendevent = 0;
            break;
        case NETCONN_EVT_ERROR:
            sock->errevent = 1;
            break;
        default:
            SAL_ASSERT("unknown event", 0);
            break;
    }

    if (sock->select_waiting == 0) {
        /* noone is waiting for this socket, no need to check select_cb_list */
        SAL_ARCH_UNPROTECT(lev);
        return;
    }

    /* Now decide if anyone is waiting for this socket */
    /* NOTE: This code goes through the select_cb_list list multiple times
       ONLY IF a select was actually waiting. We go through the list the number
       of waiting select calls + 1. This list is expected to be small. */

    /* At this point, SAL_ARCH is still protected! */
again:
    for (scb = select_cb_list; scb != NULL; scb = scb->next) {
        /* remember the state of select_cb_list to detect changes */
        last_select_cb_ctr = select_cb_ctr;
        if (scb->sem_signalled == 0) {
            /* semaphore not signalled yet */
            int do_signal = 0;
            /* Test this select call for our socket */
            if (sock->rcvevent > 0) {
                if (scb->readset && FD_ISSET(s, scb->readset)) {
                    do_signal = 1;
                }
            }
            if (sock->sendevent != 0) {
                if (!do_signal && scb->writeset && FD_ISSET(s, scb->writeset)) {
                    do_signal = 1;
                }
            }
            if (sock->errevent != 0) {
                if (!do_signal && scb->exceptset && FD_ISSET(s, scb->exceptset)) {
                    do_signal = 1;
                }
            }
            if (do_signal) {
                scb->sem_signalled = 1;
                /* Don't call SAL_ARCH_UNPROTECT() before signaling the semaphore, as this might
                   lead to the select thread taking itself off the list, invalidating the semaphore. */
                sal_sem_signal(SELECT_SEM_PTR(scb->sem));
            }
        }
        /* unlock interrupts with each step */
        SAL_ARCH_UNPROTECT(lev);
        /* this makes sure interrupt protection time is short */
        SAL_ARCH_PROTECT(lev);
        if (last_select_cb_ctr != select_cb_ctr) {
            /* someone has changed select_cb_list, restart at the beginning */
            goto again;
        }
    }
    SAL_ARCH_UNPROTECT(lev);

}



/**
 * Allocate a new socket for a given netconn.
 *
 * @param newconn the netconn for which to allocate a socket
 * @param accepted 1 if socket has been created by accept(),
 *                 0 if socket has been created by socket()
 * @return the index of the new socket; -1 on error
 */
static int alloc_socket(at_conn_t *newconn, int accepted)
{
    int i;
    SAL_ARCH_DECL_PROTECT(lev);

    /* allocate a new socket identifier */
    for (i = 0; i < NUM_SOCKETS; ++i) {
        /* Protect socket array */
        SAL_ARCH_PROTECT(lev);
        if (!sockets[i].conn) {
            sockets[i].conn       = newconn;
            /* The socket is not yet known to anyone, so no need to protect
               after having marked it as used. */
            SAL_ARCH_UNPROTECT(lev);
            sockets[i].lastdata   = NULL;
            sockets[i].lastoffset = 0;
            sockets[i].rcvevent   = 0;
            /* TCP sendbuf is empty, but the socket is not yet writable until connected
             * (unless it has been created by accept()). */
            //发送事件暂不支持
            // sockets[i].sendevent  = (NETCONNTYPE_GROUP(newconn->type) == NETCONN_TCP ? (accepted != 0) : 1);
            sockets[i].errevent   = 0;
            sockets[i].err        = 0;
            sockets[i].select_waiting = 0;
            return i + SAL_SOCKET_OFFSET;
        }
        SAL_ARCH_UNPROTECT(lev);
    }
    return -1;
}

at_conn_t* netconn_new(CONN_TYPE t)
{
  at_conn_t *conn;

  conn = (at_conn_t *)malloc(sizeof(at_conn_t));

  if (conn == NULL) {
    return NULL;
  }

  memset(conn,0,sizeof(at_conn_t));
   conn->type=t;
  return conn;
}

err_t netconn_delete(at_conn_t *conn)
{
  struct sal_sock *sock;
  if (conn == NULL) {
    return ERR_OK;
  }
  int s =conn->fd;
  sock=get_socket(s);
  if(sock){
     sock->conn=NULL; 
  }
  free(conn);
  conn=NULL;

  return ERR_OK;
}

int sal_socket(int domain, int type, int protocol)
{
  at_conn_t *conn;
  int i;

// #if !SAL_IPV6
//   SAL_UNUSED_ARG(domain); /* @todo: check this */
// #endif /* SAL_IPV6 */

  /* create a netconn */
  switch (type) {
  case SOCK_RAW://暂不支持
    set_errno(EINVAL);
    return -1;
    break;
  case SOCK_DGRAM:
    conn =netconn_new(UDP_UNICAST);
    break;
  case SOCK_STREAM:
    conn =netconn_new(TCP_CLIENT);
    break;
  default:
    set_errno(EINVAL);
    return -1;
  }

  if (!conn) {
    set_errno(ENOBUFS);
    return -1;
  }

  i = alloc_socket(conn, 0);

  if (i == -1) {
    netconn_delete(conn);
    set_errno(ENFILE);
    return -1;
  }
  conn->fd = i;
  SAL_DEBUG(("sal_socket new fd %d\n", i));
  set_errno(0);
  return i;
}

/* Call this during the init process. */
int sal_init()
{
    LOGI("sal", "Initializing SAL ...");
    sal_op.register_netconn_evt_cb(&sal_deal_event);
    return sal_op.init(); /* Low level init. */
}

#define SWAPS(s) ((((s) & 0xff) << 8) | (((s) & 0xff00) >> 8))

static void ip4_sockaddr_to_ipstr_port(const struct sockadd *name,
                                       char *ip, uint32_t *port)
{
    struct sockadd_in *saddr;
    uint32_t ip_n;

    if (!name || !ip || !port) return;

    saddr = (struct sockaddr_in *)name;
    memset(ip, 0, 16);
    *port = 0;

    /* Convert network order ip_addr to ip str (dot number fomrat) */
    ip_n = saddr->sin_addr;
    snprintf(ip, 15, "%d.%d.%d.%d", ip_n[0], ip_n[1], ip_n[2], ip_n[3]);

    /* Netwwork order port_t to host order port number */
    if (BYTE_ORDER == LITTLE_ENDIAN)
        *port = SWAPS(saddr->sin_port);
    else
        *port = saddr->sin_port;

    SAL_DEBUG("Socket address coverted to %s:%d", ip, port);
}

int sal_connect(int s, const struct sockaddr *name, socklen_t namelen)
{
    struct sal_sock *sock;
    err_t err;
    at_conn_t *c;
    char ip_str[16] = {0};
    uint32_t port;

    sock = get_socket(s);
    if (!sock) {
        SAL_ERROR("get_socket failed.");
        sock_set_errno(sock, err_to_errno(ERR_VAL));
        return -1;
    }

    /* Only IPv4 is supported. */
    if (name->sa_family != AF_INET) {
        SAL_ERROR("Not supported (only IPv4 for now)!");
        sock_set_errno(sock, err_to_errno(ERR_VAL));
        return -1;
    }

    /* Check size */
    if (!IS_SOCK_ADDR_LEN_VALID(namelen)) {
        SAL_ERROR("sal_connect: invalid address");
        sock_set_errno(sock, err_to_errno(ERR_ARG));
        return -1;
    }

    /* Do connection */
    c = sock->conn;
    ip4_sockaddr_to_ipstr_port(name, ip, &port);
    c->addr = ip;
    c->r_port = port;
    if ((err = sal_op.start(c)) != 0) {
        SAL_ERROR("sal_start failed.");
        sock_set_errno(sock, err_to_errno(ERR_IF));
        return -1;
    }

    /* Update conn state here <TODO> */

    if (err != ERR_OK) {
        SAL_ERROR("sal_connect(%d) failed, err=%d\n", s, err);
        sock_set_errno(sock, err_to_errno(err));
        return -1;
    }

    SAL_DEBUG("sal_connect(%d) succeeded\n", s);
    sock_set_errno(sock, 0);
    return 0;
}

/**
 * Free a socket. The socket's netconn must have been
 * delete before!
 *
 * @param sock the socket to free
 * @param is_tcp != 0 for TCP sockets, used to free lastdata
 */
static void free_socket(struct sal_sock *sock)
{
  sock->lastdata   = NULL;
  sock->lastoffset = 0;
  sock->err        = 0;

  /* Protect socket array */
  SAL_ARCH_SET(sock->conn, NULL);
  /* don't use 'sock' after this line, as another task might have allocated it */
}

int sal_close(int s)
{
  struct sal_sock *sock;
  struct sal_event *event;
  err_t err;

  SAL_DEBUG("sal_close(%d)\n");

  event = tryget_event(s);
  if (event) {
    event->used = 0;
    return 0;
  }

  sock = get_socket(s);
  if (!sock) {
    return -1;
  }

  if (sal_op.close(s, -1) != 0) {
    SAL_ERROR("sal_op.close failed.");
    sock_set_errno(sock, err_to_errno(ERR_IF));
    return -1;
  }

  err = netconn_delete(sock->conn);
  if (err != ERR_OK) {
    SAL_ERROR("netconn_delete failed in %s.", __func__);
    sock_set_errno(sock, err_to_errno(err));
    return -1;
  }

  free_socket(sock);
  set_errno(0);
  return 0;
}

#ifndef _SAL_PCB_H_
#define _SAL_PCB_H_


#include <aos/aos.h>
#include <err.h>

#ifdef __cplusplus
extern "C" {
#endif

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
#if SAL_NETIF_HWADDRHINT
#define IP_PCB_ADDRHINT ;u8_t addr_hint
#else
#define IP_PCB_ADDRHINT
#endif 

/** Main packet buffer struct */
struct pbuf {
    /** next pbuf in singly linked pbuf chain */
    struct pbuf *next;

    /** pointer to the actual data in the buffer */
    void *payload;

    /**
     * total length of this buffer and all next buffers in chain
     * belonging to the same packet.
     *
     * For non-queue packet chains this is the invariant:
     * p->tot_len == p->len + (p->next? p->next->tot_len: 0)
     */
    u16_t tot_len;

    /** length of this buffer */
    u16_t len;

    /** pbuf_type as u8_t instead of enum to save space */
    u8_t /*pbuf_type*/ type;

    /** misc flags */
    u8_t flags;

#if SAL_XR_EXT_MBUF_SUPPORT
    /** new flags for mbuf */
    u8_t mb_flags;

    /** decrease its size to u8_t */
    u8_t ref;
#else /* SAL_XR_EXT_MBUF_SUPPORT */
    /**
     * the reference count always equals the number of pointers
     * that refer to this pbuf. This can be pointers from an application,
     * the stack itself, or pbuf->next pointers from a chain.
     */
    u16_t ref;
#endif
};
/** This is the aligned version of ip6_addr_t,
    used as local variable, on the stack, etc. */
typedef struct ip6_addr {
  u32_t addr[4];
}ip6_addr_t;

/** This is the aligned version of ip4_addr_t,
   used as local variable, on the stack, etc. */
typedef struct ip4_addr {
  u32_t addr;
}ip4_addr_t;
/**
 * @ingroup ipaddr
 * A union struct for both IP version's addresses.
 * ATTENTION: watch out for its size when adding IPv6 address scope!
 */
typedef struct _ip_addr {
  union {
    ip6_addr_t ip6;
    ip4_addr_t ip4;
  } u_addr;
  /** @ref sal_ip_addr_type */
  u8_t type;
} ip_addr_t;
/** This is the common part of all PCB types. It needs to be at the
   beginning of a PCB type definition. It is located here so that
   changes to this common part are made in one location instead of
   having to change all PCB structs. */
#define IP_PCB \
  /* ip addresses in network byte order */ \
  ip_addr_t local_ip; \
  ip_addr_t remote_ip; \
   /* Socket options */  \
  u8_t so_options;      \
   /* Type Of Service */ \
  u8_t tos;              \
  /* Time To Live */     \
  u8_t ttl               \
  /* link layer address resolution hint */ \
  IP_PCB_ADDRHINT

struct ip_pcb {
/* Common members of all PCB types */
  IP_PCB;
};

struct tcp_pcb;
typedef void (*tcp_recv_fn)(void *arg, struct tcp_pcb *pcb, struct pbuf *p,
    const ip_addr_t *addr, u16_t port);
/**
 * members common to struct tcp_pcb and struct tcp_listen_pcb
 */
#define TCP_PCB_COMMON(type) \
  type *next; /* for the linked list */ \
  void *callback_arg; \
  u8_t prio; \
  /* ports are in host byte order */ \
  u16_t local_port
  
/** the TCP protocol control block */
struct tcp_pcb {
/** common PCB members */
  IP_PCB;
/** protocol specific PCB members */
  TCP_PCB_COMMON(struct tcp_pcb);

  /* ports are in host byte order */
  u16_t remote_port;
  /** receive callback function */
  tcp_recv_fn recv;
   /* user-supplied argument for the recv callback */
   void *recv_arg;
};

struct udp_pcb;

/** Function prototype for udp pcb receive callback functions
 * addr and port are in same byte order as in the pcb
 * The callback is responsible for freeing the pbuf
 * if it's not used any more.
 *
 * ATTENTION: Be aware that 'addr' might point into the pbuf 'p' so freeing this pbuf
 *            can make 'addr' invalid, too.
 *
 * @param arg user supplied argument (udp_pcb.recv_arg)
 * @param pcb the udp_pcb which received data
 * @param p the packet buffer that was received
 * @param addr the remote IP address from which the packet was received
 * @param port the remote port from which the packet was received
 */
typedef void (*udp_recv_fn)(void *arg, struct udp_pcb *pcb, struct pbuf *p,
    const ip_addr_t *addr, u16_t port);

/** the UDP protocol control block */
struct udp_pcb {
/** Common members of all PCB types */
  IP_PCB;

/* Protocol specific PCB members */

  struct udp_pcb *next;

  u8_t flags;
  /** ports are in host byte order */
  u16_t local_port, remote_port;

#if SAL_MULTICAST_TX_OPTIONS
  /** outgoing network interface for multicast packets */
  ip_addr_t multicast_ip;
  /** TTL for outgoing multicast packets */
  u8_t mcast_ttl;
#endif /* SAL_MULTICAST_TX_OPTIONS */

#if SAL_UDPLITE
  /** used for UDP_LITE only */
  u16_t chksum_len_rx, chksum_len_tx;
#endif /* SAL_UDPLITE */

  /** receive callback function */
  udp_recv_fn recv;
  /** user-supplied argument for the recv callback */
  void *recv_arg;
};

struct raw_pcb;

/** Function prototype for raw pcb receive callback functions.
 * @param arg user supplied argument (raw_pcb.recv_arg)
 * @param pcb the raw_pcb which received data
 * @param p the packet buffer that was received
 * @param addr the remote IP address from which the packet was received
 * @return 1 if the packet was 'eaten' (aka. deleted),
 *         0 if the packet lives on
 * If returning 1, the callback is responsible for freeing the pbuf
 * if it's not used any more.
 */
typedef u8_t (*raw_recv_fn)(void *arg, struct raw_pcb *pcb, struct pbuf *p,
    const ip_addr_t *addr);

/** the RAW protocol control block */
struct raw_pcb {
  /* Common members of all PCB types */
  IP_PCB;

  struct raw_pcb *next;

  u8_t protocol;

  /** receive callback function */
  raw_recv_fn recv;
  /* user-supplied argument for the recv callback */
  void *recv_arg;
#if SAL_IPV6
  /* fields for handling checksum computations as per RFC3542. */
  u16_t chksum_offset;
  u8_t  chksum_reqd;
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* __AOS_EVENTFD_H__ */


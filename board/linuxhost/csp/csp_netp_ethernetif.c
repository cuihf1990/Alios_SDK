/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */


#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "lwip/etharp.h"
#include "netif/ppp/pppoe.h"
#include "lwip/netif.h"

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>


#include "lwip/debug.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

/* Forward declarations. */
static void  ethernetif_input(struct netif *netif);

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
    struct tunif *tunif;
    char buf[sizeof(IFCONFIG_CALL) + 50];

    tunif = (struct tunif *)netif->state;

    /* Obtain MAC address from network interface. */

    /* Do whatever else is needed to initialize interface. */

    tunif->fd = open("/dev/tun0", O_RDWR);
    LWIP_DEBUGF(TUNIF_DEBUG, ("tunif_init: fd %d\n", tunif->fd));
    if (tunif->fd == -1) {
    perror("tunif_init");
    exit(1);
    }
    sprintf(buf, IFCONFIG_CALL,
           ip4_addr1(netif_ip4_gw(netif)),
           ip4_addr2(netif_ip4_gw(netif)),
           ip4_addr3(netif_ip4_gw(netif)),
           ip4_addr4(netif_ip4_gw(netif)),
           ip4_addr1(netif_ip4_addr(netif)),
           ip4_addr2(netif_ip4_addr(netif)),
           ip4_addr3(netif_ip4_addr(netif)),
           ip4_addr4(netif_ip4_addr(netif)));

    LWIP_DEBUGF(TUNIF_DEBUG, ("tunif_init: system(\"%s\");\n", buf));
    system(buf);
    sys_thread_new("tunif_thread", tunif_thread, netif, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

    //printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
    return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
    return 0;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static void ethernetif_input(struct netif *netif)
{
    struct ethernetif *ethernetif;
    struct eth_hdr *ethhdr;
    struct pbuf *p;

    ethernetif = netif->state;

    /* move received packet into a new pbuf */
    p = low_level_input(netif);
    /* if no packet could be read, silently ignore this */
    if (p != NULL) {
        /* pass all packets to ethernet_input, which decides what packets it supports */
        if (netif->input(p, netif) != ERR_OK) {
          LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
          pbuf_free(p);
          p = NULL;
        }
    }
}

static void ip_output(struct netif *netif, struct pbuf *q, const ip4_addr_t *ipaddr)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
}


/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif)
{
    struct ethernetif *ethernetif;
    LWIP_ASSERT("netif != NULL", (netif != NULL));
    printf("%s %d called.\n", __FUNCTION__, __LINE__);
    ethernetif = mem_malloc(sizeof(struct ethernetif));
    printf("%s %d called.\n", __FUNCTION__, __LINE__);
    if (ethernetif == NULL) {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
        printf("Error........'n'");
        return ERR_MEM;
    }
    printf("%s %d called.\n", __FUNCTION__, __LINE__);
#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */
    printf("%s %d called.\n", __FUNCTION__, __LINE__);
    /*
    * Initialize the snmp variables and counters inside the struct netif.
    * The last argument should be replaced with your link speed, in units
    * of bits per second.
    */
    MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);
        printf("%s %d called.\n", __FUNCTION__, __LINE__);
    netif->state = ethernetif;
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    /* We directly use etharp_output() here to save a function call.
    * You can instead declare your own function an call etharp_output()
    * from it if you have to do some checks before sending (e.g. if link
    * is available...) */
    netif->output = ip_output;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
    netif->linkoutput = low_level_output;
    printf("%s %d called.\n", __FUNCTION__, __LINE__);
    ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

    /* initialize the hardware */
    low_level_init(netif);

    return ERR_OK;
}

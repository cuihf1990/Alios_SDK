#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yos/kernel.h>
#include <umesh_hal.h>
#include <umesh_80211.h>

#undef USE_ACTION_FRAME
#undef MDEBUG
#define USE_MULTICAST_FRAME

static inline uint16_t calc_seqctrl(unsigned char *pkt)
{
    return (pkt[23] << 4) | (pkt[22] >> 4);
}

static void p_addr(unsigned char *pkt)
{
	printf(":%02x %02x %02x %02x %02x %02x", pkt[0] , pkt[1] , pkt[2] , pkt[3] , pkt[4] , pkt[5]);
}

static inline void dump_packet(unsigned char *pkt, int count)
{
    int seqno = calc_seqctrl(pkt);
	printf("%s(%d) %02x %02x %02x", __func__, count, pkt[0], pkt[1], seqno);
	p_addr(pkt+OFF_DST);
	p_addr(pkt+OFF_SRC);
	p_addr(pkt+OFF_BSS);
	printf("\n");
}

int umesh_80211_make_frame(ur_mesh_hal_module_t *module, frame_t *frame, mac_address_t *dest, void *fpkt)
{
    static unsigned long nb_pkt_sent;
    umesh_extnetid_t extnetid;
    const mac_address_t *mymac;
    uint8_t *pkt = fpkt;

    mymac = hal_umesh_get_mac_address(module);
    hal_umesh_get_extnetid(module, &extnetid);

    bzero(pkt, MESH_DATA_OFF);
#ifdef USE_ACTION_FRAME
    pkt[0] = 0xd0;
#else
    pkt[0] = 0x08;
#endif
    memcpy(pkt + OFF_DST, dest->addr, 6);
#ifdef USE_MULTICAST_FRAME
    pkt[OFF_DST] |= 1;
#endif
    memcpy(pkt + OFF_SRC, mymac->addr, 6);
    memcpy(pkt + OFF_BSS, extnetid.netid, 6);

    /* sequence control */
    pkt[22] = (nb_pkt_sent & 0x0000000F) << 4;
    pkt[23] = (nb_pkt_sent & 0x00000FF0) >> 4;
    nb_pkt_sent++;

#ifdef USE_ACTION_FRAME
    pkt[24] = 127;
#endif

    memcpy(pkt + MESH_DATA_OFF, frame->data, frame->len);

    return 0;
}

typedef struct mac_entry_s {
    uint64_t mactime;
    uint16_t last_seq;
    uint8_t  macaddr[6];
} mac_entry_t;

#define ENT_NUM 32
static mac_entry_t entries[ENT_NUM];
static mac_entry_t *find_mac_entry(uint8_t  macaddr[6])
{
    mac_entry_t *ment, *yent = NULL;
    uint64_t youngest = -1ULL;
    int i;

    for (i=0;i<ENT_NUM;i++) {
        ment = entries + i;
        if (memcmp(ment->macaddr, macaddr, 6) == 0)
            return ment;

        if (ment->mactime > youngest)
            continue;

        youngest = ment->mactime;
        yent = ment;
    }

    bzero(yent, sizeof(*yent));
    memcpy(yent->macaddr, macaddr, 6);
    return yent;
}

bool umesh_80211_filter_frame(ur_mesh_hal_module_t *module, uint8_t *pkt, int count)
{
    const uint8_t bcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    umesh_extnetid_t extnetid;
    const mac_address_t *mymac;
    uint16_t seqno = calc_seqctrl(pkt) << 4;
    mac_entry_t *ent;
    uint64_t mactime = yos_now_ms();

    mymac = hal_umesh_get_mac_address(module);
    hal_umesh_get_extnetid(module, &extnetid);

    if (memcmp(pkt+OFF_BSS, extnetid.netid, 6))
        return 1;

    if (memcmp(pkt+OFF_SRC, mymac->addr, 6) == 0)
        return 1;

    if (memcmp(pkt+OFF_DST, bcast, 6) == 0)
        goto next;

#ifdef USE_MULTICAST_FRAME
    pkt[OFF_DST] &= ~1;
#endif
    if (memcmp(pkt+OFF_DST, mymac->addr, 6))
        return 1;

next:
    ent = find_mac_entry(pkt+OFF_SRC);
    /* if longer than 100ms */
    if (mactime - ent->mactime > 100) {
        goto no_filter;
    }

    if ((int16_t)(seqno - ent->last_seq) <= 0) {
#ifdef MDEBUG
        dump_packet(pkt, 32);
        printf("duplicate seqno %02x %02x %02x %02x %02x %02x\n",
               ent->last_seq, seqno, pkt[0], pkt[1], pkt[22], pkt[23]);
#endif
        return 1;
    }

no_filter:
    ent->mactime = mactime;
    ent->last_seq = seqno;
	return 0;
}

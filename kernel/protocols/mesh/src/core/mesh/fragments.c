/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "umesh_utils.h"
#include "core/fragments.h"
#include "core/network_data.h"
#include "ip/ip.h"

typedef struct lowpan_reass_s {
    struct lowpan_reass_s *next;
    message_t             *message;
    uint16_t              sender_addr;
    uint16_t              datagram_size;
    uint16_t              datagram_tag;
    uint8_t               timer;
} lowpan_reass_t;

static lowpan_reass_t *g_reass_list = NULL;
static ur_timer_t g_reass_timer;

static ur_error_t dequeue_list_element(lowpan_reass_t *lrh)
{
    lowpan_reass_t *lrh_temp;

    if (g_reass_list == lrh) {
        g_reass_list = g_reass_list->next;
    } else {
        lrh_temp = g_reass_list;

        while (lrh_temp != NULL) {
            if (lrh_temp->next == lrh) {
                lrh_temp->next = lrh->next;
                break;
            }

            lrh_temp = lrh_temp->next;
        }
    }

    return UR_ERROR_NONE;
}

ur_error_t lp_reassemble(message_t *p, message_t **reass_p)
{
    frag_header_t *frag_header;
    frag_header_t frag_header_content;
    uint16_t datagram_size, datagram_tag, datagram_offset;
    lowpan_reass_t *lrh, *lrh_temp;
    message_info_t *info;

    if (p == NULL || reass_p == NULL) {
        return UR_ERROR_FAIL;
    }

    info = p->info;
    *reass_p = NULL;

    message_copy_to(p, 0, (uint8_t *)&frag_header_content, sizeof(frag_header_t));
    frag_header = &frag_header_content;
    *((uint16_t *)frag_header) = ntohs(*(uint16_t *)frag_header);
    datagram_size = frag_header->size;
    datagram_tag = ntohs(frag_header->tag);
    /* Check dispatch. */
    if (frag_header->dispatch == FRAG_1_DISPATCH) {
        /* check for duplicate */
        lrh = g_reass_list;

        while (lrh != NULL) {
            if (lrh->sender_addr == info->src.addr.short_addr) {
                /* address match with packet in reassembly. */
                if ((datagram_tag == lrh->datagram_tag) &&
                    (datagram_size == lrh->datagram_size)) {
                    /* duplicate fragment. */
                    MESH_LOG_DEBUG("lowpan6: received duplicated FRAG_1 from"
                                   " %04hx (tag=%u tot_len=%u), drop it",
                                   info->src.addr.short_addr, datagram_tag, datagram_size);
                    return UR_ERROR_FAIL;
                } else {
                    /* We are receiving the start of a new datagram. Discard old incomplete one. */
                    lrh_temp = lrh->next;
                    dequeue_list_element(lrh);
                    message_free(lrh->message);
                    ur_mem_free(lrh, sizeof(lowpan_reass_t));

                    /* Check next datagram in queue. */
                    lrh = lrh_temp;
                }
            } else {
                /* Check next datagram in queue. */
                lrh = lrh->next;
            }
        }

        message_set_payload_offset(p, - 4); /* hide FRAG_1 header */

        MESH_LOG_DEBUG("lowpan6: received new FRAG_1 from %04hx, tag=%u tot_len=%u len=%u offset=0",
                       info->src.addr.short_addr, datagram_tag, datagram_size, message_get_msglen(p));

        lrh = (lowpan_reass_t *) ur_mem_alloc(sizeof(lowpan_reass_t));
        if (lrh == NULL) {
            /* out of memory, drop */
            return UR_ERROR_FAIL;
        }

        lrh->sender_addr = info->src.addr.short_addr;
        lrh->datagram_size = datagram_size;
        lrh->datagram_tag = datagram_tag;
        lrh->message = p;
        lrh->next = g_reass_list;
        lrh->timer = 5;
        g_reass_list = lrh;

        return UR_ERROR_NONE;
    } else if (frag_header->dispatch == FRAG_N_DISPATCH) {
        /* FRAGN dispatch, find packet being reassembled. */
        datagram_offset = ((uint16_t)frag_header->offset) << 3;
        message_set_payload_offset(p, -5);

        for (lrh = g_reass_list; lrh != NULL; lrh = lrh->next) {
            if ((lrh->sender_addr ==  info->src.addr.short_addr) &&
                (lrh->datagram_tag == datagram_tag) &&
                (lrh->datagram_size == datagram_size)) {
                break;
            }
        }

        if (lrh == NULL) {
            /* rogue fragment */
            return UR_ERROR_FAIL;
        }

        if (message_get_msglen(lrh->message) > datagram_offset) {
            /* duplicate, ignore. */
            MESH_LOG_DEBUG("lowpan6: received duplicated FRAG_N from"
                           " %04hx, tag=%u len=%u offset=%u, drop it",
                           info->src.addr.short_addr, datagram_tag, message_get_msglen(p),
                           datagram_offset);
            return UR_ERROR_FAIL;
        } else if (message_get_msglen(lrh->message) < datagram_offset) {
            /* We have missed a fragment. Delete whole reassembly. */
            MESH_LOG_DEBUG("lowpan6: received disordered FRAG_N from %04hx,"
                           " tag=%u len=%u offset=%u, drop the whole fragment packets",
                           info->src.addr.short_addr, datagram_tag, message_get_msglen(p),
                           datagram_offset);
            dequeue_list_element(lrh);
            message_free(lrh->message);
            ur_mem_free(lrh, sizeof(lowpan_reass_t));
            return UR_ERROR_FAIL;
        }

        MESH_LOG_DEBUG("lowpan6: received FRAG_N from %04hx, tag=%u len=%u offset=%u",
                       info->src.addr.short_addr, datagram_tag, message_get_msglen(p),
                       datagram_offset);
        message_concatenate(lrh->message, p, false);
        p = NULL;

        /* is packet now complete?*/
        if (message_get_msglen(lrh->message) >= lrh->datagram_size) {
            /* dequeue from reass list. */
            dequeue_list_element(lrh);

            /* get message */
            *reass_p = message_alloc(message_get_msglen(lrh->message), LOWPAN6_2);
            message_copy(*reass_p, lrh->message);
            message_free(lrh->message);

            /* release helper */
            ur_mem_free(lrh, sizeof(lowpan_reass_t));
            return UR_ERROR_NONE;
        }
    } else {
        MESH_LOG_DEBUG("lowpan6: unrecognized FRAG packet, drop it");
        return UR_ERROR_FAIL;
    }

    return UR_ERROR_NONE;
}

void lp_handle_timer(void *args)
{
    lowpan_reass_t *lrh, *lrh_temp;

    g_reass_timer = ur_start_timer(REASSEMBLE_TICK_INTERVAL, lp_handle_timer, NULL);

    lrh = g_reass_list;
    while (lrh != NULL) {
        lrh_temp = lrh->next;

        if ((--lrh->timer) == 0) {
            MESH_LOG_DEBUG("lowpan6: fragment packts from %04hx (tag=%u tot_len=%u)"
                           " timeout, drop from reassemble queue",
                           lrh->sender_addr, lrh->datagram_tag, lrh->datagram_size);
            dequeue_list_element(lrh);
            message_free(lrh->message);
            ur_mem_free(lrh, sizeof(lowpan_reass_t));
        }

        lrh = lrh_temp;
    }
}

void lp_start(void)
{
    g_reass_timer = ur_start_timer(REASSEMBLE_TICK_INTERVAL, lp_handle_timer, NULL);
}

void lp_stop(void)
{
    lowpan_reass_t *lrh, *lrh_temp;

    ur_stop_timer(&g_reass_timer, NULL);

    lrh = g_reass_list;
    while (lrh != NULL) {
        lrh_temp = lrh->next;
        dequeue_list_element(lrh);
        message_free(lrh->message);
        ur_mem_free(lrh, sizeof(lowpan_reass_t));
        lrh = lrh_temp;
    }
}

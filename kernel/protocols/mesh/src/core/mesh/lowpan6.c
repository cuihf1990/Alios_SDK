/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "umesh_utils.h"
#include "core/lowpan6.h"
#include "core/network_data.h"
#include "ipv6/ip6.h"

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

static bool is_mcast_addr(ur_ip6_addr_t *ip6_addr)
{
    return (ip6_addr->m8[0] == 0xff);
}

static uint8_t ucast_addr_compress(ur_ip6_addr_t *ip6_addr, uint8_t *buffer,
                                   uint8_t *len)
{
    ur_ip6_prefix_t prefix;

    if (nd_get_ip6_prefix(&prefix) != UR_ERROR_NONE) {
        memcpy(buffer, &ip6_addr->m8[0], 16);
        *len = *len + 16;
        return UCAST_ADDR_128BIT;
    }

    if (memcmp(prefix.prefix.m8, ip6_addr->m8, 8) != 0) {
        memcpy(buffer, &ip6_addr->m8[0], 16);
        *len = *len + 16;
        return UCAST_ADDR_128BIT;
    }

    if (memcmp(&prefix.prefix.m8[8], &ip6_addr->m8[8], 6) == 0) {
        return UCAST_ADDR_ELIDED;
    }

    if (memcmp(&ip6_addr->m8[8], FIXED_IID, 6) == 0) {
        memcpy(buffer, &ip6_addr->m8[14], 2);
        *len = *len + 2;
        return UCAST_ADDR_16BIT;
    }

    memcpy(buffer, &ip6_addr->m8[8], 8);
    *len = *len + 8;
    return UCAST_ADDR_64BIT;
}

static uint8_t ucast_addr_decompress(uint8_t mode, ur_ip6_prefix_t *prefix,
                                     const uint8_t *iphc_buffer,
                                     ur_ip6_addr_t *ip6_addr, uint16_t shortid)
{
    switch (mode) {
        case UCAST_ADDR_ELIDED:
            memcpy(&ip6_addr->m8[0], prefix->prefix.m8, 14);
            ip6_addr->m8[14] = (shortid >> 8) & 0xff;
            ip6_addr->m8[15] = shortid & 0xff;
            return 0;
            break;
        case UCAST_ADDR_16BIT:
            memcpy(&ip6_addr->m8[0], prefix->prefix.m8, 8);
            memcpy(&ip6_addr->m8[8], FIXED_IID, 6);
            memcpy(&ip6_addr->m8[14], iphc_buffer, 2);
            return 2;
            break;
        case UCAST_ADDR_64BIT:
            memcpy(&ip6_addr->m8[0], prefix->prefix.m8, 8);
            memcpy(&ip6_addr->m8[8], iphc_buffer, 8);
            return 8;
            break;
        case UCAST_ADDR_128BIT:
            memcpy(&ip6_addr->m8[0], iphc_buffer, 16);
            return 16;
            break;
        default:
            break;
    }
    return 0;
}

static uint8_t mcast_addr_compress(ur_ip6_addr_t *ip6_addr,
                                   uint8_t *buffer, uint8_t *len)
{
    ur_ip6_addr_t tmp;
    memset(tmp.m8, 0x00, sizeof(tmp.m8));

    tmp.m8[0] = 0xff;
    tmp.m8[1] = 0x02;

    if (memcmp(&tmp.m8[0], ip6_addr->m8, 15) == 0) {
        buffer[0] = ip6_addr->m8[15];
        *len = *len + 1;
        return MCAST_ADDR_8BIT;
    }

    if (memcmp(&tmp.m8[2], &ip6_addr->m8[2], 11) == 0) {
        buffer[0] = ip6_addr->m8[1];
        memcpy(&buffer[1], &ip6_addr->m8[13], 3);
        *len = *len + 4;
        return MCAST_ADDR_32BIT;
    }

    if (memcmp(&tmp.m8[2], &ip6_addr->m8[2], 9) == 0) {
        buffer[0] = ip6_addr->m8[1];
        memcpy(&buffer[1], &ip6_addr->m8[11], 5);
        *len = *len + 6;
        return MCAST_ADDR_48BIT;
    }

    memcpy(buffer, &ip6_addr->m8[0], 16);
    *len = *len + 16;
    return MCAST_ADDR_128BIT;
}

static uint8_t mcast_addr_decompress(uint8_t mode, const uint8_t *iphc_buffer,
                                     ur_ip6_addr_t *ip6_addr)
{
    memset(ip6_addr->m8, 0x00, 16);
    ip6_addr->m8[0] = 0xff;
    switch (mode) {
        case MCAST_ADDR_8BIT:
            ip6_addr->m8[1] = 0x02;
            memcpy(&ip6_addr->m8[15], &iphc_buffer[0], 1);
            return 1;
            break;
        case MCAST_ADDR_32BIT:
            ip6_addr->m8[1] = iphc_buffer[0];
            memcpy(&ip6_addr->m8[13], &iphc_buffer[1], 3);
            return 4;
            break;
        case MCAST_ADDR_48BIT:
            ip6_addr->m8[1] = iphc_buffer[0];
            memcpy(&ip6_addr->m8[11], &iphc_buffer[1], 5);
            return 6;
            break;
        case MCAST_ADDR_128BIT:
            memcpy(&ip6_addr->m8[0], &iphc_buffer[0], 16);
            return 16;
            break;
    }
    return 0;
}

static uint8_t ipv6_header_compress(ur_ip6_header_t *ip6_header,
                                    uint8_t *buffer)
{
    uint8_t         iphc_len = 0;
    iphc_header_t   *iphc_header;

    iphc_len = 2;

    iphc_header = (iphc_header_t *)buffer;
    iphc_header->DP = IPHC_DISPATCH;

    /* Determine TF field: Traffic Class, Flow Label */
    uint32_t v_tc_fl = ntohl(ip6_header->v_tc_fl);
    if ((v_tc_fl & FLOW_LABEL_MASK) == 0) {
        if ((v_tc_fl & TRAFFIC_CLASS_MASK) == 0) {
            iphc_header->TF = TC_FL_BOTH_ELIDED;
        } else {
            iphc_header->TF = TC_APENDED_FL_ELIDED;
            buffer[iphc_len++] = (v_tc_fl) >> 20;
        }
    } else {
        if ((v_tc_fl & TC_DSCP_MASK) == 0) {
            iphc_header->TF = DCSP_ELEDED_ECN_FL_APPENDED;
            buffer[iphc_len] = (v_tc_fl >> 20) & 0xc0;
            buffer[iphc_len++] |= (v_tc_fl >> 16) & 0x0f;
            buffer[iphc_len++] = (v_tc_fl >> 8) & 0xff;
            buffer[iphc_len++] = v_tc_fl & 0xff;
        } else {
            iphc_header->TF = TC_FL_BOTH_APEENDED;
            buffer[iphc_len++] = (v_tc_fl >> 20) & 0xff;
            buffer[iphc_len++] = (v_tc_fl >> 16) & 0x0f;
            buffer[iphc_len++] = (v_tc_fl >> 8) & 0xff;
            buffer[iphc_len++] = v_tc_fl & 0xff;
        }
    }

    /* Compress NH? Only if UDP for now. */
    if (ip6_header->next_header == UR_IPPROTO_UDP) {
        iphc_header->NH = NEXT_HEADER_ELIDED;
    } else {
        iphc_header->NH = NEXT_HEADER_APPENDED;
        buffer[iphc_len++] = ip6_header->next_header;
    }

    /* Compress hop limit? */
    if (ip6_header->hop_lim == 255) {
        iphc_header->HLIM = HOP_LIM_255;
    } else if (ip6_header->hop_lim == 64) {
        iphc_header->HLIM = HOP_LIM_64;
    } else if (ip6_header->hop_lim == 1) {
        iphc_header->HLIM = HOP_LIM_1;
    } else {
        iphc_header->HLIM = HOP_LIM_APPENDED;
        buffer[iphc_len++] = ip6_header->hop_lim;
    }

    /* stateless address compressing: CID=0 SAC=0 DAC=0 M=0*/
    /* TODO: support context based compression */
    iphc_header->CID = STATELESS_COMPRESS;
    iphc_header->SAC = STATELESS_COMPRESS;
    iphc_header->DAC = STATELESS_COMPRESS;

    /* Compress source address */
    iphc_header->SAM = ucast_addr_compress(&ip6_header->src, &buffer[iphc_len],
                                           &iphc_len);

    /* Compress destination address */
    if (is_mcast_addr(&ip6_header->dest) == true) {
        iphc_header->M = MULTICAST_DESTINATION;
        iphc_header->DAM = mcast_addr_compress(&ip6_header->dest, &buffer[iphc_len],
                                               &iphc_len);
    } else {
        iphc_header->M = UNICAST_DESTINATION;
        iphc_header->DAM = ucast_addr_compress(&ip6_header->dest, &buffer[iphc_len],
                                               &iphc_len);
    }

    *(uint16_t *)iphc_header = htons(*(uint16_t *)iphc_header);
    return iphc_len;
}

uint8_t udp_header_compress(ur_udp_header_t *udp_header, uint8_t *buffer)
{
    int nhc_len = 1;
    nhc_header_t *nhc_header = (nhc_header_t *)buffer;

    //set UPD LOWPAN_NHC header
    nhc_header->DP = NHC_UDP_DISPATCH;

    /* TODO: support optional checksum compression */
    nhc_header->C = 0b0;

    uint16_t src_port = ntohs(udp_header->src_port);
    uint16_t dst_port = ntohs(udp_header->dst_port);
    uint16_t chksum = ntohs(udp_header->chksum);
    /* port compression */
    if ((src_port & 0xfff0) == 0xf0b0 &&
        (dst_port & 0xfff0) == 0xf0b0) {
        nhc_header->P = BOTH_PORT_COMPRESSED;
        buffer[nhc_len] = (src_port << 4) & 0xf0;
        buffer[nhc_len++] |= dst_port & 0x0f;
    } else if ((src_port & 0xff00) == 0xf000) {
        nhc_header->P = SRC_PORT_COMPRESSED;
        buffer[nhc_len++] = src_port;
        buffer[nhc_len++] = dst_port >> 8;
        buffer[nhc_len++] = dst_port;
    } else if ((dst_port & 0xff00) == 0xf000) {
        nhc_header->P = DST_PORT_COMPRESSED;
        buffer[nhc_len++] = src_port >> 8;
        buffer[nhc_len++] = src_port;
        buffer[nhc_len++] = dst_port;
    } else {
        nhc_header->P = NO_PORT_COMPRESSED;
        buffer[nhc_len++] = src_port >> 8;
        buffer[nhc_len++] = src_port;
        buffer[nhc_len++] = dst_port >> 8;
        buffer[nhc_len++] = dst_port;
    }

    buffer[nhc_len++] = chksum >> 8;
    buffer[nhc_len++] = chksum;

    return nhc_len;
}

ur_error_t lp_header_compress(const uint8_t *header, uint8_t *buffer,
                              uint8_t *ip_header_len, uint8_t *hc_header_len)
{
    uint8_t iphc_len, nhc_len;
    ur_ip6_header_t *ip6_header = (ur_ip6_header_t *)header;

    uint32_t v_tc_fl = ntohl(ip6_header->v_tc_fl);
    if ((v_tc_fl & VERSION_MASK) != IP_VERSION_6) {
        return UR_ERROR_FAIL;
    }

    /* Compress IPv6 header */
    iphc_len = ipv6_header_compress(ip6_header, buffer);
    *ip_header_len = UR_IP6_HLEN;
    *hc_header_len = iphc_len;
    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_6LOWPAN,
           "lowpan6: compressed 40 bytes IPv6 header to"
           " %u bytes 6LowPAN IPHC header\r\n", iphc_len);

    /* Compress UDP header? */
    if (ip6_header->next_header == UR_IPPROTO_UDP) {
        ur_udp_header_t *udp_header;
        udp_header = (ur_udp_header_t *)(header + UR_IP6_HLEN);
        nhc_len = udp_header_compress(udp_header, buffer + iphc_len);
        ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_6LOWPAN,
               "lowpan6: compressed 8 bytes UDP header to"
               " %d bytes 6LowPAN NHC header\r\n", nhc_len);
        *ip_header_len = UR_IP6_HLEN + UR_UDP_HLEN;
        *hc_header_len = *hc_header_len + nhc_len;
    }

    return UR_ERROR_NONE;
}

static ur_error_t ipv6_header_decompress(message_info_t *info, const uint8_t *iphc_data,
                                         uint8_t *buffer, uint8_t *hc_len)
{
    ur_ip6_header_t *ip6_header = (ur_ip6_header_t *)buffer;
    uint8_t offset = sizeof(iphc_header_t);
    ur_ip6_prefix_t prefix;

    uint16_t tmp = (((uint16_t)iphc_data[0]) << 8 ) | iphc_data[1];
    iphc_header_t *iphc_header = (iphc_header_t *)&tmp;
    if (iphc_header->CID == STATEFULL_COMPRESS ||
        iphc_header->SAC == STATEFULL_COMPRESS ||
        iphc_header->DAC == STATEFULL_COMPRESS) {
        /* does not support statefull comporess yet */
        return UR_ERROR_FAIL;
    }

    /* Set IPv6 version, traffic class and flow label. */
    ip6_header->v_tc_fl = IP_VERSION_6;
    if (iphc_header->TF == TC_FL_BOTH_APEENDED) {
        ip6_header->v_tc_fl |= ((uint32_t)iphc_data[offset++]) << 20;
        ip6_header->v_tc_fl |= ((uint32_t)(iphc_data[offset++] & 0x0F)) << 16;
        ip6_header->v_tc_fl |= ((uint32_t)iphc_data[offset++]) << 8;
        ip6_header->v_tc_fl |= ((uint32_t)iphc_data[offset++]);
    } else if (iphc_header->TF == DCSP_ELEDED_ECN_FL_APPENDED) {
        ip6_header->v_tc_fl |= (uint32_t)(iphc_data[offset] & 0xc0) << 20;
        ip6_header->v_tc_fl |= (uint32_t)(iphc_data[offset++] & 0x0f) << 16;
        ip6_header->v_tc_fl |= ((uint32_t)iphc_data[offset++]) << 8;
        ip6_header->v_tc_fl |= ((uint32_t)iphc_data[offset++]);
    } else if (iphc_header->TF == TC_APENDED_FL_ELIDED) {
        ip6_header->v_tc_fl |= ((uint32_t)iphc_data[offset++]) << 20;
    } else if (iphc_header->TF == TC_FL_BOTH_ELIDED) {
        ip6_header->v_tc_fl |= 0;
    }
    ip6_header->v_tc_fl = htonl(ip6_header->v_tc_fl);

    /* Set Next Header */
    if (iphc_header->NH == NEXT_HEADER_APPENDED) {
        ip6_header->next_header = iphc_data[offset++];
    } else {
        /* We should fill this later with NHC decoding */
        ip6_header->next_header = 0x00;
    }

    /* Set Hop Limit */
    if (iphc_header->HLIM == HOP_LIM_APPENDED) {
        ip6_header->hop_lim = iphc_data[offset++];
    } else if (iphc_header->HLIM == HOP_LIM_1) {
        ip6_header->hop_lim = 1;
    } else if (iphc_header->HLIM == HOP_LIM_64) {
        ip6_header->hop_lim = 64;
    } else if (iphc_header->HLIM == HOP_LIM_255) {
        ip6_header->hop_lim = 255;
    }

    if (nd_get_ip6_prefix(&prefix) != UR_ERROR_NONE) {
        return UR_ERROR_FAIL;
    }

    /* Source address decoding. */
    offset += ucast_addr_decompress(iphc_header->SAM, &prefix, &iphc_data[offset],
                                    &ip6_header->src, info->src.addr.short_addr);

    /* Destination address decoding. */
    if (iphc_header->M == MULTICAST_DESTINATION) {
        offset += mcast_addr_decompress(iphc_header->DAM, &iphc_data[offset],
                                        &ip6_header->dest);
    } else {
        offset += ucast_addr_decompress(iphc_header->DAM, &prefix, &iphc_data[offset],
                                        &ip6_header->dest, info->dest.addr.short_addr);
    }

    *hc_len = offset;

    return UR_ERROR_NONE;
}

static ur_error_t next_header_decompress(const uint8_t *nhc_data,
                                         uint8_t *buffer, uint8_t *hc_len)
{
    nhc_header_t *nhc_header = (nhc_header_t *)nhc_data;
    if (nhc_header->DP == NHC_UDP_DISPATCH) {
        /* UDP decompress */
        ur_udp_header_t *udp_header = (ur_udp_header_t *)buffer;
        uint8_t offset = sizeof(nhc_header_t);

        if (nhc_header->C == CHKSUM_ELIDED) {
            /* @todo support checksum decompress */
            return UR_ERROR_FAIL; /* not supported yet */
        }

        /* Decompress ports */
        if (nhc_header->P == NO_PORT_COMPRESSED) {
            udp_header->src_port = (uint16_t)nhc_data[offset++] << 8;
            udp_header->src_port |= (uint16_t)nhc_data[offset++];
            udp_header->dst_port = (uint16_t)nhc_data[offset++] << 8;
            udp_header->dst_port |= (uint16_t)nhc_data[offset++];
        } else if (nhc_header->P == DST_PORT_COMPRESSED) {
            udp_header->src_port = (uint16_t)nhc_data[offset++] << 8;
            udp_header->src_port |= (uint16_t)nhc_data[offset++];
            udp_header->dst_port = 0xf000 | nhc_data[offset++];
        } else if (nhc_header->P == SRC_PORT_COMPRESSED) {
            udp_header->src_port = 0xf000 | nhc_data[offset++];
            udp_header->dst_port = (uint16_t)nhc_data[offset++] << 8;
            udp_header->dst_port |= (uint16_t)nhc_data[offset++];
        } else if (nhc_header->P == BOTH_PORT_COMPRESSED) {
            udp_header->src_port = 0xf0b0 | ((nhc_data[offset] & 0xF0) >> 4);
            udp_header->dst_port = 0xf0b0 | (nhc_data[offset] & 0x0F);
            offset += 1;
        }
        udp_header->src_port = htons(udp_header->src_port);
        udp_header->dst_port = htons(udp_header->dst_port);

        /* fill in udp header CHECKSUM field */
        udp_header->chksum = (uint16_t)nhc_data[offset++] << 8;
        udp_header->chksum |= (uint16_t)nhc_data[offset++];
        udp_header->chksum = htons(udp_header->chksum);
        *hc_len = offset;

        return UR_ERROR_NONE;
    } else {
        /* does not support other NHC yet */
        return UR_ERROR_FAIL;
    }
}

message_t *lp_header_decompress(message_t *message)
{
    ur_error_t error;
    uint8_t *iphc_data = (uint8_t *)message_get_payload(message);
    uint16_t pkt_len = message_get_msglen(message);
    ur_ip6_header_t *ip6_header;
    uint8_t  iphc_len, nhc_len, hc_len;
    uint16_t ip_payload_len, dec_header_len;
    uint8_t  *buffer;
    message_t *dec_message = NULL;
    message_info_t *info;

    buffer = (uint8_t *)ur_mem_alloc(UR_IP6_HLEN + UR_UDP_HLEN);
    if (buffer == NULL) {
        ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_6LOWPAN,
               "lowpan6: out of memory, decompress failed\r\n");
        message_free(message);
        return NULL;
    }
    info = message->info;

    /* decompress IPv6 header */
    hc_len = 0;
    ip6_header = (ur_ip6_header_t *)buffer;
    error = ipv6_header_decompress(info, iphc_data, buffer, &iphc_len);
    if (error != UR_ERROR_NONE) {
        ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_6LOWPAN,
               "lowpan6: unsupported 6LowPAN IPHC header, decompress failed\r\n");
        message_free(message);
        ur_mem_free(buffer, (UR_IP6_HLEN + UR_UDP_HLEN));
        return NULL;
    }
    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_6LOWPAN,
           "lowpan6: decompressed %d bytes 6LowPAN IPHC header"
           " to 40 bytes IPv6 header\r\n", iphc_len);

    hc_len += iphc_len;
    dec_header_len = UR_IP6_HLEN;
    ip_payload_len = pkt_len - hc_len;

    /* decompress next header */
    /* Next Header Compression (NHC) decoding? */
    uint16_t tmp = (((uint16_t)iphc_data[0]) << 8 ) | iphc_data[1];
    iphc_header_t *iphc_header = (iphc_header_t *)&tmp;
    if (iphc_header->NH == 0b1) {
        uint8_t *nhc_data = iphc_data + iphc_len;
        ur_udp_header_t *udp_header = (ur_udp_header_t *)(buffer + UR_IP6_HLEN);
        error = next_header_decompress(nhc_data, buffer + UR_IP6_HLEN, &nhc_len);
        if (error != UR_ERROR_NONE) {
            ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_6LOWPAN,
                   "lowpan6: unsupported 6LowPAN NHC header, decompress failed\r\n");
            message_free(message);
            ur_mem_free(buffer, (UR_IP6_HLEN + UR_UDP_HLEN));
            return NULL;
        }
        ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_6LOWPAN,
               "lowpan6: decompressed %d bytes 6LowPAN NHC header"
               " to 8 bytes UDP header\r\n", nhc_len);

        hc_len += nhc_len;
        dec_header_len = UR_IP6_HLEN + UR_UDP_HLEN;
        ip_payload_len = pkt_len - hc_len + 8;
        udp_header->length = htons(ip_payload_len);
        ip6_header->next_header = UR_IPPROTO_UDP;
    }

    ip6_header->len = htons(ip_payload_len);

    message_set_payload_offset(message, -hc_len);
    dec_message = message_alloc(dec_header_len, LOWPAN6_1);
    if (dec_message == NULL) {
        message_free(message);
        ur_mem_free(buffer, (UR_IP6_HLEN + UR_UDP_HLEN));
        return NULL;
    }
    message_copy_from(dec_message, buffer, dec_header_len);
    message_concatenate(dec_message, message, false);
    ur_mem_free(buffer, (UR_IP6_HLEN + UR_UDP_HLEN));

    return dec_message;
}

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
    uint8_t *payload;

    if (p == NULL || reass_p == NULL) {
        return UR_ERROR_FAIL;
    }

    info = p->info;
    *reass_p = NULL;
    payload = (uint8_t *)message_get_payload(p);
    memcpy((uint8_t *)&frag_header_content, payload, sizeof(frag_header_t));
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
                    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_6LOWPAN,
                           "lowpan6: received duplicated FRAG_1 from"
                           " %04hx (tag=%u tot_len=%u), drop it\r\n",
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

        ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_6LOWPAN,
               "lowpan6: received new FRAG_1 from %04hx, tag=%u tot_len=%u len=%u offset=0\r\n",
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
            ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_6LOWPAN,
                   "lowpan6: received duplicated FRAG_N from"
                   " %04hx, tag=%u len=%u offset=%u, drop it\r\n",
                   info->src.addr.short_addr, datagram_tag, message_get_msglen(p),
                   datagram_offset);
            return UR_ERROR_FAIL;
        } else if (message_get_msglen(lrh->message) < datagram_offset) {
            /* We have missed a fragment. Delete whole reassembly. */
            ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_6LOWPAN,
                   "lowpan6: received disordered FRAG_N from %04hx,"
                   " tag=%u len=%u offset=%u, drop the whole fragment packets\r\n",
                   info->src.addr.short_addr, datagram_tag, message_get_msglen(p),
                   datagram_offset);
            dequeue_list_element(lrh);
            message_free(lrh->message);
            ur_mem_free(lrh, sizeof(lowpan_reass_t));
            return UR_ERROR_FAIL;
        }

        ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_6LOWPAN,
               "lowpan6: received FRAG_N from %04hx, tag=%u len=%u offset=%u\r\n",
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
        } else {
            return UR_ERROR_NONE;
        }
    } else {
        ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_6LOWPAN,
               "lowpan6: unrecognized FRAG packet, drop it\r\n");
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
            ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_6LOWPAN,
                   "lowpan6: fragment packts from %04hx (tag=%u tot_len=%u)"
                   " timeout, drop from reassemble queue\r\n",
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

bool lp_reass_queue_empty(void)
{
    return (g_reass_list == NULL);
}


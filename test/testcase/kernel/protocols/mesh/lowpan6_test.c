#include "yunit.h"

#include "umesh_types.h"
#include "core/mesh_forwarder.h"
#include "core/lowpan6.h"
#include "core/network_data.h"
#include "ipv6/ip6.h"
#include "utilities/encoding.h"
#include "utilities/message.h"

extern void hal_arch_time_msleep(int ms);

void test_uradar_6lowpan_case(void)
{
    uint8_t ip6_buffer[50];
    uint8_t lowpan_buffer[50];
    uint8_t ip_hdr_len, lowpan_hdr_len, pkt_len;
    message_t *message, *dec_message;
    ur_ip6_header_t *ip6hdr;
    iphc_header_t iphc_header;
    ur_udp_header_t *udphdr;
    nhc_header_t  nhc_header;
    ur_ip6_prefix_t prefix;

    lp_init();
    nd_get_ip6_prefix(&prefix);

    /*********test header compress and decompress functions*********/
    memset(ip6_buffer, 0x00, sizeof(ip6_buffer));
    ip6hdr = (ur_ip6_header_t *)ip6_buffer;
    udphdr  = (ur_udp_header_t *)(ip6_buffer + UR_IP6_HLEN);

    ip6hdr->v_tc_fl = ur_swap32(6 << 28); /* TC=0, Fl=0 */
    ip6hdr->len = ur_swap16(sizeof(ip6_buffer) - UR_IP6_HLEN);
    ip6hdr->hop_lim = 255;
    ip6hdr->next_header = UR_IPPROTO_UDP;
    ip6hdr->src.m8[0] = 0xfc; /*src is local unicast address */
    ip6hdr->src.m8[6] = prefix.prefix.m8[6];
    ip6hdr->src.m8[7] = prefix.prefix.m8[7];
    ip6hdr->src.m8[15] = 0x01;
    ip6hdr->dest.m8[0] = 0xfc; /*dest is local unicast address */
    ip6hdr->dest.m8[6] = prefix.prefix.m8[6];
    ip6hdr->dest.m8[7] = prefix.prefix.m8[7];
    ip6hdr->dest.m8[15] = 0x02;
    udphdr->src_port = ur_swap16(0xf0b1);
    udphdr->dst_port = ur_swap16(0xf0b1);
    udphdr->length = ur_swap16(sizeof(ip6_buffer) - UR_IP6_HLEN);
    udphdr->chksum = ur_swap16(0xffbb);
    YUNIT_ASSERT(UR_ERROR_NONE == lp_header_compress(ip6_buffer, lowpan_buffer, &ip_hdr_len, &lowpan_hdr_len));
    *((uint16_t *)&iphc_header) = ur_swap16(*((uint16_t *)lowpan_buffer));
    YUNIT_ASSERT(IPHC_DISPATCH == iphc_header.DP);
    YUNIT_ASSERT(TC_FL_BOTH_ELIDED == iphc_header.TF);
    YUNIT_ASSERT(NEXT_HEADER_ELIDED == iphc_header.NH);
    YUNIT_ASSERT(HOP_LIM_255 == iphc_header.HLIM);
    YUNIT_ASSERT(STATELESS_COMPRESS == iphc_header.CID);
    YUNIT_ASSERT(STATELESS_COMPRESS == iphc_header.SAC);
    YUNIT_ASSERT(STATELESS_COMPRESS == iphc_header.DAC);
    YUNIT_ASSERT(UCAST_ADDR_ELIDED == iphc_header.SAM);
    YUNIT_ASSERT(UCAST_ADDR_ELIDED == iphc_header.DAM);
    nhc_header = *((nhc_header_t *)(lowpan_buffer + sizeof(iphc_header)));
    YUNIT_ASSERT(NHC_UDP_DISPATCH == nhc_header.DP);
    YUNIT_ASSERT(CHKSUM_APPENDED  == nhc_header.C);
    YUNIT_ASSERT(BOTH_PORT_COMPRESSED == nhc_header.P);
    YUNIT_ASSERT((UR_IP6_HLEN + UR_UDP_HLEN) == ip_hdr_len);
    YUNIT_ASSERT(6 == lowpan_hdr_len);
    pkt_len = lowpan_hdr_len + sizeof(ip6_buffer) - ip_hdr_len;
    message = message_alloc(pkt_len);
    message_copy_from(message, lowpan_buffer, pkt_len);
    dec_message = lp_header_decompress(message);
    YUNIT_ASSERT_PTR_NOT_NULL(dec_message);
    YUNIT_ASSERT(sizeof(ip6_buffer) == message_get_msglen(dec_message));
    message_free(dec_message);

    ip6hdr->v_tc_fl = ur_swap32((6 << 28) | (1 << 20)); /* TC=1, Fl=0 */
    ip6hdr->hop_lim = 64;
    ip6hdr->next_header = UR_IPPROTO_UDP;
    ip6hdr->src.m8[0] = 0xfc; /*src is local address that can be compressed to 16bit */
    memcpy(&ip6hdr->src.m8[8], FIXED_IID, 6);
    ip6hdr->dest.m8[0] = 0xff; /*dest is multicast address that can be compressed to 8bit*/
    ip6hdr->dest.m8[6] = 0x00;
    ip6hdr->dest.m8[7] = 0x00;
    ip6hdr->dest.m8[1] = 0x02;
    udphdr->src_port = ur_swap16(0xf0b1);
    udphdr->dst_port = ur_swap16(0x00b1);
    YUNIT_ASSERT(UR_ERROR_NONE == lp_header_compress(ip6_buffer, lowpan_buffer, &ip_hdr_len, &lowpan_hdr_len));
    *((uint16_t *)&iphc_header) = ur_swap16(*((uint16_t *)lowpan_buffer));
    YUNIT_ASSERT(TC_APENDED_FL_ELIDED == iphc_header.TF);
    YUNIT_ASSERT(HOP_LIM_64 == iphc_header.HLIM);
    YUNIT_ASSERT(UCAST_ADDR_16BIT == iphc_header.SAM);
    YUNIT_ASSERT(MCAST_ADDR_8BIT == iphc_header.DAM);
    nhc_header = *((nhc_header_t *)(lowpan_buffer + sizeof(iphc_header) + 3));
    YUNIT_ASSERT(SRC_PORT_COMPRESSED == nhc_header.P);
    YUNIT_ASSERT((UR_IP6_HLEN + UR_UDP_HLEN) == ip_hdr_len);
    YUNIT_ASSERT(12 == lowpan_hdr_len);
    pkt_len = lowpan_hdr_len + sizeof(ip6_buffer) - ip_hdr_len;
    message = message_alloc(pkt_len);
    message_copy_from(message, lowpan_buffer, pkt_len);
    dec_message = lp_header_decompress(message);
    YUNIT_ASSERT_PTR_NOT_NULL(dec_message);
    YUNIT_ASSERT(sizeof(ip6_buffer) == message_get_msglen(dec_message));
    message_free(dec_message);

    ip6hdr->v_tc_fl = ur_swap32((6 << 28) | (1 << 0)); /* TC=0, Fl=1 */
    ip6hdr->hop_lim = 1;
    ip6hdr->next_header = UR_IPPROTO_UDP;
    ip6hdr->src.m8[8] = 0xff; /*src is local address that can be compressed to 64bit */
    ip6hdr->dest.m8[13] = 0x03; /*dest is multicast address that can be compressed to 32bit*/
    udphdr->src_port = ur_swap16(0x00b1);
    udphdr->dst_port = ur_swap16(0xf0b1);
    YUNIT_ASSERT(UR_ERROR_NONE == lp_header_compress(ip6_buffer, lowpan_buffer, &ip_hdr_len, &lowpan_hdr_len));
    *((uint16_t *)&iphc_header) = ur_swap16(*((uint16_t *)lowpan_buffer));
    YUNIT_ASSERT(DCSP_ELEDED_ECN_FL_APPENDED == iphc_header.TF);
    YUNIT_ASSERT(HOP_LIM_1 == iphc_header.HLIM);
    YUNIT_ASSERT(UCAST_ADDR_64BIT == iphc_header.SAM);
    YUNIT_ASSERT(MCAST_ADDR_32BIT == iphc_header.DAM);
    nhc_header = *((nhc_header_t *)(lowpan_buffer + sizeof(iphc_header) + 15));
    YUNIT_ASSERT(DST_PORT_COMPRESSED == nhc_header.P);
    YUNIT_ASSERT((UR_IP6_HLEN + UR_UDP_HLEN) == ip_hdr_len);
    YUNIT_ASSERT(23 == lowpan_hdr_len);
    pkt_len = lowpan_hdr_len + sizeof(ip6_buffer) - ip_hdr_len;
    message = message_alloc(pkt_len);
    message_copy_from(message, lowpan_buffer, pkt_len);
    dec_message = lp_header_decompress(message);
    YUNIT_ASSERT_PTR_NOT_NULL(dec_message);
    YUNIT_ASSERT(sizeof(ip6_buffer) == message_get_msglen(dec_message));
    message_free(dec_message);

    ip6hdr->v_tc_fl = ur_swap32((6 << 28) | (1 << 20) | (1 << 0)); /* TC=1, Fl=1 */
    ip6hdr->hop_lim = 1;
    ip6hdr->src.m8[0] = 0x0f; /*src is address that can not be compressed */
    ip6hdr->dest.m8[11] = 0x03; /*dest is multicast address that can be compressed to 48bit*/
    ip6hdr->next_header = UR_IPPROTO_UDP;
    udphdr->src_port = ur_swap16(0x00b1);
    udphdr->dst_port = ur_swap16(0x00b1);
    YUNIT_ASSERT(UR_ERROR_NONE == lp_header_compress(ip6_buffer, lowpan_buffer, &ip_hdr_len, &lowpan_hdr_len));
    *((uint16_t *)&iphc_header) = ur_swap16(*((uint16_t *)lowpan_buffer));
    YUNIT_ASSERT(TC_FL_BOTH_APEENDED == iphc_header.TF);
    YUNIT_ASSERT(HOP_LIM_1 == iphc_header.HLIM);
    YUNIT_ASSERT(UCAST_ADDR_128BIT == iphc_header.SAM);
    YUNIT_ASSERT(MCAST_ADDR_48BIT == iphc_header.DAM);
    nhc_header = *((nhc_header_t *)(lowpan_buffer + sizeof(iphc_header) + 26));
    YUNIT_ASSERT(NO_PORT_COMPRESSED == nhc_header.P);
    YUNIT_ASSERT((UR_IP6_HLEN + UR_UDP_HLEN) == ip_hdr_len);
    YUNIT_ASSERT(35 == lowpan_hdr_len);
    pkt_len = lowpan_hdr_len + sizeof(ip6_buffer) - ip_hdr_len;
    message = message_alloc(pkt_len);
    message_copy_from(message, lowpan_buffer, pkt_len);
    dec_message = lp_header_decompress(message);
    YUNIT_ASSERT_PTR_NOT_NULL(dec_message);
    YUNIT_ASSERT(sizeof(ip6_buffer) == message_get_msglen(dec_message));
    message_free(dec_message);

    ip6hdr->v_tc_fl = ur_swap32((6 << 28) | (1 << 0)); /* TC=0, Fl=1 */
    ip6hdr->hop_lim = 20;
    ip6hdr->dest.m8[9] = 0x03; /*dest is multicast address that can not be compressed */
    ip6hdr->next_header = UR_IPPROTO_ICMPV6;
    ip6hdr->src.m8[0] = 0xff;
    ip6hdr->dest.m8[0] = 0xff;
    YUNIT_ASSERT(UR_ERROR_NONE == lp_header_compress(ip6_buffer, lowpan_buffer, &ip_hdr_len, &lowpan_hdr_len));
    *((uint16_t *)&iphc_header) = ur_swap16(*((uint16_t *)lowpan_buffer));
    YUNIT_ASSERT(DCSP_ELEDED_ECN_FL_APPENDED == iphc_header.TF);
    YUNIT_ASSERT(HOP_LIM_APPENDED == iphc_header.HLIM);
    YUNIT_ASSERT(NEXT_HEADER_APPENDED == iphc_header.NH);
    YUNIT_ASSERT(UCAST_ADDR_128BIT == iphc_header.SAM);
    YUNIT_ASSERT(MCAST_ADDR_128BIT == iphc_header.DAM);
    YUNIT_ASSERT(UR_IP6_HLEN == ip_hdr_len);
    YUNIT_ASSERT(39 == lowpan_hdr_len);
    pkt_len = lowpan_hdr_len + sizeof(ip6_buffer) - ip_hdr_len;
    message = message_alloc(pkt_len);
    message_copy_from(message, lowpan_buffer, pkt_len);
    dec_message = lp_header_decompress(message);
    YUNIT_ASSERT_PTR_NOT_NULL(dec_message);
    YUNIT_ASSERT(sizeof(ip6_buffer) == message_get_msglen(dec_message));
    message_free(dec_message);

    ip6hdr->v_tc_fl = ur_swap32((4 << 28)); /* IPv4 */
    YUNIT_ASSERT(UR_ERROR_FAIL == lp_header_compress(ip6_buffer, lowpan_buffer, &ip_hdr_len, &lowpan_hdr_len));

    uint8_t tmp = lowpan_buffer[1];
    lowpan_buffer[1] = tmp | 0x80; /* STATEFULL compress */
    message = message_alloc(pkt_len);
    message_copy_from(message, lowpan_buffer, pkt_len);
    dec_message = lp_header_decompress(message);
    YUNIT_ASSERT_PTR_NULL(dec_message);

    lowpan_buffer[1] = tmp | 0x40; /* STATEFULL compress */
    message = message_alloc(pkt_len);
    message_copy_from(message, lowpan_buffer, pkt_len);
    dec_message = lp_header_decompress(message);
    YUNIT_ASSERT_PTR_NULL(dec_message);

    lowpan_buffer[1] = tmp | 0x04; /* STATEFULL compress */
    message = message_alloc(pkt_len);
    message_copy_from(message, lowpan_buffer, pkt_len);
    dec_message = lp_header_decompress(message);
    YUNIT_ASSERT_PTR_NULL(dec_message);

    /*************** test reassemble functions *******************/
    message_t *reass_pkt = NULL;
    uint8_t   *data;
    uint16_t  size = 256, length = 96, offset = 0;

    message = message_alloc(length + sizeof(frag_header_t) - 1);
    data = message_get_payload(message);
    data[0] = 0xC0 | ((size & 0x0700) >> 8); /* FRAG_1 packet from 0x1000 */
    data[1] = size & 0xff;
    data[2] = 0x00;
    data[3] = 0x01;
    YUNIT_ASSERT(UR_ERROR_NONE == lp_reassemble(message, &reass_pkt));
    YUNIT_ASSERT_PTR_NULL(reass_pkt);

    YUNIT_ASSERT(UR_ERROR_FAIL == lp_reassemble(NULL, &reass_pkt));
    YUNIT_ASSERT(UR_ERROR_FAIL == lp_reassemble(message, &reass_pkt));

    message = message_alloc(length + sizeof(frag_header_t) - 1);
    data = message_get_payload(message);
    data[0] = 0xC0 | ((size & 0x0700) >> 8); /* repeated FRAG_1 packet from 0x1000 */
    data[1] = size & 0xff;
    data[2] = 0x00;
    data[3] = 0x01;
    YUNIT_ASSERT(UR_ERROR_FAIL == lp_reassemble(message, &reass_pkt));

    message = message_alloc(length + sizeof(frag_header_t) - 1);
    data = message_get_payload(message);
    data[0] = 0xC0 | ((size & 0x0700) >> 8); /* FRAG_1 packet from 0x2000 */
    data[1] = size & 0xff;
    data[2] = 0x00;
    data[3] = 0x02;
    YUNIT_ASSERT(UR_ERROR_NONE == lp_reassemble(message, &reass_pkt));
    YUNIT_ASSERT_PTR_NULL(reass_pkt);

    offset = length;
    message = message_alloc(length + sizeof(frag_header_t));
    data = message_get_payload(message);
    data[0] = 0xE0 | ((size & 0x0700) >> 8); /* FRAG_N packet from 0x2000 */
    data[1] = size & 0xff;
    data[2] = 0x00;
    data[3] = 0x02;
    data[4] = (offset >> 3);
    YUNIT_ASSERT(UR_ERROR_NONE == lp_reassemble(message, &reass_pkt));
    YUNIT_ASSERT_PTR_NULL(reass_pkt);

    message = message_alloc(length + sizeof(frag_header_t));
    data = message_get_payload(message);
    data[0] = 0xE0 | ((size & 0x0700) >> 8); /* repeated FRAG_N packet from 0x2000 */
    data[1] = size & 0xff;
    data[2] = 0x00;
    data[3] = 0x02;
    data[4] = (offset >> 3);
    YUNIT_ASSERT(UR_ERROR_FAIL == lp_reassemble(message, &reass_pkt));
    YUNIT_ASSERT_PTR_NULL(reass_pkt);

    offset += length;
    message = message_alloc(size - 2 * length + sizeof(frag_header_t));
    data = message_get_payload(message);
    data[0] = 0xE0 | ((size & 0x0700) >> 8); /* FRAG_N packet from 0x2000 */
    data[1] = size & 0xff;
    data[2] = 0x00;
    data[3] = 0x02;
    data[4] = (offset >> 3);
    YUNIT_ASSERT(UR_ERROR_NONE == lp_reassemble(message, &reass_pkt));
    YUNIT_ASSERT_PTR_NOT_NULL(reass_pkt);

    offset += length;
    message = message_alloc(length + sizeof(frag_header_t));
    data = message_get_payload(message);
    data[0] = 0xE0 | ((size & 0x0700) >> 8); /*rougue FRAG_N packet from 0x2000 */
    data[1] = size & 0xff;
    data[2] = 0x00;
    data[3] = 0x03;
    data[4] = (offset >> 3);
    YUNIT_ASSERT(UR_ERROR_FAIL == lp_reassemble(message, &reass_pkt));

    message = message_alloc(length + sizeof(frag_header_t) - 1);
    data = message_get_payload(message);
    data[0] = 0xC0 | ((size & 0x0700) >> 8); /* FRAG_1 packet from 0x2000 */
    data[1] = size & 0xff;
    data[2] = 0x00;
    data[3] = 0x04;
    YUNIT_ASSERT(UR_ERROR_NONE == lp_reassemble(message, &reass_pkt));
    YUNIT_ASSERT_PTR_NULL(reass_pkt);

    offset += length;
    offset += length; /* pack with unexpected offset */
    message = message_alloc(size - 2 * length + sizeof(frag_header_t));
    data = message_get_payload(message);
    data[0] = 0xE0 | ((size & 0x0700) >> 8); /* FRAG_N packet from 0x2000 */
    data[1] = size & 0xff;
    data[2] = 0x00;
    data[3] = 0x04;
    data[4] = (offset >> 3);
    YUNIT_ASSERT(UR_ERROR_FAIL == lp_reassemble(message, &reass_pkt));

    message = message_alloc(length + sizeof(frag_header_t) - 1);
    data = message_get_payload(message);
    data[0] = 0xC0 | ((size & 0x0700) >> 8); /* FRAG_1 packet from 0x3000 */
    data[1] = size & 0xff;
    data[2] = 0x00;
    data[3] = 0x02;
    YUNIT_ASSERT(UR_ERROR_NONE == lp_reassemble(message, &reass_pkt));
    YUNIT_ASSERT_PTR_NULL(reass_pkt);

    message = message_alloc(length + sizeof(frag_header_t) - 1);
    data = message_get_payload(message);
    data[0] = 0xC0 | ((size & 0x0700) >> 8); /* FRAG_1 packet from 0x4000 */
    data[1] = size & 0xff;
    data[2] = 0x00;
    data[3] = 0x02;
    YUNIT_ASSERT(UR_ERROR_NONE == lp_reassemble(message, &reass_pkt));
    YUNIT_ASSERT_PTR_NULL(reass_pkt);

    message = message_alloc(length + sizeof(frag_header_t));
    data = message_get_payload(message);
    data[0] = 0xA0 | ((size & 0x0700) >> 8); /* packet with wrong DISPATCH */
    data[1] = size & 0xff;
    data[2] = 0x00;
    data[3] = 0x02;
    data[4] = (offset >> 3);
    YUNIT_ASSERT(UR_ERROR_FAIL == lp_reassemble(message, &reass_pkt));
    YUNIT_ASSERT_PTR_NULL(reass_pkt);

    hal_arch_time_msleep(6000); /* wait timer_hanlder to clean up incomplete fragments */
}

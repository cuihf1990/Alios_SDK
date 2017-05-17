
#include "yunit.h"

#include "yos/framework.h"
#include "yos/kernel.h"

#include "umesh.h"
#include "core/link_mgmt.h"
#include "core/sid_allocator.h"
#include "core/router_mgr.h"
#include "utilities/logging.h"
#include "utilities/encoding.h"
#include "tools/cli.h"

#include "dda_util.h"

static void topology_line_case(void)
{
    char ping_cmd[64];
    const ur_netif_ip6_address_t *myaddr;

    /* topology:
     *   router     super       super    router
     *   11 <------> 12  <===>  13 <---> 14
     */
    set_line_rssi(11, 14);

    start_node_ext(13, MODE_SUPER, -1, -1);
    check_p2p_str_wait("leader", 13, "testcmd state", 10);
    check_p2p_str_wait("VECTOR_ROUTER", 13, "testcmd router", 2);

    start_node_ext(12, MODE_SUPER, -1, -1);
    check_p2p_str_wait("super_router", 12, "testcmd state", 10);
    check_p2p_str_wait("VECTOR_ROUTER", 12, "testcmd router", 2);

    start_node_ext(14, MODE_MOBILE, -1, -1);
    check_p2p_str_wait("leaf", 14, "testcmd state", 10);
    check_p2p_str_wait("SID_ROUTER", 14, "testcmd router", 2);

    ur_mesh_set_mode(MODE_MOBILE);
    cmd_to_agent("start");
    check_cond_wait((DEVICE_STATE_LEAF == mm_get_device_state()), 15);
    YUNIT_ASSERT(ur_router_get_default_router() == SID_ROUTER);

    myaddr = ur_mesh_get_ucast_addr();
    snprintf(ping_cmd, sizeof ping_cmd, "send 14 ping " IP6_ADDR_FMT, IP6_ADDR_DATA(myaddr->addr));
    cmd_to_master(ping_cmd);
    check_p2p_str_wait("1", 14, "testcmd icmp_acked", 5);

    stop_node(12);
    stop_node(13);
    stop_node(14);
    cmd_to_agent("stop");
}

static void dual_if_topology_line_case(void)
{
    char ping_cmd[64];

    /* topology:
     *   ble       wifi/ble  wifi/ble  ble
     *   12 <------> 13  <===>  14 <---> 15
     */
    set_line_rssi(12, 15);

    start_node_ext(13, MODE_SUPER, -1, 3);
    check_p2p_str_wait("leader", 13, "testcmd state", 10);
    check_p2p_str_wait("VECTOR_ROUTER", 13, "testcmd router", 2);

    start_node_ext(14, MODE_SUPER, -1, 3);
    check_p2p_str_wait("super_router", 14, "testcmd state", 10);
    check_p2p_str_wait("VECTOR_ROUTER", 14, "testcmd router", 2);

    start_node_ext(15, MODE_MOBILE, -1, 2);
    check_p2p_str_wait("leaf", 15, "testcmd state", 10);
    check_p2p_str_wait("SID_ROUTER", 15, "testcmd router", 2);

    start_node_ext(12, MODE_MOBILE, -1, 2);
    check_p2p_str_wait("leaf", 12, "testcmd state", 10);
    check_p2p_str_wait("SID_ROUTER", 12, "testcmd router", 2);

    char *ipaddr = dda_p2p_req_and_wait(15, "testcmd ipaddr", 5);
    YUNIT_ASSERT(ipaddr != NULL);
    if (ipaddr) {
        snprintf(ping_cmd, sizeof ping_cmd, "send 12 ping %s", ipaddr);
        yos_free(ipaddr);

        cmd_to_master(ping_cmd);
        check_p2p_str_wait("1", 12, "testcmd icmp_acked", 5);
    }

    stop_node(12);
    stop_node(13);
    stop_node(14);
    stop_node(15);
}

void test_uradar_layer_routing_2mobile_case(void)
{
    run_times(topology_line_case(), 1);
    run_times(dual_if_topology_line_case(), 1);
}

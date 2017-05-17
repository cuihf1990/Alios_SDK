#include "yunit.h"

#include "yos/framework.h"
#include "yos/kernel.h"

#include "umesh.h"
#include "core/link_mgmt.h"
#include "core/sid_allocator.h"
#include "core/router_mgr.h"
#include "utilities/logging.h"
#include "utilities/encoding.h"
#include "cli/cli.h"

#include "dda_util.h"

static void one_super_router_case(void)
{
    char ping_cmd[64];
    const ur_netif_ip6_address_t *myaddr;

    /* topology:
     *   leader        router
     *   12 <---wifi--> 11
     *   ^
     *   |             router
     *   |  <---wifi--> 13
     *   |
     *   |             router
     *   |  <---ble---> 14
     *   |
     *   |             router
     *   |  <---ble---> 15
     *
     */

    set_full_rssi(11, 15);

    start_node_ext(12, MODE_SUPER, -1, 3);
    check_p2p_str_wait("leader", 12, "testcmd state", 10);
    check_p2p_str_wait("VECTOR_ROUTER", 12, "testcmd router", 2);

    ur_mesh_set_mode(MODE_RX_ON);
    cmd_to_agent("start");
    check_cond_wait((DEVICE_STATE_ROUTER == mm_get_device_state()), 15);
    YUNIT_ASSERT(ur_router_get_default_router() == SID_ROUTER);

    start_node_ext(13, MODE_RX_ON, -1, 1);
    check_p2p_str_wait("router", 13, "testcmd state", 10);
    check_p2p_str_wait("SID_ROUTER", 13, "testcmd router", 2);

    start_node_ext(14, MODE_RX_ON, -1, 2);
    check_p2p_str_wait("router", 14, "testcmd state", 10);
    check_p2p_str_wait("SID_ROUTER", 14, "testcmd router", 2);

    start_node_ext(15, MODE_RX_ON, -1, 2);
    check_p2p_str_wait("router", 15, "testcmd state", 10);
    check_p2p_str_wait("SID_ROUTER", 15, "testcmd router", 2);

    myaddr = ur_mesh_get_ucast_addr();
    snprintf(ping_cmd, sizeof ping_cmd, "send 14 ping " IP6_ADDR_FMT, IP6_ADDR_DATA(myaddr->addr));
    cmd_to_master(ping_cmd);
    check_p2p_str_wait("1", 14, "testcmd icmp_acked", 5);

    stop_node(12);
    stop_node(13);
    stop_node(14);
    stop_node(15);
    cmd_to_agent("stop");
}

static void two_super_router_case(void)
{
    char ping_cmd[64];
    const ur_netif_ip6_address_t *myaddr;

    /* topology:
     *   leader        router
     *   12 <----------------> 13
     *   |                     |
     *   |             router  |
     *   |  <-----wifi----> 11 |
     *   |                     |
     *   |             router  |
     *   |  <-----wifi----> 14 |
     *   |                     |
     *   |             router  |
     *   |  <-----ble-----> 15 |
     *   |                     |
     *   |             router  |
     *   |  <-----ble-----> 16 |
     *
     */

    set_full_rssi(11, 16);

    start_node_ext(12, MODE_SUPER, -1, 3);
    check_p2p_str_wait("leader", 12, "testcmd state", 10);
    check_p2p_str_wait("VECTOR_ROUTER", 12, "testcmd router", 2);

    start_node_ext(13, MODE_SUPER, -1, 3);
    check_p2p_str_wait("super_router", 13, "testcmd state", 10);
    check_p2p_str_wait("VECTOR_ROUTER", 13, "testcmd router", 2);

    ur_mesh_set_mode(MODE_RX_ON);
    cmd_to_agent("start");
    check_cond_wait((DEVICE_STATE_ROUTER == mm_get_device_state()), 15);
    YUNIT_ASSERT(ur_router_get_default_router() == SID_ROUTER);

    start_node_ext(14, MODE_RX_ON, -1, 1);
    check_p2p_str_wait("router", 14, "testcmd state", 10);
    check_p2p_str_wait("SID_ROUTER", 14, "testcmd router", 2);

    start_node_ext(15, MODE_RX_ON, -1, 2);
    check_p2p_str_wait("router", 15, "testcmd state", 10);
    check_p2p_str_wait("SID_ROUTER", 15, "testcmd router", 2);

    start_node_ext(16, MODE_RX_ON, -1, 2);
    check_p2p_str_wait("router", 16, "testcmd state", 10);
    check_p2p_str_wait("SID_ROUTER", 16, "testcmd router", 2);

    myaddr = ur_mesh_get_ucast_addr();
    snprintf(ping_cmd, sizeof ping_cmd, "send 14 ping " IP6_ADDR_FMT, IP6_ADDR_DATA(myaddr->addr));
    cmd_to_master(ping_cmd);
    check_p2p_str_wait("1", 14, "testcmd icmp_acked", 5);

    stop_node(12);
    stop_node(13);
    stop_node(14);
    stop_node(15);
    stop_node(16);
    cmd_to_agent("stop");
}

void test_uradar_topo_case(void)
{
    run_times(one_super_router_case(), 1);
    run_times(two_super_router_case(), 1);
}

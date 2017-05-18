#include "yunit.h"

#include "yos/framework.h"
#include "yos/kernel.h"

#include "umesh.h"
#include "core/link_mgmt.h"
#include "core/sid_allocator.h"
#include "utilities/logging.h"
#include "utilities/encoding.h"
#include "tools/cli.h"

#include "dda_util.h"

static void run_in_hop1(void)
{
    const ur_netif_ip6_address_t *addr;
    char ping_cmd[64];

    set_full_rssi(11, 13);

    start_node(12);
    check_p2p_str_wait("leader", 12, "testcmd state", 10);
    start_node(13);
    check_p2p_str_wait("router", 13, "testcmd state", 10);

    cmd_to_agent("mode MOBILE");
    cmd_to_agent("start");
    yos_msleep(10000);

    YUNIT_ASSERT(ur_mesh_get_sid() == 0xc001);

    addr = ur_mesh_get_ucast_addr();
    snprintf(ping_cmd, sizeof ping_cmd, "send 12 ping " IP6_ADDR_FMT, IP6_ADDR_DATA(addr->addr));
    cmd_to_master(ping_cmd);
    check_p2p_str_wait("1", 12, "testcmd icmp_acked", 5);

    cmd_to_agent("stop");
    cmd_to_agent("mode FIXED");
    stop_node(12);
    stop_node(13);

    yos_msleep(2 * 1000);
}

static void run_in_hop2(void)
{
    const ur_netif_ip6_address_t *addr;
    char ping_cmd[64];

    set_line_rssi(11, 13);

    start_node(13);
    yos_msleep(5000);
    start_node(12);
    yos_msleep(5000);

    cmd_to_agent("mode MOBILE");
    cmd_to_agent("start");

    yos_msleep(10 * 1000);
    YUNIT_ASSERT(ur_mesh_get_sid() == 0xc001);

    addr = ur_mesh_get_ucast_addr();
    snprintf(ping_cmd, sizeof ping_cmd, "send 13 ping " IP6_ADDR_FMT, IP6_ADDR_DATA(addr->addr));
    cmd_to_master(ping_cmd);
    check_p2p_str_wait("1", 13, "testcmd icmp_acked", 5);

    cmd_to_agent("stop");
    cmd_to_agent("mode FIXED");
    stop_node(12);
    stop_node(13);

    yos_msleep(2 * 1000);
}


void test_uradar_1mobile_case(void)
{
    run_times(run_in_hop1(), 2);
    run_times(run_in_hop2(), 2);
}

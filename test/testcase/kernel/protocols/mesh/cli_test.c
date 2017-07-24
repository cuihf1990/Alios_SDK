#include <stdio.h>
#include <stdint.h>

#include <yos/kernel.h>
#include "yunit.h"

#include "dda_util.h"

#include "umesh.h"
#include "utilities/logging.h"
#include "utilities/encoding.h"
#include "core/mesh_mgmt.h"
#include "tools/cli.h"

void test_uradar_cli_case(void)
{
    cmd_to_agent("stop");
    cmd_to_agent("start");
    check_cond_wait(ur_mesh_get_device_state() == DEVICE_STATE_LEADER, 10);

    cmd_to_agent("help");
    cmd_to_agent("channel");
    cmd_to_agent("loglevel");
    cmd_to_agent("ipaddr");
    cmd_to_agent("macaddr");
    cmd_to_agent("meshnetsize");
    cmd_to_agent("meshnetid");
    cmd_to_agent("autotest");
    cmd_to_agent("prefix");
    cmd_to_agent("seclevel");
    cmd_to_agent("state");
    cmd_to_agent("stats");
    cmd_to_agent("whitelist");
    cmd_to_agent("whitelist add 0100000000000000");
    cmd_to_agent("whitelist remove 0100000000000000");
    cmd_to_agent("whitelist enable");
    cmd_to_agent("whitelist disable");
    cmd_to_agent("whitelist clear");
    cmd_to_agent("networks");

    char ping_cmd[64];
    const ur_netif_ip6_address_t *myaddr;

    myaddr = ur_mesh_get_ucast_addr();
    snprintf(ping_cmd, sizeof ping_cmd, "autotest " IP6_ADDR_FMT " 2", IP6_ADDR_DATA(myaddr->addr));
    cmd_to_agent(ping_cmd);
    yos_msleep(5 * 1000);

    cmd_to_agent("stop");
    yos_msleep(2 * 1000);
}


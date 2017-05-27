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
    yos_msleep(1 * 1000);
    cmd_to_agent("channel");
    yos_msleep(1 * 1000);
    cmd_to_agent("loglevel");
    yos_msleep(1 * 1000);
    cmd_to_agent("ipaddr");
    yos_msleep(1 * 1000);
    cmd_to_agent("macaddr");
    yos_msleep(1 * 1000);
    cmd_to_agent("meshnetsize");
    yos_msleep(1 * 1000);
    cmd_to_agent("meshnetid");
    yos_msleep(1 * 1000);
    cmd_to_agent("autotest");
    yos_msleep(1 * 1000);
    cmd_to_agent("prefix");
    yos_msleep(1 * 1000);
    cmd_to_agent("seclevel");
    yos_msleep(1 * 1000);
    cmd_to_agent("state");
    yos_msleep(1 * 1000);
    cmd_to_agent("stats");
    yos_msleep(1 * 1000);
    cmd_to_agent("whitelist");
    yos_msleep(1 * 1000);
    cmd_to_agent("whitelist add 0100000000000000");
    yos_msleep(1 * 1000);
    cmd_to_agent("whitelist remove 0100000000000000");
    yos_msleep(1 * 1000);
    cmd_to_agent("whitelist enable");
    yos_msleep(1 * 1000);
    cmd_to_agent("whitelist disable");
    yos_msleep(1 * 1000);
    cmd_to_agent("whitelist clear");
    yos_msleep(1 * 1000);
    cmd_to_agent("networks");
    yos_msleep(1 * 1000);

    char ping_cmd[64];
    const ur_netif_ip6_address_t *myaddr;

    myaddr = ur_mesh_get_ucast_addr();
    snprintf(ping_cmd, sizeof ping_cmd, "autotest " IP6_ADDR_FMT " 2", IP6_ADDR_DATA(myaddr->addr));
    cmd_to_agent(ping_cmd);
    yos_msleep(17 * 1000);

    cmd_to_agent("stop");
    yos_msleep(2 * 1000);
}


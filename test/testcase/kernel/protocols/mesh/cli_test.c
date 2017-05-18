#include "yunit.h"

#include "tools/cli.h"

extern void ur_ut_send_cmd_to_ddm(const char *cmd);

void test_uradar_cli_case(void)
{
    uint16_t startup_time = 5000;
    ur_ut_send_cmd_to_ddm("rssi 1 2 2 2");
    ur_ut_send_cmd_to_ddm("rssi 1 3 2 2");
    ur_ut_send_cmd_to_ddm("rssi 2 3 2 2");
    ur_ut_send_cmd_to_ddm("rssi 1 4 0 0");
    ur_ut_send_cmd_to_ddm("rssi 2 4 2 2");
    ur_ut_send_cmd_to_ddm("rssi 3 4 0 0");
    ur_ut_send_cmd_to_ddm("start 1");
    yos_msleep(startup_time);
    ur_ut_send_cmd_to_ddm("start 2");
    yos_msleep(startup_time);
    ur_ut_send_cmd_to_ddm("start 3");
    yos_msleep(startup_time);
    ur_ut_send_cmd_to_ddm("start 4");
    yos_msleep(startup_time);

    ur_ut_send_cmd_to_ddm("goto 2");
    ur_ut_send_cmd_to_ddm("help");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("channel");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("ipaddr");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("ipaddr add ff00::0001");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("ipaddr del ff00::0001");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("loglevel");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("macaddr");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("meshnetid");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("meshnetid 0x1000");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("meshnetsize");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("mode");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("mode RX_OFF MOBILE");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("mode RX_ON FIXED");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("nbrs");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("ping fc00:0:0:beef:0:0:0:0");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("prefix");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("router");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("router VECTOR_ROUTER");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("router SID_ROUTER");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("sids");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("state");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("stats");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("status");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist enable");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist add");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist add 01020304a50607");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist add G10203B405060708");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist add 010203B405060708");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist add 010203B405060708 1");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist remove 010203B405060708");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist clear");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist disable");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("stop");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("state");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("start");
    ur_ut_send_cmd_to_ddm("state");
    yos_msleep(startup_time);
    ur_ut_send_cmd_to_ddm("state");
    yos_msleep(startup_time);
    ur_ut_send_cmd_to_ddm("state");
    yos_msleep(200);

    ur_ut_send_cmd_to_ddm("goto 0");
    ur_ut_send_cmd_to_ddm("goto 1");
    ur_ut_send_cmd_to_ddm("state");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("sids");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("autotest fc00:0:0:beef:0:0:0:2000");
    yos_msleep(200);
    ur_ut_send_cmd_to_ddm("autotest fc08::fc");
    yos_msleep(2000);

    ur_ut_send_cmd_to_ddm("goto 0");
    ur_ut_send_cmd_to_ddm("goto 2");
    ur_ut_send_cmd_to_ddm("autotest fc08::fc");
    yos_msleep(2000);

    ur_ut_send_cmd_to_ddm("goto 0");
    ur_ut_send_cmd_to_ddm("stop 4");
    ur_ut_send_cmd_to_ddm("stop 3");
    ur_ut_send_cmd_to_ddm("stop 2");
    ur_ut_send_cmd_to_ddm("stop 1");
}



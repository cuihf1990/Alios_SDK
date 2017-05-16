#include "yunit.h"

#include "cli/cli.h"

extern void hal_arch_time_msleep(int ms);
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
    hal_arch_time_msleep(startup_time);
    ur_ut_send_cmd_to_ddm("start 2");
    hal_arch_time_msleep(startup_time);
    ur_ut_send_cmd_to_ddm("start 3");
    hal_arch_time_msleep(startup_time);
    ur_ut_send_cmd_to_ddm("start 4");
    hal_arch_time_msleep(startup_time);

    ur_ut_send_cmd_to_ddm("goto 2");
    ur_ut_send_cmd_to_ddm("help");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("channel");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("ipaddr");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("ipaddr add ff00::0001");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("ipaddr del ff00::0001");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("loglevel");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("macaddr");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("meshnetid");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("meshnetid 0x1000");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("meshnetsize");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("mode");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("mode RX_OFF MOBILE");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("mode RX_ON FIXED");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("nbrs");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("ping fc00:0:0:beef:0:0:0:0");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("prefix");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("router");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("router VECTOR_ROUTER");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("router SID_ROUTER");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("sids");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("state");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("stats");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("status");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist enable");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist add");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist add 01020304a50607");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist add G10203B405060708");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist add 010203B405060708");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist add 010203B405060708 1");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist remove 010203B405060708");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist clear");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("whitelist disable");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("stop");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("state");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("start");
    ur_ut_send_cmd_to_ddm("state");
    hal_arch_time_msleep(startup_time);
    ur_ut_send_cmd_to_ddm("state");
    hal_arch_time_msleep(startup_time);
    ur_ut_send_cmd_to_ddm("state");
    hal_arch_time_msleep(200);

    ur_ut_send_cmd_to_ddm("goto 0");
    ur_ut_send_cmd_to_ddm("goto 1");
    ur_ut_send_cmd_to_ddm("state");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("sids");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("autotest fc00:0:0:beef:0:0:0:2000");
    hal_arch_time_msleep(200);
    ur_ut_send_cmd_to_ddm("autotest fc08::fc");
    hal_arch_time_msleep(2000);

    ur_ut_send_cmd_to_ddm("goto 0");
    ur_ut_send_cmd_to_ddm("goto 2");
    ur_ut_send_cmd_to_ddm("autotest fc08::fc");
    hal_arch_time_msleep(2000);

    ur_ut_send_cmd_to_ddm("goto 0");
    ur_ut_send_cmd_to_ddm("stop 4");
    ur_ut_send_cmd_to_ddm("stop 3");
    ur_ut_send_cmd_to_ddm("stop 2");
    ur_ut_send_cmd_to_ddm("stop 1");
}



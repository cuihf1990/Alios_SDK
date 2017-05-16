#include <stdint.h>

#include "yunit.h"

#include "core/mesh_forwarder.h"

extern void ur_ut_send_cmd_to_ddm(const char *cmd);
extern void hal_arch_time_msleep(int ms);

void test_uradar_forwarder_case(void)
{
    ur_ut_send_cmd_to_ddm("rssi 1 2 2 2");
    ur_ut_send_cmd_to_ddm("rssi 1 3 2 2");
    ur_ut_send_cmd_to_ddm("rssi 2 3 0 0");
    ur_ut_send_cmd_to_ddm("rssi 1 4 0 0");
    ur_ut_send_cmd_to_ddm("rssi 2 4 0 0");
    ur_ut_send_cmd_to_ddm("rssi 3 4 2 2");
    ur_ut_send_cmd_to_ddm("start 1");
    hal_arch_time_msleep(5000);
    ur_ut_send_cmd_to_ddm("start 2");
    hal_arch_time_msleep(5000);
    ur_ut_send_cmd_to_ddm("goto 2");
    hal_arch_time_msleep(6000);
    ur_ut_send_cmd_to_ddm("ping fc00:0:0:beef:0:0:0:0");
    hal_arch_time_msleep(100);
    ur_ut_send_cmd_to_ddm("ping fc00:0:0:beef:0:0:0:1000");
    hal_arch_time_msleep(100);
    ur_ut_send_cmd_to_ddm("ping fc00:0:0:beef:0:0:0:1100");
    hal_arch_time_msleep(100);
    ur_ut_send_cmd_to_ddm("ping fc00:0:0:beef:0:0:0:0 90");
    hal_arch_time_msleep(100);
    ur_ut_send_cmd_to_ddm("ping fc00:0:0:beef:0:0:0:0 500");
    hal_arch_time_msleep(100);
    ur_ut_send_cmd_to_ddm("ping ff00:0:0:0:0:0:0:0 200");
    hal_arch_time_msleep(100);

    ur_ut_send_cmd_to_ddm("goto 0");
    ur_ut_send_cmd_to_ddm("start 3");
    hal_arch_time_msleep(5000);
    ur_ut_send_cmd_to_ddm("goto 3");
    ur_ut_send_cmd_to_ddm("ipaddr");

    ur_ut_send_cmd_to_ddm("goto 0");
    ur_ut_send_cmd_to_ddm("start 4");
    hal_arch_time_msleep(5000);
    ur_ut_send_cmd_to_ddm("goto 4");
    ur_ut_send_cmd_to_ddm("stop");
    ur_ut_send_cmd_to_ddm("mode MOBILE");
    ur_ut_send_cmd_to_ddm("start");
    hal_arch_time_msleep(5000);
    ur_ut_send_cmd_to_ddm("ipaddr");
    hal_arch_time_msleep(6000);
    ur_ut_send_cmd_to_ddm("ping fc00:0:0:beef:0:0:0:0");
    hal_arch_time_msleep(100);
    ur_ut_send_cmd_to_ddm("ping fc00:0:0:beef:0:0:0:1000");
    hal_arch_time_msleep(100);

    ur_ut_send_cmd_to_ddm("goto 0");
    ur_ut_send_cmd_to_ddm("goto 2");
    ur_ut_send_cmd_to_ddm("ping fc00:0:0:beef:0:0:0:c001");
    hal_arch_time_msleep(500);

    ur_ut_send_cmd_to_ddm("goto 0");
    ur_ut_send_cmd_to_ddm("stop 4");
    ur_ut_send_cmd_to_ddm("stop 3");
    ur_ut_send_cmd_to_ddm("stop 2");
    ur_ut_send_cmd_to_ddm("stop 1");

    hal_arch_time_msleep(100);
}



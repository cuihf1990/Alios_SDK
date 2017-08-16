#include "yunit.h"

#include "yos/kernel.h"
#include "umesh.h"
#include "core/topology.h"
#include "core/mesh_mgmt.h"
#include "hal/interfaces.h"
#include "hal/interface_context.h"

#include "dda_util.h"

void test_uradar_urmesh_case(void)
{
    ur_ip6_addr_t ip6addr;

    YUNIT_ASSERT(INVALID_SID == umesh_get_sid());
    YUNIT_ASSERT(UR_ERROR_NONE == umesh_start());
    check_cond_wait(umesh_get_device_state() == DEVICE_STATE_LEADER, 5);

    YUNIT_ASSERT(UR_ERROR_NONE == umesh_set_mode(MODE_MOBILE));
    YUNIT_ASSERT(MODE_MOBILE == umesh_get_mode());
    YUNIT_ASSERT(NULL != umesh_get_mac_address());
    umesh_set_mode(MODE_RX_ON);
    YUNIT_ASSERT(MODE_RX_ON == umesh_get_mode());

    memset(ip6addr.m8, 0x00, sizeof(ip6addr.m8));
    ip6addr.m8[0] = 0xff;
    ip6addr.m8[1] = 0x08;
    ip6addr.m8[6] = (umesh_get_meshnetid() >> 8);
    ip6addr.m8[7] = umesh_get_meshnetid();
    ip6addr.m8[15] = 0xfc;
    YUNIT_ASSERT(true == umesh_is_mcast_subscribed(&ip6addr));

    YUNIT_ASSERT(NULL != umesh_get_ucast_addr());
    YUNIT_ASSERT(NULL != umesh_get_mcast_addr());

    mac_address_t mac_addr;
    umesh_enable_whitelist();
    mac_addr.len = sizeof(mac_addr.addr);
    memset(mac_addr.addr, 0x00, sizeof(mac_addr.addr));
    mac_addr.addr[0] = 0x20;
    YUNIT_ASSERT(UR_ERROR_NONE == umesh_add_whitelist(&mac_addr));
    mac_addr.addr[0] = 0x30;
    YUNIT_ASSERT(UR_ERROR_NONE == umesh_add_whitelist_rssi(&mac_addr, 1));
    mac_addr.addr[0] = 0x20;
    umesh_remove_whitelist(&mac_addr);
    umesh_clear_whitelist();
    umesh_disable_whitelist();

    YUNIT_ASSERT(UR_ERROR_NONE == umesh_stop());
}



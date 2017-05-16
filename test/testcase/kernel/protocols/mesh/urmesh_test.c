#include "yunit.h"

#include "umesh.h"
#include "core/topology.h"
#include "core/mesh_mgmt.h"

extern void hal_arch_time_msleep(int ms);
void test_uradar_urmesh_case(void)
{
    ur_ip6_addr_t ip6addr;

    YUNIT_ASSERT(UR_ERROR_NONE == ur_mesh_init(NULL));
    YUNIT_ASSERT(BCAST_SID != ur_mesh_get_sid());
    YUNIT_ASSERT(UR_ERROR_NONE == ur_mesh_start());
    hal_arch_time_msleep(5000); /* wait till node become leader */
    if (mm_get_device_state() == DEVICE_STATE_LEADER) {
        ur_mesh_set_meshnetid(0x1000);
        YUNIT_ASSERT(0x1000 == ur_mesh_get_meshnetid());
    }

    ur_mesh_set_mode(MODE_MOBILE);
    YUNIT_ASSERT(MODE_MOBILE == ur_mesh_get_mode());
    YUNIT_ASSERT(NULL != ur_mesh_get_mac_address());
    ur_mesh_set_mode(MODE_RX_ON);
    YUNIT_ASSERT(MODE_RX_ON == ur_mesh_get_mode());

    memset(ip6addr.m8, 0x00, sizeof(ip6addr.m8));
    ip6addr.m8[0] = 0xff;
    ip6addr.m8[1] = 0x08;
    ip6addr.m8[15] = 0xfd;
    if (mm_get_device_state() == DEVICE_STATE_LEADER) {
        YUNIT_ASSERT(true == ur_mesh_is_mcast_subscribed(&ip6addr));
    } else {
        YUNIT_ASSERT(false == ur_mesh_is_mcast_subscribed(&ip6addr));
    }

    YUNIT_ASSERT(NULL != ur_mesh_get_ucast_addr());
    YUNIT_ASSERT(NULL != ur_mesh_get_mcast_addr());

    mac_address_t mac_addr;
    ur_mesh_enable_whitelist();
    mac_addr.len = sizeof(mac_addr.addr);
    memset(mac_addr.addr, 0x00, sizeof(mac_addr.addr));
    mac_addr.addr[0] = 0x20;
    YUNIT_ASSERT(UR_ERROR_NONE == ur_mesh_add_whitelist(&mac_addr));
    mac_addr.addr[0] = 0x30;
    YUNIT_ASSERT(UR_ERROR_NONE == ur_mesh_add_whitelist_rssi(&mac_addr, 1));
    mac_addr.addr[0] = 0x20;
    ur_mesh_remove_whitelist(&mac_addr);
    ur_mesh_clear_whitelist();
    ur_mesh_disable_whitelist();

    YUNIT_ASSERT(UR_ERROR_NONE == ur_mesh_stop());
}



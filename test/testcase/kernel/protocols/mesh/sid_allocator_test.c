#include "yunit.h"

#include "umesh_types.h"
#include "core/sid_allocator.h"
#include "hal/interface_context.h"
#include "hal/interfaces.h"

void test_uradar_sid_allocator_case(void)
{
    ur_node_id_t node_id = {.ueid = {0x00, 0x02, 0x03,0x04, 0x05, 0x06, 0x07, 0x08},
                            .sid = INVALID_SID,
                            .attach_sid = 0x0000};
    uint16_t index;
    network_context_t *network;

    ur_router_register_module();
    interface_init();
    interface_start();
    network = get_default_network_context();
    network->sid = 0x0000;
    allocator_init(network);

    for(index = 1; index <= 11; index++) {
        node_id.ueid[0] += 1;
        YUNIT_ASSERT(UR_ERROR_NONE == allocate_sid(network, &node_id));
        YUNIT_ASSERT((index << 12) == node_id.sid);
    }
    YUNIT_ASSERT(11 == get_allocated_number(network));
    allocator_deinit(network);
    network->sid = 0x1000;
    allocator_init(network);
    node_id.sid = 0x1000;
    free_sid(network, node_id.sid);
    node_id.mode = MODE_RX_ON;
    YUNIT_ASSERT(UR_ERROR_NONE == allocate_sid(network, &node_id));
    YUNIT_ASSERT(0x1100 == node_id.sid);
    YUNIT_ASSERT(1 == get_allocated_number(network));

    node_id.ueid[0] += 1;
    YUNIT_ASSERT(UR_ERROR_NONE == allocate_sid(network, &node_id));

    allocator_deinit(network);
    network->sid = 0x0000;
    allocator_init(network);
    node_id.ueid[0] += 1;
    node_id.mode = MODE_MOBILE;
    YUNIT_ASSERT(UR_ERROR_NONE == allocate_sid(network, &node_id));
    YUNIT_ASSERT(0xc001 == node_id.sid);
    YUNIT_ASSERT(true == is_partial_function_sid(node_id.sid));
    YUNIT_ASSERT(1 == get_allocated_pf_number(network));
    YUNIT_ASSERT(UR_ERROR_NONE == allocate_sid(network, &node_id));
    YUNIT_ASSERT(0xc001 == node_id.sid);
    YUNIT_ASSERT(true == is_partial_function_sid(node_id.sid));
    YUNIT_ASSERT(1 == get_allocated_pf_number(network));
    node_id.ueid[0] += 1;
    node_id.mode = MODE_MOBILE;
    YUNIT_ASSERT(UR_ERROR_NONE == allocate_sid(network, &node_id));
    YUNIT_ASSERT(0xc002 == node_id.sid);
    YUNIT_ASSERT(true == is_partial_function_sid(node_id.sid));
    YUNIT_ASSERT(2 == get_allocated_pf_number(network));

    allocator_deinit(network);
    interface_stop();
}

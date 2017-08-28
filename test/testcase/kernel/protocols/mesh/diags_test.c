#include <yunit.h>
#include <yos/framework.h>
#include <yos/kernel.h>

#include "core/mesh_mgmt.h"
#include "tools/diags.h"
#include "hal/interfaces.h"

void test_diags_case(void)
{
    network_context_t *network;
    ur_addr_t dest;
    message_t *message;
    mm_header_t *mm_header;
    mm_timestamp_tv_t *timestamp;
    message_info_t *info;
    uint8_t *data;
    uint16_t length;

    interface_start();
    network = get_default_network_context();
    dest.netid = 0x100;
    dest.addr.len = SHORT_ADDR_SIZE;
    dest.addr.short_addr = 0x1000;
    YUNIT_ASSERT(UR_ERROR_NONE == send_trace_route_request(network, &dest));

    length = sizeof(mm_header_t) + sizeof(mm_timestamp_tv_t);
    message = message_alloc(length, UT_MSG);
    if (message == NULL) {
        return;
    }
    data = message_get_payload(message);
    info = message->info;
    data += set_mm_header_type(info, data, COMMAND_TRACE_ROUTE_RESPONSE);

    timestamp = (mm_timestamp_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)timestamp, TYPE_TIMESTAMP);
    timestamp->timestamp = 10;
    data += sizeof(mm_timestamp_tv_t);

    info->network = network;
    memcpy(&info->dest, &dest, sizeof(info->dest));

    YUNIT_ASSERT(UR_ERROR_NONE == handle_diags_command(message, true));
    message_free(message);

    length = sizeof(mm_header_t) + sizeof(mm_timestamp_tv_t);
    message = message_alloc(length, UT_MSG);
    if (message == NULL) {
        return;
    }
    data = message_get_payload(message);
    info = message->info;
    data += set_mm_header_type(info, data, COMMAND_TRACE_ROUTE_REQUEST);

    timestamp = (mm_timestamp_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)timestamp, TYPE_TIMESTAMP);
    timestamp->timestamp = ur_get_now();
    data += sizeof(mm_timestamp_tv_t);

    info->network = network;
    memcpy(&info->dest, &dest, sizeof(info->dest));

    YUNIT_ASSERT(UR_ERROR_NONE == handle_diags_command(message, true));
    message_free(message);
    interface_stop();
}

#include "yunit.h"

#include "yos/framework.h"
#include "yos/kernel.h"

#include "core/mesh_mgmt.h"

#include "dda_util.h"

extern void topo_test_function(uint16_t first_node, uint16_t num, uint32_t timeout);
void test_uradar_scales_5nodes_case(void)
{
    topo_test_function(11, 5, 60);
}


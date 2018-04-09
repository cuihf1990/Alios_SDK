src = ['board_init.c', 'net/ethernetif.c']

component = aos_arch_component('board_hobbit1_2', src)

component.add_global_includes('include')
component.add_component_dependencis('platform/mcu/csky')
component.add_global_macros('STDIO_UART=0')
component.add_global_macros('MBEDTLS_AES_ROM_TABLES=1')

# component.set('MODULE', 'HOBBIT1_2')
# component.set('HOST_CHIP', 'hobbit1_2')
component.set_global_arch('ck802')
component.set_global_mcu_family('csky')
component.add_global_cflags('-std=gnu99')

ld_files = []
ld_files.append('gcc_csky.ld')
for ld in ld_files:
    component.add_global_ld_file(ld)

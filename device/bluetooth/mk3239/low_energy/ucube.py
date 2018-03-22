src =Split(''' 
    ../BTE_platform/mico_bt_bus.c
    ../BTE_platform/mico_bt_hcd.c
    ../BTE_platform/mico_bt_logmsg.c
    ../BTE_platform/mico_bt_nvram_access.c
    ../BTE_platform/mico_upio.c
''')
component =aos_arch_component('lib_ble_low_energy', src)

dependencis =Split(''' 
    ./device/bluetooth/mk3239/firmware
''')
for i in dependencis:
    component.add_component_dependencis(i)

global_includes =Split(''' 
    .
    ../include
''')
for i in global_includes:
    component.add_global_includes(i)

aos_global_config.set_aos_global_config('BLUETOOTH_LIB_TYPE','low_energy')
BLUETOOTH_LIB_TYPE = aos_global_config.get_aos_global_config('BLUETOOTH_LIB_TYPE')
HOST_ARCH = aos_global_config.get_aos_global_config('HOST_ARCH')
TOOLCHAIN_NAME = aos_global_config.get_aos_global_config('TOOLCHAIN_NAME')
component.add_prebuilt_lib('BTE_'+BLUETOOTH_LIB_TYPE+'.'+HOST_ARCH+'.'+TOOLCHAIN_NAME+'.release.a')

aos_global_config.set_aos_global_config('VALID_PLATFORMS',['MK3238','MK3239'])


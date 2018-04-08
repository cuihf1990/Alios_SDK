src =Split(''' 
    hal/gps_hal.c
    parse/gps_parse.c
''')
component =aos_component('gps', src)

dependencis =Split(''' 
    device/sal
    framework/atparser
''')
for i in dependencis:
    component.add_component_dependencis(i)

global_includes =Split(''' 
    .
''')
for i in global_includes:
    component.add_global_includes(i)

global_macros =Split(''' 
    AOS_GPS
''')
for i in global_macros:
    component.add_global_macro(i)

includes =Split(''' 
    ../../utility/iotx-utils/sdk-impl
    ../../framework/atparser
    ./hal
    ./parse
''')
for i in includes:
    component.add_includes(i)


cflags =Split(''' 
    -Wall
    -Werror
''')
for i in cflags:
    component.add_cflags(i)

module =  aos_global_config.get('module')
if module == 'gps.sim868':
    component.add_component_dependencis('device/sal/gprs/sim800')
    component.add_global_macro('AOS_GPS_SIM868')
    src.add_sources('drv/drv_sim868/gps_drv_simcom_sim868.c')
    src.add_sources('drv/drv_sim868/gps_parse_simcom_sim868.c')



    
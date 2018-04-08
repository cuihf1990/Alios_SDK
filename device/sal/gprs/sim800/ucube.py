src =Split(''' 
''')
component =aos_component('device_sal_sim800', src)

dependencis =Split(''' 
    kernel/yloop
''')
for i in dependencis:
    component.add_component_dependencis(i)

global_includes =Split(''' 
    ./
''')
for i in global_includes:
    component.add_global_includes(i)

global_macros =Split(''' 
    DEV_SAL_SIM800
''')
for i in global_macros:
    component.add_global_macros(i)

at_adapter = aos_global_config.get('at_adapter')
if at_adapter == 1:
    component.add_component_dependencis('device/sal')
    component.add_component_dependencis('framework/atparser')
    src.add_sources('sim800.c')

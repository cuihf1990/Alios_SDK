src     = Split('''
        starterkitgui.c
''')

component = aos_component('starterkitgui', src)
component.add_component_dependencis('kernel/yloop', 'tools/cli')
component.add_global_macros('AOS_NO_WIFI')

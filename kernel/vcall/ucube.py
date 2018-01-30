src     = Split('''
        mico/mico_rhino.c
        aos/aos_rhino.c
''')

component = aos_component('vcall', src)

component.add_component_dependencis('kernel/rhino')

component.add_global_includes('mico/include')

component.add_global_macro('VCALL_RHINO')

src     = Split('''
        gcc/port_c.c
        gcc/port_s.S
''')

component = aos_component('armv5', src)
component.add_global_includes('gcc')

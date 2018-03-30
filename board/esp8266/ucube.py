src     = Split('''
        board.c
''')

component = aos_component('board_esp8266', src)
component.add_component_dependencis('platform/mcu/esp8266')

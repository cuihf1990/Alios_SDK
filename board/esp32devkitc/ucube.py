src = ['board.c']

component = aos_arch_component('board_esp32devkitc', src)
component.add_component_dependencis('platform/mcu/esp32')

if aos_global_config.get('hci_h4', 0):
    component.add_global_macro('CONFIG_BLE_HCI_H4_UART_PORT=1')


src     = ['board.c']

component = aos_arch_component('board_mk3060', src)
component.add_component_dependencis('platform/mcu/moc108')
aos_global_config.add_ld_files('memory.ld.S')


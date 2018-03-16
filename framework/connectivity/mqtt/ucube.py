src = Split('''
    mqtt_client.c
''')

MQTT_UTILS_PATH =  '../../framework/protocol/alink-ilop'

if aos_global_config.board == 'linuxhost':
    PLATFORM_MQTT = 'linux'
    #component.add_component_dependencis('utility/iotx-utils/hal/linux')
else:
    PLATFORM_MQTT = 'rhino'
    #component.add_component_dependencis('utility/iotx-utils/hal/rhino')


src.append( MQTT_UTILS_PATH+'/hal-imp/'+PLATFORM_MQTT+'/HAL_OS_'+PLATFORM_MQTT+'.c' )
src.append( MQTT_UTILS_PATH+'/hal-imp/'+PLATFORM_MQTT+'/HAL_TCP_'+PLATFORM_MQTT+'.c' )
src.append( MQTT_UTILS_PATH+'/hal-imp/tls/HAL_TLS_mbedtls.c' )

component = aos_component('mqtt', src)

dependencis = Split('''
    framework/connectivity/mqtt/MQTTPacket
    security/mbedtls
    framework/connectivity/mqtt
    utility/digest_algorithm
    framework/protocol/alink-ilop/iotkit-system
    framework/protocol/alink-ilop/sdk-encap
''')
for i in dependencis:
    component.add_component_dependencis(i)

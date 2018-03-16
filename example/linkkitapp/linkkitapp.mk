
NAME := linkkitapp

GLOBAL_DEFINES      +=  MQTT_DIRECT  ALIOT_DEBUG IOTX_DEBUG USE_LPTHREAD  FOTA_RAM_LIMIT_MODE
#CONFIG_OTA_CH = linkkit
$(NAME)_SOURCES     := linkkit-example.c linkkit_app.c linkkit_export.c lite_queue.c

#$(NAME)_COMPONENTS := protocol.linkkit protocol.alink-ilop connectivity.mqtt cjson fota netmgr framework.common  ywss4linkkit
$(NAME)_COMPONENTS := protocol.alink-ilop connectivity.mqtt fota netmgr framework.common  protocol.linkkit.cmp protocol.linkkit.dm ywss4linkkit

LWIP := 0
ifeq ($(LWIP),1)
$(NAME)_COMPONENTS  += protocols.net
no_with_lwip := 0
endif
#ifeq ($(auto_netmgr),1)
#GLOBAL_DEFINES += AUTO_NETMGR
#endif

#GLOBAL_CFLAGS += -DDEBUG


NAME := linkkitapp

GLOBAL_DEFINES      +=  MQTT_DIRECT  ALIOT_DEBUG IOTX_DEBUG USE_LPTHREAD   COAP_WITH_YLOOP

$(NAME)_SOURCES     := linkkit-example.c linkkit_app.c linkkit_export.c lite_queue.c

$(NAME)_COMPONENTS := protocol.alink-ilop connectivity.mqtt fota netmgr framework.common  protocol.linkkit.cm protocol.linkkit.dm protocol.linkkit.alcs ywss4linkkit

ifeq ($(LWIP),1)
$(NAME)_COMPONENTS  += protocols.net
no_with_lwip := 0
endif

#GLOBAL_CFLAGS += -DON_PRE2=1

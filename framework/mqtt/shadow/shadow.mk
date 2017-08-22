NAME := shadow

$(NAME)_INCLUDES := ../platform/ ../utils/misc/ ../utils/misc/ ../mqtt/ ../system-mqtt/ ../sdk-impl/ ../packages-mqtt/LITE-log/ ../packages-mqtt/LITE-utils/
$(NAME)_SOURCES := shadow.c shadow_common.c shadow_delta.c shadow_update.c

#$(NAME)_INCLUDES += ../../protocol/alink/json/

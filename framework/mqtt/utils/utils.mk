NAME := utils
$(NAME)_INCLUDES := ./misc/ ./digest/digest/ ../sdk-impl/ ../packages-mqtt/LITE-log/ ../packages-mqtt/LITE-utils/ ../system-mqtt/ ../security-mqtt/ ../import/linux/include/
$(NAME)_SOURCES := digest/utils_base64.c digest/utils_hmac.c  digest/utils_md5.c digest/utils_sha1.c 
$(NAME)_SOURCES += misc/utils_epoch_time.c misc/utils_httpc.c misc/utils_list.c misc/utils_net.c misc/utils_timer.c

NAME := ble_app_framework

ifneq (,$(filter mk3239,$(COMPONENTS)))
$(NAME)_COMPONENTS := bluetooth.mk3239.ble_app_framework_impl
else
$(NAME)_COMPONENTS := bluetooth.ble_app_framework_impl
endif

GLOBAL_INCLUDES += .

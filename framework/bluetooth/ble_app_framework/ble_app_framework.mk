NAME := ble_app_framework

$(NAME)_COMPONENTS :=
$(info Hello COMPONENTS: $(COMPONENTS))
ifeq (,$(filter %.ble_app_framework_impl,$(COMPONENTS)))
$(NAME)_COMPONENTS += bluetooth.ble_app_framework_impl
endif

GLOBAL_INCLUDES += .

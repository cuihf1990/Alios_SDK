.PHONY: download_app download kill_openocd

EXTRA_POST_BUILD_TARGETS += copy_output_for_eclipse

ifeq (download,$(findstring download,$(MAKECMDGOALS)))
OPENOCD_LOG_FILE ?= $(BUILD_DIR)/openocd_log.txt
DOWNLOAD_LOG := >> $(OPENOCD_LOG_FILE)
endif

ifeq (,$(and $(OPENOCD_PATH),$(OPENOCD_FULL_NAME)))
	$(error Path to OpenOCD has not been set using OPENOCD_PATH and OPENOCD_FULL_NAME)
endif

download_app: all_bin display_map_summary kill_openocd
	$(eval IMAGE_SIZE := $(shell $(PYTHON) $(IMAGE_SIZE_SCRIPT) $(BIN_OUTPUT_FILE)))
	$(QUIET)$(ECHO) Downloading application to partition: $(APPLICATION_FIRMWARE_PARTITION_TCL) size: $(IMAGE_SIZE) bytes... 
	$(call CONV_SLASHES, $(OPENOCD_FULL_NAME)) -s $(SOURCE_ROOT) -f $(OPENOCD_CFG_PATH)interface/$(JTAG).cfg -f $(OPENOCD_CFG_PATH)rtl8710/rtl8710.cfg -c init -c erase_chip -c "program_flash $(LINK_OUTPUT_FILE:$(LINK_OUTPUT_SUFFIX)=.all$(BIN_OUTPUT_SUFFIX)) 0" -c reset -c shutdown $(DOWNLOAD_LOG) 2>&1 && $(ECHO) Download complete && $(ECHO_BLANK_LINE) || $(ECHO) Download failed. See $(OPENOCD_LOG_FILE) for detail.

download: download_app $(if $(findstring total,$(MAKECMDGOALS)), EXT_IMAGE_DOWNLOAD,)


copy_output_for_eclipse: build_done kill_openocd
	$(QUIET)$(call MKDIR, $(BUILD_DIR)/eclipse_debug/)
	$(QUIET)$(CP) $(LINK_OUTPUT_FILE) $(BUILD_DIR)/eclipse_debug/last_built.elf

ifneq ($(MICO_OS_PATH),)
kill_openocd:
	$(KILL_OPENOCD)
else
kill_openocd:
endif
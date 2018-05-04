EXTRA_POST_BUILD_TARGETS += image_xip

TEXT_OUTPUT_FILE           :=$(LINK_OUTPUT_FILE:$(LINK_OUTPUT_SUFFIX)=.text.bin)
DATA_OUTPUT_FILE           :=$(LINK_OUTPUT_FILE:$(LINK_OUTPUT_SUFFIX)=.data.bin)

image_xip:
	$(OBJCOPY) -j .xip_image2.text -Obinary $(LINK_OUTPUT_FILE) $(TEXT_OUTPUT_FILE)
	$(OBJCOPY) -j .ram_image2.entry -j .ram_image2.text -j .ram_image2.data -Obinary $(LINK_OUTPUT_FILE) $(DATA_OUTPUT_FILE)
	$(PYTHON) platform/mcu/rtl8710bn/pick.py $(TEXT_OUTPUT_FILE) $(DATA_OUTPUT_FILE) $(BIN_OUTPUT_FILE)
include $(MAKEFILES_PATH)/aos_host_cmd.mk

CONFIG_FILE := out/$(CLEANED_BUILD_STRING)/config.mk

include $(CONFIG_FILE)

# Include all toolchain makefiles - one of them will handle the architecture
include $(MAKEFILES_PATH)/aos_toolchain_gcc.mk

.PHONY: app_display_map_summary app_build_done kernel_display_map_summary kernel_build_done

##################################
# Filenames
##################################

## APP
APP_LINK_OUTPUT_FILE          :=$(OUTPUT_DIR)/binary/$(CLEANED_BUILD_STRING).app$(LINK_OUTPUT_SUFFIX)         # out/.../helloworld@mk108.app.elf
APP_STRIPPED_LINK_OUTPUT_FILE :=$(APP_LINK_OUTPUT_FILE:$(LINK_OUTPUT_SUFFIX)=.stripped$(LINK_OUTPUT_SUFFIX))  # out/.../helloworld@mk108.app.stripped.elf
APP_BIN_OUTPUT_FILE           :=$(APP_LINK_OUTPUT_FILE:$(LINK_OUTPUT_SUFFIX)=$(BIN_OUTPUT_SUFFIX))            # out/.../helloworld@mk108.app.bin
APP_HEX_OUTPUT_FILE           :=$(APP_LINK_OUTPUT_FILE:$(LINK_OUTPUT_SUFFIX)=$(HEX_OUTPUT_SUFFIX))            # out/.../helloworld@mk108.app.hex

APP_MAP_OUTPUT_FILE           :=$(APP_LINK_OUTPUT_FILE:$(LINK_OUTPUT_SUFFIX)=.map)                            # out/.../helloworld@mk108.app.map
APP_MAP_CSV_OUTPUT_FILE       :=$(APP_LINK_OUTPUT_FILE:$(LINK_OUTPUT_SUFFIX)=_map.csv)                        # out/.../helloworld@mk108.app_map.csv

APP_LINK_OPTS_FILE        := $(OUTPUT_DIR)/binary/link_app.opts
APP_LINT_OPTS_FILE        := $(OUTPUT_DIR)/binary/lint_app.opts

## KERNEL
KERNEL_LINK_OUTPUT_FILE          :=$(OUTPUT_DIR)/binary/$(CLEANED_BUILD_STRING).kernel$(LINK_OUTPUT_SUFFIX)         # out/.../helloworld@mk108.kernel.elf
KERNEL_STRIPPED_LINK_OUTPUT_FILE :=$(KERNEL_LINK_OUTPUT_FILE:$(LINK_OUTPUT_SUFFIX)=.stripped$(LINK_OUTPUT_SUFFIX))  # out/.../helloworld@mk108.kernel.stripped.elf
KERNEL_BIN_OUTPUT_FILE           :=$(KERNEL_LINK_OUTPUT_FILE:$(LINK_OUTPUT_SUFFIX)=$(BIN_OUTPUT_SUFFIX))            # out/.../helloworld@mk108.kernel.bin
KERNEL_HEX_OUTPUT_FILE           :=$(KERNEL_LINK_OUTPUT_FILE:$(LINK_OUTPUT_SUFFIX)=$(HEX_OUTPUT_SUFFIX))            # out/.../helloworld@mk108.kernel.hex

KERNEL_MAP_OUTPUT_FILE           :=$(KERNEL_LINK_OUTPUT_FILE:$(LINK_OUTPUT_SUFFIX)=.map)                            # out/.../helloworld@mk108.kernel.map
KERNEL_MAP_CSV_OUTPUT_FILE       :=$(KERNEL_LINK_OUTPUT_FILE:$(LINK_OUTPUT_SUFFIX)=_map.csv)                        # out/.../helloworld@mk108.kernel_map.csv

KERNEL_LINK_OPTS_FILE     := $(OUTPUT_DIR)/binary/link_kernel.opts
KERNEL_LINT_OPTS_FILE     := $(OUTPUT_DIR)/binary/lint_kernel.opts

OPENOCD_LOG_FILE          ?= $(OUTPUT_DIR)/openocd_log.txt
LIBS_DIR                  := $(OUTPUT_DIR)/libraries
##################################

ifeq (,$(SUB_BUILD))
ifneq (,$(EXTRA_TARGET_MAKEFILES))
$(foreach makefile_name,$(EXTRA_TARGET_MAKEFILES),$(eval include $(makefile_name)))
endif
endif


include $(MAKEFILES_PATH)/aos_resources.mk
include $(MAKEFILES_PATH)/aos_images_download.mk

##################################
# Macros
##################################

###############################################################################
# MACRO: GET_BARE_LOCATION
# Returns a the location of the given component relative to source-tree-root
# rather than from the cwd
# $(1) is component
GET_BARE_LOCATION =$(patsubst $(call ESCAPE_BACKSLASHES,$(SOURCE_ROOT))%,%,$(strip $($(1)_LOCATION)))


###############################################################################
# MACRO: BUILD_C_RULE
# Creates a target for building C language files (*.c)
# $(1) is component, $(2) is the source file
define BUILD_C_RULE
-include $(OUTPUT_DIR)/Modules/$(call GET_BARE_LOCATION,$(1))$(2:.c=.d)
$(OUTPUT_DIR)/Modules/$(call GET_BARE_LOCATION,$(1))$(2:.c=.o): $(strip $($(1)_LOCATION))$(2) $(CONFIG_FILE) $$(dir $(OUTPUT_DIR)/Modules/$(call GET_BARE_LOCATION,$(1))$(2)).d $(RESOURCES_DEPENDENCY) $(LIBS_DIR)/$(1).c_opts | $(EXTRA_PRE_BUILD_TARGETS)
	$$(if $($(1)_START_PRINT),,$(eval $(1)_START_PRINT:=1) $(QUIET)$(ECHO) Compiling $(1) )
	$(QUIET)$(CC) $(OPTIONS_IN_FILE_OPTION)$(LIBS_DIR)/$(1).c_opts -D__FILENAME__='"$$(notdir $$<)"' -o $$@ $$< $(COMPILER_SPECIFIC_STDOUT_REDIRECT)
endef

###############################################################################
# MACRO: CHECK_HEADER_RULE
# Compiles a C language header file to ensure it is stand alone complete
# $(1) is component, $(2) is the source header file
define CHECK_HEADER_RULE
$(eval $(1)_CHECK_HEADER_LIST+=$(OUTPUT_DIR)/Modules/$(strip $($(1)_LOCATION))$(2:.h=.chk) )
.PHONY: $(OUTPUT_DIR)/Modules/$(strip $($(1)_LOCATION))$(2:.h=.chk)
$(OUTPUT_DIR)/Modules/$(strip $($(1)_LOCATION))$(2:.h=.chk): $(strip $($(1)_LOCATION))$(2) $(CONFIG_FILE) $$(dir $(OUTPUT_DIR)/Modules/$(call GET_BARE_LOCATION,$(1))$(2)).d
	$(QUIET)$(ECHO) Checking header  $(2)
	$(QUIET)$(CC) -c $(YOS_SDK_CFLAGS) $(filter-out -pedantic -Werror, $($(1)_CFLAGS) $(C_BUILD_OPTIONS) ) $($(1)_INCLUDES) $($(1)_DEFINES) $(YOS_SDK_INCLUDES) $(YOS_SDK_DEFINES) -o $$@ $$<
endef

###############################################################################
# MACRO: BUILD_CPP_RULE
# Creates a target for building C++ language files (*.cpp)
# $(1) is component name, $(2) is the source file
define BUILD_CPP_RULE
-include $(OUTPUT_DIR)/Modules/$(call GET_BARE_LOCATION,$(1))$(patsubst %.cc,%.d,$(2:.cpp=.d))
$(OUTPUT_DIR)/Modules/$(call GET_BARE_LOCATION,$(1))$(patsubst %.cc,%.o,$(2:.cpp=.o)): $(strip $($(1)_LOCATION))$(2) $(CONFIG_FILE) $$(dir $(OUTPUT_DIR)/Modules/$(call GET_BARE_LOCATION,$(1))$(2)).d $(RESOURCES_DEPENDENCY) $(LIBS_DIR)/$(1).cpp_opts | $(EXTRA_PRE_BUILD_TARGETS)
	$$(if $($(1)_START_PRINT),,$(eval $(1)_START_PRINT:=1) $(ECHO) Compiling $(1))
	$(QUIET)$(CXX) $(OPTIONS_IN_FILE_OPTION)$(LIBS_DIR)/$(1).cpp_opts -o $$@ $$<  $(COMPILER_SPECIFIC_STDOUT_REDIRECT)
endef

###############################################################################
# MACRO: BUILD_S_RULE
# Creates a target for building Assembly language files (*.s & *.S)
# $(1) is component name, $(2) is the source file
define BUILD_S_RULE
$(OUTPUT_DIR)/Modules/$(call GET_BARE_LOCATION,$(1))$(strip $(patsubst %.S,%.o, $(2:.s=.o) )): $(strip $($(1)_LOCATION))$(2) $($(1)_PRE_BUILD_TARGETS) $(CONFIG_FILE) $$(dir $(OUTPUT_DIR)/Modules/$(call GET_BARE_LOCATION,$(1))$(strip $(patsubst %.S, %.o, $(2)))).d $(RESOURCES_DEPENDENCY) $(LIBS_DIR)/$(1).c_opts | $(EXTRA_PRE_BUILD_TARGETS)
	$$(if $($(1)_START_PRINT),,$(eval $(1)_START_PRINT:=1) $(ECHO) Compiling $(1))
	$(QUIET)$(CC) $(OPTIONS_IN_FILE_OPTION)$(LIBS_DIR)/$(1).c_opts -o $$@ $$< $(COMPILER_SPECIFIC_STDOUT_REDIRECT)
endef

###############################################################################
# MACRO: BUILD_COMPONENT_RULES
# Creates targets for building an entire component
# Target for the component static library is created in this macro
# Targets for source files are created by calling the macros defined above
# $(1) is component name
define BUILD_COMPONENT_RULES

$(eval LINK_LIBS +=$(if $($(1)_SOURCES),$(LIBS_DIR)/$(1).a))

ifeq ($($(1)_TYPE),kernel)
KERNEL_LINK_LIBS += $(if $($(1)_SOURCES),$(LIBS_DIR)/$(1).a)
endif

ifeq ($($(1)_TYPE),app)
APP_LINK_LIBS += $(if $($(1)_SOURCES),$(LIBS_DIR)/$(1).a)
endif

ifeq ($($(1)_TYPE),share)
KERNEL_LINK_LIBS += $(if $($(1)_SOURCES),$(LIBS_DIR)/$(1).a)
APP_LINK_LIBS += $(if $($(1)_SOURCES),$(LIBS_DIR)/$(1).a)
endif

ifeq ($($(1)_TYPE),)
APP_LINK_LIBS += $(if $($(1)_SOURCES),$(LIBS_DIR)/$(1).a)
endif

ifneq ($($(1)_PRE_BUILD_TARGETS),)
include $($(1)_MAKEFILE)
endif

# Make a list of the object files that will be used to build the static library
$(eval $(1)_LIB_OBJS := $(addprefix $(strip $(OUTPUT_DIR)/Modules/$(call GET_BARE_LOCATION,$(1))),  $(filter %.o, $($(1)_SOURCES:.cc=.o) $($(1)_SOURCES:.cpp=.o) $($(1)_SOURCES:.c=.o) $($(1)_SOURCES:.s=.o) $($(1)_SOURCES:.S=.o)))  $(patsubst %.c,%.o,$(call RESOURCE_FILENAME, $($(1)_RESOURCES))))


$(LIBS_DIR)/$(1).c_opts: $($(1)_PRE_BUILD_TARGETS) $(CONFIG_FILE) | $(LIBS_DIR)
	$(QUIET)$$(call WRITE_FILE_CREATE, $$@, $(subst $(COMMA),$$(COMMA), $(COMPILER_SPECIFIC_COMP_ONLY_FLAG) $(COMPILER_SPECIFIC_DEPS_FLAG) $($(1)_CFLAGS) $($(1)_INCLUDES) $($(1)_DEFINES) $(YOS_SDK_INCLUDES) $(YOS_SDK_DEFINES)))

$(LIBS_DIR)/$(1).cpp_opts: $($(1)_PRE_BUILD_TARGETS) $(CONFIG_FILE) | $(LIBS_DIR)
	 $(QUIET)$$(call WRITE_FILE_CREATE, $$@ ,$(COMPILER_SPECIFIC_COMP_ONLY_FLAG) $(COMPILER_SPECIFIC_DEPS_FLAG) $($(1)_CXXFLAGS)  $($(1)_INCLUDES) $($(1)_DEFINES) $(YOS_SDK_INCLUDES) $(YOS_SDK_DEFINES))

#$(LIBS_DIR)/$(1).as_opts: $(CONFIG_FILE) | $(LIBS_DIR)
#	$(QUIET)$$(call WRITE_FILE_CREATE, $$@ ,$($(1)_ASMFLAGS))

$(LIBS_DIR)/$(1).ar_opts: $(CONFIG_FILE) | $(LIBS_DIR)
	$(QUIET)$$(call WRITE_FILE_CREATE, $$@ ,$($(1)_LIB_OBJS))

# Allow checking of completeness of headers
$(foreach src, $(if $(findstring 1,$(CHECK_HEADERS)), $(filter %.h, $($(1)_CHECK_HEADERS)), ),$(eval $(call CHECK_HEADER_RULE,$(1),$(src))))

# Target for build-from-source
#$(OUTPUT_DIR)/libraries/$(1).a: $$($(1)_LIB_OBJS) $($(1)_CHECK_HEADER_LIST) $(OUTPUT_DIR)/libraries/$(1).ar_opts $$(if $(YOS_BUILT_WITH_ROM_SYMBOLS),$(ROMOBJCOPY_OPTS_FILE))
$(LIBS_DIR)/$(1).a: $$($(1)_LIB_OBJS) $($(1)_CHECK_HEADER_LIST) $(OUTPUT_DIR)/libraries/$(1).ar_opts
	$(ECHO) Making $$@
	$(QUIET)$(AR) $(YOS_SDK_ARFLAGS) $(COMPILER_SPECIFIC_ARFLAGS_CREATE) $$@ $(OPTIONS_IN_FILE_OPTION)$(OUTPUT_DIR)/libraries/$(1).ar_opts

# Create targets to built the component's source files into object files
$(foreach src, $(filter %.c, $($(1)_SOURCES)),$(eval $(call BUILD_C_RULE,$(1),$(src))))
$(foreach src, $(filter %.cpp, $($(1)_SOURCES)) $(filter %.cc, $($(1)_SOURCES)),$(eval $(call BUILD_CPP_RULE,$(1),$(src))))
$(foreach src, $(filter %.s %.S, $($(1)_SOURCES)),$(eval $(call BUILD_S_RULE,$(1),$(src))))


$(eval $(1)_LINT_FLAGS +=  $(filter -D% -I%, $($(1)_CFLAGS) $($(1)_INCLUDES) $($(1)_DEFINES) $(YOS_SDK_INCLUDES) $(YOS_SDK_DEFINES) ) )
$(eval LINT_FLAGS +=  $($(1)_LINT_FLAGS) )
$(eval LINT_FILES +=  $(addprefix $(strip $($(1)_LOCATION)), $(filter %.c, $($(1)_SOURCES))) )
endef

##################################
# Processing
##################################

# Create targets for resource files
# $(info Resources: $(ALL_RESOURCES))
$(eval $(if $(ALL_RESOURCES),$(call CREATE_ALL_RESOURCE_TARGETS,$(ALL_RESOURCES))))
LINK_LIBS += $(RESOURCES_LIBRARY)

# $(info Components: $(COMPONENTS))
# Create targets for components
$(foreach comp,$(COMPONENTS),$(eval $(call BUILD_COMPONENT_RULES,$(comp))))

# Add pre-built libraries
LINK_LIBS += $(YOS_SDK_PREBUILT_LIBRARIES)

##################################
# Build rules
##################################

$(LIBS_DIR):
	$(QUIET)$(call MKDIR, $@)

# Directory dependency - causes mkdir to be called once for each directory.
%/.d:
	$(QUIET)$(call MKDIR, $(dir $@))
	$(QUIET)$(TOUCH) $(@)

##################################
## APP^KERNEL

#APP_LINK_LIBS := $(foreach lib,$(LINK_LIBS),$(if $(filter-out hal_% platform_% kernel_% board_% devices_%,$(notdir $(lib))),$(lib)))

#KERNEL_LINK_LIBS := $(foreach lib,$(LINK_LIBS),$(if $(filter hal_% lib% platform_% kernel_% board_% devices_% share_%,$(notdir $(lib))),$(lib)))

APP_LINK_LIBS += 	./security/mbedtls/lib/mk108/libmbedtls.a \
			./security/alicrypto/lib/mk108/thumb/libmbedcrypto.a \
			./security/alicrypto/lib/mk108/thumb/libalicrypto.a

KERNEL_LINK_LIBS +=	./platform/mcu/beken/librwnx.a \
			./security/mbedtls/lib/mk108/libmbedtls.a \
			./security/alicrypto/lib/mk108/thumb/libmbedcrypto.a \
			./security/alicrypto/lib/mk108/thumb/libalicrypto.a

$(warning --------------------------------------------------)
$(warning $(APP_LINK_LIBS))
$(warning --------------------------------------------------)
$(warning $(KERNEL_LINK_LIBS))
$(warning --------------------------------------------------)
$(warning $(YOS_SDK_PREBUILT_LIBRARIES))
$(warning --------------------------------------------------)
##################################
## APP

$(APP_LINK_OPTS_FILE): out/$(CLEANED_BUILD_STRING)/config.mk
#$(COMPILER_SPECIFIC_LINK_MAP) $(APP_MAP_OUTPUT_FILE) $(APP_LINK_OPTS_FILE)
	$(QUIET)$(call WRITE_FILE_CREATE, $@ ,$(YOS_SDK_LINK_SCRIPT_CMD) $(call COMPILER_SPECIFIC_LINK_MAP,$(APP_MAP_OUTPUT_FILE))  $(call COMPILER_SPECIFIC_LINK_FILES, $(YOS_SDK_LINK_FILES) $(filter %.a,$^) $(APP_LINK_LIBS)) $(YOS_SDK_LDFLAGS) $(GLOBAL_LDFLAGS_APP))

$(APP_LINT_OPTS_FILE): $(APP_LINK_LIBS)
	$(QUIET)$(call WRITE_FILE_CREATE, $@ , )
	$(QUIET)$(foreach opt,$(sort $(subst \",",$(LINT_FLAGS))) $(sort $(LINT_FILES)),$(call WRITE_FILE_APPEND, $@ ,$(opt)))

$(APP_LINK_OUTPUT_FILE): $(APP_LINK_LIBS) $(YOS_SDK_LINK_SCRIPT) $(APP_LINK_OPTS_FILE) $(LINT_DEPENDENCY) | $(EXTRA_PRE_LINK_TARGETS)
	$(QUIET)$(ECHO) Making $(notdir $@)
	$(warning $(LINKER) $@ $(OPTIONS_IN_FILE_OPTION)$(APP_LINK_OPTS_FILE) $(COMPILER_SPECIFIC_STDOUT_REDIRECT))
	$(QUIET)$(LINKER) -o  $@ $(OPTIONS_IN_FILE_OPTION)$(APP_LINK_OPTS_FILE) $(COMPILER_SPECIFIC_STDOUT_REDIRECT)
	$(QUIET)$(ECHO_BLANK_LINE)
	$(QUIET)$(call COMPILER_SPECIFIC_MAPFILE_TO_CSV,$(APP_MAP_OUTPUT_FILE),$(APP_MAP_CSV_OUTPUT_FILE))

# Stripped elf file target - Strips the full elf file and outputs to a new .stripped.elf file
$(APP_STRIPPED_LINK_OUTPUT_FILE): $(APP_LINK_OUTPUT_FILE)
	$(QUIET)$(STRIP) -o $@ $(STRIPFLAGS) $<

# Bin file target - uses objcopy to convert the stripped elf into a binary file
$(APP_BIN_OUTPUT_FILE): $(APP_STRIPPED_LINK_OUTPUT_FILE)
	$(QUIET)$(ECHO) Making $(notdir $@)
	$(QUIET)$(OBJCOPY) -O binary -R .eh_frame -R .init -R .fini -R .comment -R .ARM.attributes $< $@

$(APP_HEX_OUTPUT_FILE): $(APP_STRIPPED_LINK_OUTPUT_FILE)
	$(QUIET)$(ECHO) Making $(notdir $@)
	$(QUIET)$(OBJCOPY) -O ihex -R .eh_frame -R .init -R .fini -R .comment -R .ARM.attributes $< $@
# Linker output target - This links all component & resource libraries and objects into an output executable
# CXX is used for compatibility with C++
#$(YOS_SDK_CONVERTER_OUTPUT_FILE): $(APP_LINK_OUTPUT_FILE)
#	$(QUIET)$(ECHO) Making $(notdir $@)
#	$(QUIET)$(CONVERTER) "--ihex" "--verbose" $(APP_LINK_OUTPUT_FILE) $@

#$(YOS_SDK_FINAL_OUTPUT_FILE): $(YOS_SDK_CONVERTER_OUTPUT_FILE)
#	$(QUIET)$(ECHO) Making $(PYTHON_FULL_NAME) $(YOS_SDK_CHIP_SPECIFIC_SCRIPT) -i $(YOS_SDK_CONVERTER_OUTPUT_FILE) -o $(YOS_SDK_FINAL_OUTPUT_FILE)
#	$(QUIET)$(PYTHON_FULL_NAME) $(YOS_SDK_CHIP_SPECIFIC_SCRIPT) -i $(YOS_SDK_CONVERTER_OUTPUT_FILE) -o $(YOS_SDK_FINAL_OUTPUT_FILE)

app_display_map_summary: $(APP_LINK_OUTPUT_FILE) $(YOS_SDK_CONVERTER_OUTPUT_FILE) $(YOS_SDK_FINAL_OUTPUT_FILE)
	$(QUIET) $(call COMPILER_SPECIFIC_MAPFILE_DISPLAY_SUMMARY,$(APP_MAP_OUTPUT_FILE))

# Main Target - Ensures the required parts get built
# $(info Prebuild targets:$(EXTRA_PRE_BUILD_TARGETS))
# $(info $(APP_BIN_OUTPUT_FILE))
app_build_done: $(EXTRA_PRE_BUILD_TARGETS) $(APP_BIN_OUTPUT_FILE) $(APP_HEX_OUTPUT_FILE) app_display_map_summary

##################################
## KERNEL

$(KERNEL_LINK_OPTS_FILE): out/$(CLEANED_BUILD_STRING)/config.mk
#$(COMPILER_SPECIFIC_LINK_MAP) $(KERNEL_MAP_OUTPUT_FILE) $(KERNEL_LINK_OPTS_FILE)
	$(QUIET)$(call WRITE_FILE_CREATE, $@ ,$(YOS_SDK_LINK_SCRIPT_CMD) $(call COMPILER_SPECIFIC_LINK_MAP,$(KERNEL_MAP_OUTPUT_FILE))  $(call COMPILER_SPECIFIC_LINK_FILES, $(YOS_SDK_LINK_FILES) $(filter %.a,$^) $(KERNEL_LINK_LIBS)) $(YOS_SDK_LDFLAGS) $(GLOBAL_LDFLAGS_KERNEL))

$(KERNELLINT_OPTS_FILE): $(KERNEL_LINK_LIBS)
	$(QUIET)$(call WRITE_FILE_CREATE, $@ , )
	$(QUIET)$(foreach opt,$(sort $(subst \",",$(LINT_FLAGS))) $(sort $(LINT_FILES)),$(call WRITE_FILE_APPEND, $@ ,$(opt)))

$(KERNEL_LINK_OUTPUT_FILE): $(KERNEL_LINK_LIBS) $(YOS_SDK_LINK_SCRIPT) $(KERNEL_LINK_OPTS_FILE) $(LINT_DEPENDENCY) | $(EXTRA_PRE_LINK_TARGETS)
	$(QUIET)$(ECHO) Making $(notdir $@)
	$(warning $(LINKER) $@ $(OPTIONS_IN_FILE_OPTION)$(KERNEL_LINK_OPTS_FILE) $(COMPILER_SPECIFIC_STDOUT_REDIRECT))
	$(QUIET)$(LINKER) -o  $@ $(OPTIONS_IN_FILE_OPTION)$(KERNEL_LINK_OPTS_FILE) $(COMPILER_SPECIFIC_STDOUT_REDIRECT)
	$(QUIET)$(ECHO_BLANK_LINE)
	$(QUIET)$(call COMPILER_SPECIFIC_MAPFILE_TO_CSV,$(KERNEL_MAP_OUTPUT_FILE),$(KERNEL_MAP_CSV_OUTPUT_FILE))

# Stripped elf file target - Strips the full elf file and outputs to a new .stripped.elf file
$(KERNEL_STRIPPED_LINK_OUTPUT_FILE): $(KERNEL_LINK_OUTPUT_FILE)
	$(QUIET)$(STRIP) -o $@ $(STRIPFLAGS) $<

# Bin file target - uses objcopy to convert the stripped elf into a binary file
$(KERNEL_BIN_OUTPUT_FILE): $(KERNEL_STRIPPED_LINK_OUTPUT_FILE)
	$(QUIET)$(ECHO) Making $(notdir $@)
	$(QUIET)$(OBJCOPY) -O binary -R .eh_frame -R .init -R .fini -R .comment -R .ARM.attributes $< $@

$(KERNEL_HEX_OUTPUT_FILE): $(KERNEL_STRIPPED_LINK_OUTPUT_FILE)
	$(QUIET)$(ECHO) Making $(notdir $@)
	$(QUIET)$(OBJCOPY) -O ihex -R .eh_frame -R .init -R .fini -R .comment -R .ARM.attributes $< $@
# Linker output target - This links all component & resource libraries and objects into an output executable
# CXX is used for compatibility with C++
#$(YOS_SDK_CONVERTER_OUTPUT_FILE): $(KERNEL_LINK_OUTPUT_FILE)
#	$(QUIET)$(ECHO) Making $(notdir $@)
#	$(QUIET)$(CONVERTER) "--ihex" "--verbose" $(KERNEL_LINK_OUTPUT_FILE) $@

#$(YOS_SDK_FINAL_OUTPUT_FILE): $(YOS_SDK_CONVERTER_OUTPUT_FILE)
#	$(QUIET)$(ECHO) Making $(PYTHON_FULL_NAME) $(YOS_SDK_CHIP_SPECIFIC_SCRIPT) -i $(YOS_SDK_CONVERTER_OUTPUT_FILE) -o $(YOS_SDK_FINAL_OUTPUT_FILE)
#	$(QUIET)$(PYTHON_FULL_NAME) $(YOS_SDK_CHIP_SPECIFIC_SCRIPT) -i $(YOS_SDK_CONVERTER_OUTPUT_FILE) -o $(YOS_SDK_FINAL_OUTPUT_FILE)

kernel_display_map_summary: $(KERNEL_LINK_OUTPUT_FILE) $(YOS_SDK_CONVERTER_OUTPUT_FILE) $(YOS_SDK_FINAL_OUTPUT_FILE)
	$(QUIET) $(call COMPILER_SPECIFIC_MAPFILE_DISPLAY_SUMMARY,$(KERNEL_MAP_OUTPUT_FILE))

# Main Target - Ensures the required parts get built
# $(info Prebuild targets:$(EXTRA_PRE_BUILD_TARGETS))
# $(info $(KERNEL_BIN_OUTPUT_FILE))
kernel_build_done: $(EXTRA_PRE_BUILD_TARGETS) $(KERNEL_BIN_OUTPUT_FILE) $(KERNEL_HEX_OUTPUT_FILE) kernel_display_map_summary

##################################

build_done: app_build_done kernel_build_done

$(EXTRA_POST_BUILD_TARGETS): app_build_done kernel_build_done

$(BUILD_STRING): $(if $(EXTRA_POST_BUILD_TARGETS),$(EXTRA_POST_BUILD_TARGETS),build_done)

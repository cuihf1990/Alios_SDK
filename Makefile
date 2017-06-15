default: Help

export YOS_SDK_VERSION_MAJOR    :=  3
export YOS_SDK_VERSION_MINOR    :=  2
export YOS_SDK_VERSION_REVISION :=  3

export SOURCE_ROOT:=$(dir $(word $(words $(MAKEFILE_LIST)), $(MAKEFILE_LIST)))

export MAKEFILES_PATH := $(SOURCE_ROOT)/build
export SCRIPTS_PATH := $(SOURCE_ROOT)/build/scripts
MAKEFILE_TARGETS := clean  # targets used by makefile 

#define BUILD_STRING, YOS toolchain commands on different hosts
include $(MAKEFILES_PATH)/yos_host_cmd.mk

define USAGE_TEXT
Aborting due to invalid targets

Usage: make <target> [download] [run | debug] [JTAG=xxx] [total] [VERBOSE=1]
       make run

  <target>
    One each of the following mandatory [and optional] components separated by '@'
      * Application (apps in sub-directories are referenced by subdir.appname)
      * Board ($(filter-out common  include README.txt,$(notdir $(wildcard board/*))))
      * [RTOS] ($(notdir $(wildcard YOS/RTOS/*)))
      * [Network Stack] ($(notdir $(wildcard YOS/net/*)))
      * [debug | release] Building for debug or release configurations

  [download]
    Download firmware image to target platform

  [run]
    Reset and run an application on the target hardware

  [debug]
    Connect to the target platform and run the debugger

  [total]
    Build all targets related to this application and board

  [JTAG=xxx]
    JTAG interface configuration file from the tools/OpenOCD dirctory
    Default option is jlink_swd

  [VERBOSE=1]
    Shows the commands as they are being executed

  Notes
    * Component names are case sensitive
    * 'YOS', 'FreeRTOS', 'Lwip' and 'debug' are reserved component names
    * Component names MUST NOT include space or '@' characters
    * Building for debug is assumed unless '@release' is appended to the target

  Example Usage
    Build for Debug
      $> make application.wifi_uart@MK3165
      $> make bootloader@MK3165@NoRTOS

    Build, Download and Run using the default USB-JTAG programming interface
      $> make helloworld@MK3165 download run

    Build for Release
      $> make helloworld@MK3165@release

    Build, Download and Debug using command line GDB
      $> make helloworld@MK3165 download debug

    Reset and run an application on the target hardware
      $> make run

    Clean output directory
      $> make clean
endef

############################
# Extra options:
#                CHECK_HEADERS=1 : builds header files to test for their completeness
############################

OPENOCD_LOG_FILE ?= out/openocd_log.txt
DOWNLOAD_LOG := >> $(OPENOCD_LOG_FILE)

BOOTLOADER_LOG_FILE ?= out/bootloader.log
export HOST_OS
export VERBOSE
export SUB_BUILD
export OPENOCD_LOG_FILE

.PHONY: $(BUILD_STRING) main_app bootloader clean Help download total release download_dct copy_elf_for_eclipse run debug download_bootloader sflash_image .gdbinit factory_reset_dct sflash

Help: 
	$(TOOLCHAIN_HOOK_TARGETS)
	$(error $(USAGE_TEXT))

clean:
	$(QUIET)$(ECHO) Cleaning...
	$(QUIET)$(CLEAN_COMMAND)
	$(QUIET)$(RM) -rf .gdbinit
	$(QUIET)$(ECHO) Done

ifneq ($(BUILD_STRING),)
-include out/$(CLEANED_BUILD_STRING)/config.mk
# Now we know the target architecture - include all toolchain makefiles and check one of them can handle the architecture
 include $(MAKEFILES_PATH)/yos_toolchain_GCC.mk

out/$(CLEANED_BUILD_STRING)/config.mk: $(SOURCE_ROOT)Makefile $(MAKEFILES_PATH)/yos_target_config.mk $(MAKEFILES_PATH)/yos_host_cmd.mk $(MAKEFILES_PATH)/yos_toolchain_GCC.mk $(YOS_SDK_MAKEFILES)
	$(QUIET)$(ECHO) $(if $(YOS_SDK_MAKEFILES),Applying changes made to: $?,Making config file for first time)
	$(QUIET)$(MAKE) -r $(SILENT) -f $(MAKEFILES_PATH)/yos_target_config.mk $(CLEANED_BUILD_STRING)
endif


JOBS ?=4
ifeq (,$(SUB_BUILD))
JOBSNO := -j$(JOBS)
endif


PASSDOWN_TARGETS := $(strip $(filter-out $(MAKEFILE_TARGETS) $(BUILD_STRING),$(MAKECMDGOALS))) #download total
$(PASSDOWN_TARGETS):
	@:

$(BUILD_STRING): main_app $(if $(SFLASH),sflash_image) copy_elf_for_eclipse  $(if $(SUB_BUILD),,.gdbinit .openocd_cfg)

main_app: out/$(CLEANED_BUILD_STRING)/config.mk $(YOS_SDK_PRE_APP_BUILDS) $(MAKEFILES_PATH)/yos_target_build.mk
	$(QUIET)$(COMMON_TOOLS_PATH)mkdir -p $(OUTPUT_DIR)/binary $(OUTPUT_DIR)/modules $(OUTPUT_DIR)/libraries $(OUTPUT_DIR)/resources
	$(QUIET)$(MAKE) -r $(JOBSNO) $(SILENT) -f $(MAKEFILES_PATH)/yos_target_build.mk $(CLEANED_BUILD_STRING) $(PASSDOWN_TARGETS)
	$(QUIET)$(ECHO) Build complete

ifeq ($(SUB_BUILD),)
.gdbinit: out/$(CLEANED_BUILD_STRING)/config.mk $(MAKEFILES_PATH)/yos_host_cmd.mk main_app
	$(QUIET)$(ECHO) Making $@
	$(QUIET)$(ECHO) set remotetimeout 20 > $@
	$(QUIET)$(ECHO) $(GDBINIT_STRING) >> $@
	
.openocd_cfg: .gdbinit
	$(QUIET)$(ECHO) Making $@
	$(QUIET)$(ECHO) source [find $(OPENOCD_PATH)$(JTAG).cfg] > $@
	$(QUIET)$(ECHO) source [find $(OPENOCD_PATH)$(HOST_OPENOCD).cfg] >> $@
	$(QUIET)$(ECHO) source [find $(OPENOCD_PATH)$(HOST_OPENOCD)_gdb_jtag.cfg] >> $@
	
endif

ifneq ($(SFLASH),)
sflash_image: main_app
	$(QUIET)$(ECHO) Building Serial Flash Image
	$(QUIET)$(MAKE) $(SILENT) -f $(MAKEFILES_PATH)/mfg_image.mk $(SFLASH) FRAPP=$(CLEANED_BUILD_STRING) SFLASH=
endif


sflash: main_app
	$(QUIET)$(ECHO) Building Serial Flash Image $@
	$(QUIET)$(MAKE) $(SILENT) -f $(MAKEFILES_PATH)/yos_sflash.mk FRAPP=$(CLEANED_BUILD_STRING) $@

sflash_download: main_app sflash
	$(QUIET)$(ECHO) Downloading Serial Flash Image $@
	$(QUIET)$(MAKE) $(SILENT) -f $(MAKEFILES_PATH)/yos_sflash.mk FRAPP=$(CLEANED_BUILD_STRING) $@


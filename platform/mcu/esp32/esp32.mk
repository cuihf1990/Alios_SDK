#xtensa-esp32-elf-gcc -nostdlib -u call_user_start_cpu0  -Wl,--gc-sections -Wl,-static -Wl,--start-group  -L/home/cai/esp-idf/examples/wifi/iperf/build/app_trace -lapp_trace -L/home/cai/esp-idf/examples/wifi/iperf/build/app_update -lapp_update -L/home/cai/esp-idf/examples/wifi/iperf/build/aws_iot  -L/home/cai/esp-idf/examples/wifi/iperf/build/bootloader_support -lbootloader_support -L/home/cai/esp-idf/examples/wifi/iperf/build/bt -lbt -L/home/cai/esp-idf/examples/wifi/iperf/build/coap -lcoap -L/home/cai/esp-idf/examples/wifi/iperf/build/components -lcomponents -L/home/cai/esp-idf/examples/wifi/iperf/build/console -lconsole -L/home/cai/esp-idf/examples/wifi/iperf/build/cxx -lcxx -u __cxa_guard_dummy -L/home/cai/esp-idf/examples/wifi/iperf/build/driver -ldriver -L/home/cai/esp-idf/examples/wifi/iperf/build/esp32 -lesp32 /home/cai/esp-idf/components/esp32/libhal.a -L/home/cai/esp-idf/components/esp32/lib -lcore -lrtc -lnet80211 -lpp -lwpa -lsmartconfig -lcoexist -lwps -lwpa2 -lespnow -lphy -L /home/cai/esp-idf/components/esp32/ld -T esp32_out.ld -u ld_include_panic_highint_hdl -T esp32.common.ld -T esp32.rom.ld -T esp32.peripherals.ld -T esp32.rom.spiram_incompatible_fns.ld -L/home/cai/esp-idf/examples/wifi/iperf/build/esp_adc_cal -lesp_adc_cal -L/home/cai/esp-idf/examples/wifi/iperf/build/ethernet -lethernet -L/home/cai/esp-idf/examples/wifi/iperf/build/expat -lexpat -L/home/cai/esp-idf/examples/wifi/iperf/build/fatfs -lfatfs -L/home/cai/esp-idf/examples/wifi/iperf/build/freertos -lfreertos -Wl,--undefined=uxTopUsedPriority -L/home/cai/esp-idf/examples/wifi/iperf/build/heap -lheap -L/home/cai/esp-idf/examples/wifi/iperf/build/jsmn -ljsmn -L/home/cai/esp-idf/examples/wifi/iperf/build/json -ljson -L/home/cai/esp-idf/examples/wifi/iperf/build/libsodium -llibsodium -L/home/cai/esp-idf/examples/wifi/iperf/build/log -llog -L/home/cai/esp-idf/examples/wifi/iperf/build/lwip -llwip -L/home/cai/esp-idf/examples/wifi/iperf/build/main -lmain -L/home/cai/esp-idf/examples/wifi/iperf/build/mbedtls -lmbedtls -L/home/cai/esp-idf/examples/wifi/iperf/build/mdns -lmdns -L/home/cai/esp-idf/examples/wifi/iperf/build/micro-ecc -lmicro-ecc -L/home/cai/esp-idf/examples/wifi/iperf/build/newlib /home/cai/esp-idf/components/newlib/lib/libc.a /home/cai/esp-idf/components/newlib/lib/libm.a -lnewlib -L/home/cai/esp-idf/examples/wifi/iperf/build/nghttp -lnghttp -L/home/cai/esp-idf/examples/wifi/iperf/build/nvs_flash -lnvs_flash -L/home/cai/esp-idf/examples/wifi/iperf/build/openssl -lopenssl -L/home/cai/esp-idf/examples/wifi/iperf/build/pthread -lpthread -L/home/cai/esp-idf/examples/wifi/iperf/build/sdmmc -lsdmmc -L/home/cai/esp-idf/examples/wifi/iperf/build/soc -lsoc -L/home/cai/esp-idf/examples/wifi/iperf/build/spi_flash -lspi_flash -L/home/cai/esp-idf/examples/wifi/iperf/build/spiffs -lspiffs -L/home/cai/esp-idf/examples/wifi/iperf/build/tcpip_adapter -ltcpip_adapter -L/home/cai/esp-idf/examples/wifi/iperf/build/ulp -lulp -L/home/cai/esp-idf/examples/wifi/iperf/build/vfs -lvfs -L/home/cai/esp-idf/examples/wifi/iperf/build/wear_levelling -lwear_levelling -L/home/cai/esp-idf/examples/wifi/iperf/build/wpa_supplicant -lwpa_supplicant -L/home/cai/esp-idf/examples/wifi/iperf/build/xtensa-debug-module -lxtensa-debug-module -lgcc -lstdc++ -lgcov -Wl,--end-group -Wl,-EL -o /home/cai/esp-idf/examples/wifi/iperf/build/iperf.elf -Wl,-Map=/home/cai/esp-idf/examples/wifi/iperf/build/iperf.map

HOST_OPENOCD := esp32

NAME := esp32

$(NAME)_COMPONENTS := framework.common

GLOBAL_CFLAGS    += -I $(IDF_PATH)/components/freertos/include
GLOBAL_CFLAGS    += -I $(IDF_PATH)/components/soc/esp32/include
GLOBAL_CFLAGS    += -I $(IDF_PATH)/components/soc/include
GLOBAL_CFLAGS    += -I $(IDF_PATH)/components/driver/include
GLOBAL_CFLAGS    += -I $(IDF_PATH)/components/heap/include
GLOBAL_INCLUDES  += system/include ../../arch/xtensa
GLOBAL_CFLAGS    += -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls
GLOBAL_LDFLAGS   += -nostdlib -Lplatform/mcu/esp32/ -lc #-Wl,--start-group
#GLOBAL_LDFLAGS   += -llog -lesp32 -lrtc -lsoc -lfreertos -lhal -lc -lspi_flash -lspiffs #-lstdcc++-cache-workaround
#GLOBAL_LDFLAGS   += -lheap -ldriver -lcore -ldriver -lnewlib -lvfs -lpthread -lc_nano -ltcpip_adapter -llwip
#GLOBAL_LDFLAGS   += -lwpa_supplicant -lmbedtls -lconsole -lcxx -lnet80211 -lphy -lpp -lnvs_flash -lcoexist -lwpa
GLOBAL_LDFLAGS   += -lgcc -lstdc++ -lgcov -lm #-lmain
#GLOBAL_LDFLAGS   += #-Wl,--end-group

GLOBAL_LDS_FILES += platform/mcu/esp32/system/ld/esp32.ld.S
GLOBAL_LDS_FILES += platform/mcu/esp32/system/ld/esp32.common.ld
GLOBAL_LDS_FILES += platform/mcu/esp32/system/ld/esp32.rom.ld
GLOBAL_LDS_FILES += platform/mcu/esp32/system/ld/esp32.peripherals.ld
GLOBAL_LDS_FILES += platform/mcu/esp32/system/ld/esp32.rom.spiram_incompatible_fns.ld
GLOBAL_LDFLAGS   += -L platform/mcu/esp32/system/ld

$(NAME)_SOURCES  := bsp/entry.c bsp/hal.c #system/cpu_start.c
$(NAME)_INCLUDES := soc/include soc/esp32/include
$(NAME)_CFLAGS   := -std=gnu99

libs := $(wildcard platform/mcu/esp32/lib/lib*.a)
libs := $(foreach lib,$(libs),lib/$(notdir $(lib)))
$(NAME)_PREBUILT_LIBRARY := $(libs)

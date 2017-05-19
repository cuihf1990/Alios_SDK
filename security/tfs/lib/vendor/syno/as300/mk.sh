xtensa-lx106-elf-gcc -Os -g -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals -ffunction-sections -fdata-sections  -DICACHE_FLASH -D_POSIX_SOURCE -DLWIP_OPEN_SRC -DPBUF_RSV_FOR_WLAN -DEBUF_LWIP -DCONFIG_AS608 -DCONFIG_AS300 -DCONFIG_SYNO_FINER  -I  ../../../include -o ./syno_tfshal_spi.o -c syno_tfshal_spi.c

xtensa-lx106-elf-ar  -r libtfshal.a -o ./syno_tfshal_spi.o 

openocd -f build/OpenOCD/interface/jlink_swd.cfg -f build/OpenOCD/rtl8710/rtl8710.cfg -c init -c "reset halt" -c "dump_image dump.bin 0x08000000 0x100000" -c shutdown
dd if=dump.bin of=boot_all.bin bs=1k count=8
dd if=dump.bin of=system.bin bs=1k count=4 skip=36
dd if=dump.bin of=image.bin bs=1k count=600 skip=44
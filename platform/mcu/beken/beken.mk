#
#  UNPUBLISHED PROPRIETARY SOURCE CODE
#  Copyright (c) 2016 MXCHIP Inc.
#
#  The contents of this file may not be disclosed to third parties, copied or
#  duplicated in any form, in whole or in part, without the prior written
#  permission of MXCHIP Corporation.
#

NAME := beken

HOST_OPENOCD := beken

$(NAME)_COMPONENTS += platform/arch/arm/armv5
$(NAME)_COMPONENTS += platform/mcu/beken/hal
$(NAME)_COMPONENTS := hal vflash netmgr framework mbedtls cjson cli

GLOBAL_CFLAGS += -mcpu=arm968e-s \
                 -march=armv5te \
                 -marm \
                 -mlittle-endian

GLOBAL_CFLAGS += -w

GLOBAL_INCLUDES += ../../arch/arm/armv5

GLOBAL_INCLUDES += beken7231/beken378/func/mxchip/lwip-2.0.2/port \
                   beken7231/beken378/common \
                   beken7231/beken378/driver/include \
                   beken7231/beken378/driver/common \
                   beken7231/beken378/driver/entry \
                   beken7231/beken378/ip/common \
                   beken7231/beken378/os/FreeRTOSv9.0.0/FreeRTOS/Source/portable/Keil/ARM968es

ifneq ($(mico_lwip), 1)
$(NAME)_COMPONENTS += kernel.protocols.net
else
GLOBAL_INCLUDES += beken7231/beken378/func/mxchip/lwip-2.0.2/src/include
endif

GLOBAL_LDFLAGS += -mcpu=arm968e-s \
                 -march=armv5te \
                 -marm \
                 -mlittle-endian \
                 --specs=nosys.specs \
                 -nostartfiles

GLOBAL_LDFLAGS += -T platform/mcu/beken/beken7231/beken378/build/bk7231.ld

GLOBAL_LDFLAGS += -Wl,-wrap,_malloc_r -Wl,-wrap,free -Wl,-wrap,realloc -Wl,-wrap,malloc -Wl,-wrap,calloc -Wl,-wrap,_free_r -Wl,-wrap,_realloc_r 
$(NAME)_INCLUDES := beken7231/beken378/ip/common \
                    beken7231/beken378/func/rf_test \
                    beken7231/mico_api \
                    beken7231/beken378/func/user_driver \
                    beken7231/mico_api \
                    beken7231/mico_api/MiCODrivers \
                    beken7231/beken378/func/power_save \
                    beken7231/beken378/ip/lmac/src/hal \
                    beken7231/beken378/ip/lmac/src/mm \
                    beken7231/beken378/driver/common/reg \
                    beken7231/beken378/ip/lmac/src/ps \
                    beken7231/beken378/ip/lmac/src/rd \
                    beken7231/beken378/ip/lmac/src/rwnx \
                    beken7231/beken378/ip/lmac/src/rx \
                    beken7231/beken378/ip/lmac/src/scan \
                    beken7231/beken378/ip/lmac/src/sta \
                    beken7231/beken378/ip/lmac/src/tx \
                    beken7231/beken378/ip/lmac/src/vif \
                    beken7231/beken378/ip/lmac/src/rx/rxl \
                    beken7231/beken378/ip/lmac/src/tx/txl \
                    beken7231/beken378/ip/umac/src/bam \
                    beken7231/beken378/ip/umac/src/llc \
                    beken7231/beken378/ip/umac/src/me \
                    beken7231/beken378/ip/umac/src/rxu \
                    beken7231/beken378/ip/umac/src/scanu \
                    beken7231/beken378/ip/umac/src/sm \
                    beken7231/beken378/ip/umac/src/txu \
                    beken7231/beken378/ip/ke \
                    beken7231/beken378/ip/mac \
                    beken7231/beken378/driver/sdio \
                    beken7231/beken378/driver/common \
                    beken7231/beken378/driver/include \
                    beken7231/beken378/driver/uart \
                    beken7231/beken378/driver/sys_ctrl \
                    beken7231/beken378/func/sdio_intf \
                    beken7231/beken378/driver/gpio \
                    beken7231/beken378/ip/lmac/src/p2p \
                    beken7231/beken378/ip/umac/src/apm \
                    beken7231/beken378/func/include \
                    beken7231/beken378/app/config \
                    beken7231/beken378/driver/sdcard \
                    beken7231/beken378/common \
                    beken7231/beken378/ip/lmac/src/chan \
                    beken7231/beken378/ip/lmac/src/td \
                    beken7231/beken378/driver/common/reg \
                    beken7231/beken378/driver/entry \
                    beken7231/beken378/driver/dma \
                    beken7231/beken378/driver/intc \
                    beken7231/beken378/driver/phy \
                    beken7231/beken378/driver/rc_beken \
                    beken7231/beken378/func/sd_music \
                    beken7231/beken378/func/hostapd-2.5/src/utils \
                    beken7231/beken378/func/hostapd-2.5/src \
                    beken7231/beken378/func/hostapd-2.5/bk_patch \
                    beken7231/beken378/func/hostapd-2.5/src/ap \
                    beken7231/beken378/app/standalone-ap \
                    beken7231/beken378/func/hostapd-2.5/hostapd \
                    beken7231/beken378/func/ethernet_intf \
                    beken7231/beken378/app/standalone-station \
                    beken7231/beken378/func/hostapd-2.5/src/common \
                    beken7231/beken378/func/hostapd-2.5/src/drivers \
                    beken7231/beken378/driver/usb/src/systems/none \
                    beken7231/beken378/driver/usb/src/systems/none/afs \
                    beken7231/beken378/driver/usb/include \
                    beken7231/beken378/driver/usb/src/cd \
                    beken7231/beken378/driver/usb/src/drivers/msd \
                    beken7231/beken378/driver/usb/include/class \
                    beken7231/beken378/app/net_work \
                    beken7231/beken378/driver/usb/src/msc \
                    beken7231/beken378/driver/usb \
                    beken7231/beken378/driver/usb/src/hid \
                    beken7231/beken378/driver/usb/src/drivers/hid \
                    beken7231/beken378/driver/usb/src/uvc \
                    beken7231/beken378/func/temp_detect \
                    beken7231/beken378/ip/lmac/src/tpc \
                    beken7231/beken378/ip/lmac/src/tdls \
                    beken7231/beken378/ip/umac/src/mesh \
                    beken7231/beken378/ip/umac/src/rc \
                    beken7231/beken378/func/spidma_intf \
                    beken7231/beken378/driver/general_dma \
                    beken7231/beken378/driver/spidma \
                    beken7231/beken378/os/include \
                    beken7231/beken378/func/rwnx_intf \
                    beken7231/beken378/app \
                    beken7231/beken378/app/ftp \
                    beken7231/beken378/app/led \
                    beken7231/beken378/os/FreeRTOSv9.0.0/FreeRTOS/Source/portable/Keil/ARM968es \
                    beken7231/beken378/os/FreeRTOSv9.0.0/FreeRTOS/Source/include \
                    beken7231/beken378/os/FreeRTOSv9.0.0

$(NAME)_SOURCES :=  beken7231/beken378/app/app.c \
                    beken7231/beken378/app/config/param_config.c \
                    beken7231/beken378/app/ftp/ftpd.c \
                    beken7231/beken378/app/ftp/vfs.c \
                    beken7231/beken378/app/led/app_led.c \
                    beken7231/beken378/app/net_work/app_lwip_tcp.c \
                    beken7231/beken378/app/net_work/app_lwip_udp.c \
                    beken7231/beken378/app/standalone-ap/sa_ap.c \
                    beken7231/beken378/app/standalone-station/sa_station.c \
                    beken7231/beken378/demo/ieee802_11_demo.c \
                    beken7231/beken378/driver/common/dd.c \
                    beken7231/beken378/driver/common/drv_model.c \
                    beken7231/beken378/driver/dma/dma.c \
                    beken7231/beken378/driver/driver.c \
                    beken7231/beken378/driver/entry/arch_main.c \
                    beken7231/beken378/driver/entry/boot_handlers.S \
                    beken7231/beken378/driver/entry/boot_vectors.S \
                    beken7231/beken378/driver/entry/ll.S \
                    beken7231/beken378/driver/fft/fft.c \
                    beken7231/beken378/driver/flash/flash.c \
                    beken7231/beken378/driver/general_dma/general_dma.c \
                    beken7231/beken378/driver/gpio/gpio.c \
                    beken7231/beken378/driver/i2s/i2s.c \
                    beken7231/beken378/driver/icu/icu.c \
                    beken7231/beken378/driver/intc/intc.c \
                    beken7231/beken378/driver/irda/irda.c \
                    beken7231/beken378/driver/macphy_bypass/mac_phy_bypass.c \
                    beken7231/beken378/driver/phy/phy_trident.c \
                    beken7231/beken378/driver/pwm/pwm.c \
                    beken7231/beken378/driver/saradc/saradc.c \
                    beken7231/beken378/driver/sdcard/sdcard.c \
                    beken7231/beken378/driver/sdcard/sdio_driver.c \
                    beken7231/beken378/driver/sdio/sdio.c \
                    beken7231/beken378/driver/sdio/sdma.c \
                    beken7231/beken378/driver/sdio/sutil.c \
                    beken7231/beken378/driver/spi/spi.c \
                    beken7231/beken378/driver/spidma/spidma.c \
                    beken7231/beken378/driver/sys_ctrl/sys_ctrl.c \
                    beken7231/beken378/driver/uart/Retarget.c \
                    beken7231/beken378/driver/uart/uart.c \
                    beken7231/beken378/driver/usb/src/cd/mu_cntlr.c \
                    beken7231/beken378/driver/usb/src/cd/mu_descs.c \
                    beken7231/beken378/driver/usb/src/cd/mu_drc.c \
                    beken7231/beken378/driver/usb/src/cd/mu_fc.c \
                    beken7231/beken378/driver/usb/src/cd/mu_fun.c \
                    beken7231/beken378/driver/usb/src/cd/mu_funex.c \
                    beken7231/beken378/driver/usb/src/cd/mu_hc.c \
                    beken7231/beken378/driver/usb/src/cd/mu_hdr.c \
                    beken7231/beken378/driver/usb/src/cd/mu_hsdma.c \
                    beken7231/beken378/driver/usb/src/cd/mu_hst.c \
                    beken7231/beken378/driver/usb/src/cd/mu_list.c \
                    beken7231/beken378/driver/usb/src/cd/mu_mdr.c \
                    beken7231/beken378/driver/usb/src/cd/mu_pip.c \
                    beken7231/beken378/driver/usb/src/drivers/comm/mu_comif.c \
                    beken7231/beken378/driver/usb/src/drivers/hid/mu_hidif.c \
                    beken7231/beken378/driver/usb/src/drivers/hid/mu_hidkb.c \
                    beken7231/beken378/driver/usb/src/drivers/hid/mu_hidmb.c \
                    beken7231/beken378/driver/usb/src/drivers/msd/mu_mapi.c \
                    beken7231/beken378/driver/usb/src/drivers/msd/mu_mbot.c \
                    beken7231/beken378/driver/usb/src/drivers/msd/mu_mscsi.c \
                    beken7231/beken378/driver/usb/src/examples/msd/mu_msdfn.c \
                    beken7231/beken378/driver/usb/src/hid/usb_hid.c \
                    beken7231/beken378/driver/usb/src/lib/mu_bits.c \
                    beken7231/beken378/driver/usb/src/lib/mu_stack.c \
                    beken7231/beken378/driver/usb/src/lib/mu_stdio.c \
                    beken7231/beken378/driver/usb/src/lib/mu_strng.c \
                    beken7231/beken378/driver/usb/src/msc/usb_msd.c \
                    beken7231/beken378/driver/usb/src/systems/none/afs/board.c \
                    beken7231/beken378/driver/usb/src/systems/none/plat_uds.c \
                    beken7231/beken378/driver/usb/src/uvc/usb_uvc.c \
                    beken7231/beken378/driver/usb/src/uvc/uvc_driver.c \
                    beken7231/beken378/driver/usb/usb.c \
                    beken7231/beken378/driver/wdt/wdt.c \
                    beken7231/beken378/func/bk7011_cal/bk7011_cal.c \
                    beken7231/beken378/func/fs_fat/disk_io.c \
                    beken7231/beken378/func/fs_fat/ff.c \
                    beken7231/beken378/func/fs_fat/playmode.c \
                    beken7231/beken378/func/func.c \
                    beken7231/beken378/func/hostapd-2.5/bk_patch/ddrv.c \
                    beken7231/beken378/func/hostapd-2.5/bk_patch/signal.c \
                    beken7231/beken378/func/hostapd-2.5/bk_patch/sk_intf.c \
                    beken7231/beken378/func/hostapd-2.5/bk_patch/socket.c \
                    beken7231/beken378/func/hostapd-2.5/hostapd/main_none.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/ap_config.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/ap_drv_ops.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/ap_list.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/ap_mlme.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/authsrv.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/beacon.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/bss_load.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/dfs.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/drv_callbacks.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/eap_user_db.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/hostapd.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/hw_features.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/ieee802_11.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/ieee802_11_auth.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/ieee802_11_ht.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/ieee802_11_shared.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/ieee802_1x.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/pmksa_cache_auth.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/sta_info.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/tkip_countermeasures.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/utils.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/wmm.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/wpa_auth.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/wpa_auth_glue.c \
                    beken7231/beken378/func/hostapd-2.5/src/ap/wpa_auth_ie.c \
                    beken7231/beken378/func/hostapd-2.5/src/common/hw_features_common.c \
                    beken7231/beken378/func/hostapd-2.5/src/common/ieee802_11_common.c \
                    beken7231/beken378/func/hostapd-2.5/src/common/wpa_common.c \
                    beken7231/beken378/func/hostapd-2.5/src/crypto/aes-internal-dec.c \
                    beken7231/beken378/func/hostapd-2.5/src/crypto/aes-internal-enc.c \
                    beken7231/beken378/func/hostapd-2.5/src/crypto/aes-internal.c \
                    beken7231/beken378/func/hostapd-2.5/src/crypto/aes-unwrap.c \
                    beken7231/beken378/func/hostapd-2.5/src/crypto/aes-wrap.c \
                    beken7231/beken378/func/hostapd-2.5/src/crypto/md5-internal.c \
                    beken7231/beken378/func/hostapd-2.5/src/crypto/md5.c \
                    beken7231/beken378/func/hostapd-2.5/src/crypto/rc4.c \
                    beken7231/beken378/func/hostapd-2.5/src/crypto/sha1-internal.c \
                    beken7231/beken378/func/hostapd-2.5/src/crypto/sha1-pbkdf2.c \
                    beken7231/beken378/func/hostapd-2.5/src/crypto/sha1-prf.c \
                    beken7231/beken378/func/hostapd-2.5/src/crypto/sha1.c \
                    beken7231/beken378/func/hostapd-2.5/src/crypto/tls_none.c \
                    beken7231/beken378/func/hostapd-2.5/src/drivers/driver_beken.c \
                    beken7231/beken378/func/hostapd-2.5/src/drivers/driver_common.c \
                    beken7231/beken378/func/hostapd-2.5/src/drivers/drivers.c \
                    beken7231/beken378/func/hostapd-2.5/src/eap_common/eap_common.c \
                    beken7231/beken378/func/hostapd-2.5/src/eap_server/eap_server.c \
                    beken7231/beken378/func/hostapd-2.5/src/eap_server/eap_server_methods.c \
                    beken7231/beken378/func/hostapd-2.5/src/eapol_auth/eapol_auth_sm.c \
                    beken7231/beken378/func/hostapd-2.5/src/l2_packet/l2_packet_none.c \
                    beken7231/beken378/func/hostapd-2.5/src/rsn_supp/preauth.c \
                    beken7231/beken378/func/hostapd-2.5/src/rsn_supp/wpa.c \
                    beken7231/beken378/func/hostapd-2.5/src/rsn_supp/wpa_ie.c \
                    beken7231/beken378/func/hostapd-2.5/src/utils/common.c \
                    beken7231/beken378/func/hostapd-2.5/src/utils/eloop.c \
                    beken7231/beken378/func/hostapd-2.5/src/utils/os_none.c \
                    beken7231/beken378/func/hostapd-2.5/src/utils/wpabuf.c \
                    beken7231/beken378/func/hostapd-2.5/wpa_supplicant/ap.c \
                    beken7231/beken378/func/hostapd-2.5/wpa_supplicant/blacklist.c \
                    beken7231/beken378/func/hostapd-2.5/wpa_supplicant/bss.c \
                    beken7231/beken378/func/hostapd-2.5/wpa_supplicant/config.c \
                    beken7231/beken378/func/hostapd-2.5/wpa_supplicant/config_none.c \
                    beken7231/beken378/func/hostapd-2.5/wpa_supplicant/eap_register.c \
                    beken7231/beken378/func/hostapd-2.5/wpa_supplicant/events.c \
                    beken7231/beken378/func/hostapd-2.5/wpa_supplicant/main_supplicant.c \
                    beken7231/beken378/func/hostapd-2.5/wpa_supplicant/notify.c \
                    beken7231/beken378/func/hostapd-2.5/wpa_supplicant/wmm_ac.c \
                    beken7231/beken378/func/hostapd-2.5/wpa_supplicant/wpa_scan.c \
                    beken7231/beken378/func/hostapd-2.5/wpa_supplicant/wpa_supplicant.c \
                    beken7231/beken378/func/hostapd-2.5/wpa_supplicant/wpas_glue.c \
                    beken7231/beken378/func/hostapd_intf/hostapd_intf.c \
                    beken7231/beken378/func/misc/fake_clock.c \
                    beken7231/beken378/func/misc/target_util.c \
                    beken7231/beken378/func/mxchip/dhcpd/dhcp-server-main.c \
                    beken7231/beken378/func/mxchip/dhcpd/dhcp-server.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/port/ethernetif.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/port/net.c \
                    beken7231/beken378/func/rf_test/rx_sensitivity.c \
                    beken7231/beken378/func/rf_test/tx_evm.c \
                    beken7231/beken378/func/rwnx_intf/rw_ieee80211.c \
                    beken7231/beken378/func/rwnx_intf/rw_msdu.c \
                    beken7231/beken378/func/rwnx_intf/rw_msg_rx.c \
                    beken7231/beken378/func/rwnx_intf/rw_msg_tx.c \
                    beken7231/beken378/func/sd_music/sdcard_test.c \
                    beken7231/beken378/func/sdio_intf/sdio_intf.c \
                    beken7231/beken378/func/sdio_trans/sdio_trans.c \
                    beken7231/beken378/func/sim_uart/gpio_uart.c \
                    beken7231/beken378/func/sim_uart/pwm_uart.c \
                    beken7231/beken378/func/spidma_intf/spidma_intf.c \
                    beken7231/beken378/func/temp_detect/temp_detect.c \
                    beken7231/beken378/func/uart_debug/cmd_evm.c \
                    beken7231/beken378/func/uart_debug/cmd_help.c \
                    beken7231/beken378/func/uart_debug/cmd_reg.c \
                    beken7231/beken378/func/uart_debug/cmd_rx_sensitivity.c \
                    beken7231/beken378/func/uart_debug/command_line.c \
                    beken7231/beken378/func/uart_debug/command_table.c \
                    beken7231/beken378/func/uart_debug/udebug.c \
                    beken7231/beken378/func/usb/fusb.c \
                    beken7231/beken378/func/user_driver/BkDriverFlash.c \
                    beken7231/beken378/func/user_driver/BkDriverGpio.c \
                    beken7231/beken378/func/user_driver/BkDriverPwm.c \
                    beken7231/beken378/func/user_driver/BkDriverUart.c \
                    beken7231/beken378/func/user_driver/BkDriverWdg.c \
                    beken7231/beken378/func/wlan_ui/wlan_ui.c \
                    beken7231/beken378/ip/common/co_dlist.c \
                    beken7231/beken378/ip/common/co_list.c \
                    beken7231/beken378/ip/common/co_math.c \
                    beken7231/beken378/ip/common/co_pool.c \
                    beken7231/beken378/ip/common/co_ring.c \
                    beken7231/beken378/ip/ke/ke_env.c \
                    beken7231/beken378/ip/ke/ke_event.c \
                    beken7231/beken378/ip/ke/ke_msg.c \
                    beken7231/beken378/ip/ke/ke_queue.c \
                    beken7231/beken378/ip/ke/ke_task.c \
                    beken7231/beken378/ip/ke/ke_timer.c \
                    beken7231/beken378/ip/lmac/src/chan/chan.c \
                    beken7231/beken378/ip/lmac/src/hal/hal_desc.c \
                    beken7231/beken378/ip/lmac/src/hal/hal_dma.c \
                    beken7231/beken378/ip/lmac/src/hal/hal_machw.c \
                    beken7231/beken378/ip/lmac/src/hal/hal_mib.c \
                    beken7231/beken378/ip/lmac/src/mm/mm.c \
                    beken7231/beken378/ip/lmac/src/mm/mm_bcn.c \
                    beken7231/beken378/ip/lmac/src/mm/mm_task.c \
                    beken7231/beken378/ip/lmac/src/mm/mm_timer.c \
                    beken7231/beken378/ip/lmac/src/p2p/p2p.c \
                    beken7231/beken378/ip/lmac/src/ps/ps.c \
                    beken7231/beken378/ip/lmac/src/rd/rd.c \
                    beken7231/beken378/ip/lmac/src/rwnx/rwnx.c \
                    beken7231/beken378/ip/lmac/src/rx/rx_swdesc.c \
                    beken7231/beken378/ip/lmac/src/rx/rxl/rxl_cntrl.c \
                    beken7231/beken378/ip/lmac/src/rx/rxl/rxl_hwdesc.c \
                    beken7231/beken378/ip/lmac/src/scan/scan.c \
                    beken7231/beken378/ip/lmac/src/scan/scan_shared.c \
                    beken7231/beken378/ip/lmac/src/scan/scan_task.c \
                    beken7231/beken378/ip/lmac/src/sta/sta_mgmt.c \
                    beken7231/beken378/ip/lmac/src/td/td.c \
                    beken7231/beken378/ip/lmac/src/tdls/tdls.c \
                    beken7231/beken378/ip/lmac/src/tdls/tdls_task.c \
                    beken7231/beken378/ip/lmac/src/tpc/tpc.c \
                    beken7231/beken378/ip/lmac/src/tx/tx_swdesc.c \
                    beken7231/beken378/ip/lmac/src/tx/txl/txl_buffer.c \
                    beken7231/beken378/ip/lmac/src/tx/txl/txl_buffer_shared.c \
                    beken7231/beken378/ip/lmac/src/tx/txl/txl_cfm.c \
                    beken7231/beken378/ip/lmac/src/tx/txl/txl_cntrl.c \
                    beken7231/beken378/ip/lmac/src/tx/txl/txl_frame.c \
                    beken7231/beken378/ip/lmac/src/tx/txl/txl_frame_shared.c \
                    beken7231/beken378/ip/lmac/src/tx/txl/txl_hwdesc.c \
                    beken7231/beken378/ip/lmac/src/vif/vif_mgmt.c \
                    beken7231/beken378/ip/mac/mac.c \
                    beken7231/beken378/ip/mac/mac_ie.c \
                    beken7231/beken378/ip/umac/src/apm/apm.c \
                    beken7231/beken378/ip/umac/src/apm/apm_task.c \
                    beken7231/beken378/ip/umac/src/bam/bam.c \
                    beken7231/beken378/ip/umac/src/bam/bam_task.c \
                    beken7231/beken378/ip/umac/src/me/me.c \
                    beken7231/beken378/ip/umac/src/me/me_mgmtframe.c \
                    beken7231/beken378/ip/umac/src/me/me_mic.c \
                    beken7231/beken378/ip/umac/src/me/me_task.c \
                    beken7231/beken378/ip/umac/src/me/me_utils.c \
                    beken7231/beken378/ip/umac/src/rc/rc.c \
                    beken7231/beken378/ip/umac/src/rc/rc_basic.c \
                    beken7231/beken378/ip/umac/src/rxu/rxu_cntrl.c \
                    beken7231/beken378/ip/umac/src/scanu/scanu.c \
                    beken7231/beken378/ip/umac/src/scanu/scanu_shared.c \
                    beken7231/beken378/ip/umac/src/scanu/scanu_task.c \
                    beken7231/beken378/ip/umac/src/sm/sm.c \
                    beken7231/beken378/ip/umac/src/sm/sm_task.c \
                    beken7231/beken378/ip/umac/src/txu/txu_cntrl.c \
                    beken7231/beken378/os/mem_arch.c \
                    beken7231/beken378/os/str_arch.c \
                    beken7231/mico_api/MiCODrivers/MiCODriverFlash.c \
                    beken7231/mico_api/MiCODrivers/MiCODriverGpio.c \
                    beken7231/mico_api/MiCODrivers/MiCODriverPwm.c \
                    beken7231/mico_api/MiCODrivers/MiCODriverUart.c \
                    beken7231/mico_api/MiCODrivers/MiCODriverWdg.c \
                    beken7231/mico_api/mico_cli.c \
                    beken7231/mico_api/mico_wlan.c \
                    beken7231/mico_api/mxchipWNet.c \
                    beken7231/mico_api/platform_stub.c \
                    ../../arch/arm/armv5/port_c.c \
                    ../../arch/arm/armv5/port_s.S \
                    ../../arch/arm/armv5/soc_impl.c \

$(NAME)_SOURCES	 += hal/wdg.c \
                    hal/hw.c \
                    hal/flash.c \
					hal/uart.c \
					hal/ringbuf.c \
					hal/wifi_port.c \
                    port/ota_port.c

ifneq (,$(filter protocols.mesh,$(COMPONENTS)))
$(NAME)_SOURCES +=  beken7231/mesh_wifi_hal.c
endif

ifneq ($(mico_lwip), 1)
$(NAME)_INCLUDES += ../../../kernel/protocols/net/include/lwip \
                    ../../../kernel/protocols/net/include/netif

else
$(NAME)_INCLUDES += beken7231/beken378/func/mxchip/lwip-2.0.2/src \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/port \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/include \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/include/lwip \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/include/netif

$(NAME)_SOURCES  += beken7231/beken378/func/mxchip/lwip-2.0.2/port/sys_arch.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/api/api_lib.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/api/api_msg.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/api/err.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/api/netbuf.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/api/netdb.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/api/netifapi.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/api/sockets.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/api/tcpip.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/def.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/dns.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/inet_chksum.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/init.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ip.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv4/autoip.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv4/dhcp.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv4/etharp.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv4/icmp.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv4/igmp.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv4/ip4.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv4/ip4_addr.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv4/ip4_frag.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv6/dhcp6.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv6/ethip6.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv6/icmp6.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv6/inet6.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv6/ip6.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv6/ip6_addr.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv6/ip6_frag.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv6/mld6.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/ipv6/nd6.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/mem.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/memp.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/netif.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/pbuf.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/raw.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/stats.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/sys.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/tcp.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/tcp_in.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/tcp_out.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/timeouts.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/core/udp.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ethernet.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/lowpan6.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/auth.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/ccp.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/chap-md5.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/chap-new.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/chap_ms.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/demand.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/eap.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/ecp.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/eui64.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/fsm.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/ipcp.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/ipv6cp.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/lcp.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/magic.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/mppe.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/multilink.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/polarssl/arc4.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/polarssl/des.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/polarssl/md4.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/polarssl/md5.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/polarssl/sha1.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/ppp.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/pppapi.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/pppcrypt.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/pppoe.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/pppol2tp.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/pppos.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/upap.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/utils.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/ppp/vj.c \
                    beken7231/beken378/func/mxchip/lwip-2.0.2/src/netif/slipif.c
endif

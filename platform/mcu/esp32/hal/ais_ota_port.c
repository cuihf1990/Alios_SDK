#include <stdint.h>
#include <hal/ais_ota.h>

static flash_event_handler_t flash_handler;
static settings_event_handler_t settings_hanlder;

uint32_t ais_ota_get_setting_fw_offset()
{
    return 0;
}

void ais_ota_set_setting_fw_offset(uint32_t offset)
{

}

uint32_t ais_ota_get_page_size()
{
    return 0;
}

ali_ota_flash_err_t ais_ota_flash_erase(uint32_t const *addr, uint32_t num_pages, flash_event_handler_t cb)
{
    return ALI_OTA_FLASH_CODE_SUCCESS;
}

ali_ota_flash_err_t ais_ota_flash_store(uint32_t const *addr, uint32_t const *p_data, uint16_t len, flash_event_handler_t cb)
{
    return ALI_OTA_FLASH_CODE_SUCCESS;
}

void ais_ota_flash_init()
{

}

void ais_ota_settings_init()
{

}

uint32_t ais_ota_get_dst_addr()
{
    return 0;
}

ali_ota_settings_err_t ais_ota_settings_write(settings_event_handler_t cb)
{
    return ALI_OTA_SETTINGS_CODE_SUCCESS;
}

bool ais_ota_check_if_resume(uint8_t * p_data, uint16_t length)
{
    return 0;
}

void ais_ota_update_fw_version(uint8_t * p_data, uint16_t length)
{

}

bool ais_ota_check_if_update_finished()
{
    return 0;
}

void ais_ota_update_settings_after_update_finished()
{

}

void ais_ota_update_setting_after_xfer_finished(uint32_t img_size, uint32_t img_crc)
{

}

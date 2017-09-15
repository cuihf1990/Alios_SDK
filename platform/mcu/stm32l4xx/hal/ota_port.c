#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 

#include <errno.h>
#include <hal/ota.h>
#include <aos/log.h>
#include <hal/soc/soc.h>
#include <CheckSumUtils.h>
#include "stm32l4xx_hal_flash.h"

typedef struct
{
    uint32_t addr;
    uint32_t size;
    uint16_t crc;
} ota_hdl_t;

typedef struct
{
    uint32_t ota_len;
    uint32_t ota_crc;
} ota_reboot_info_t;

static ota_reboot_info_t ota_info;

extern int FLASH_set_boot_bank(uint32_t bank);

int hal_ota_switch_to_new_fw( int ota_data_len, uint16_t ota_data_crc )
{
    if (0 == FLASH_set_boot_bank(FLASH_BANK_BOTH)) {
        printf("Default boot bank switched successfully.\n");
        return 0;
    } else {
        printf("Error: failed changing the boot configuration\n");
        return -1;
    }

    return 0;
}

static  CRC16_Context contex;

unsigned int _off_set = 0;
static int stm32l475_ota_init(hal_ota_module_t *m, void *something)
{
    hal_logic_partition_t *partition_info;

    printf("set ota init---------------\n");
    
    partition_info = hal_flash_get_info( HAL_PARTITION_OTA_TEMP );
    hal_flash_erase(HAL_PARTITION_OTA_TEMP, 0 ,partition_info->partition_length);

    int _off_set = 0;
    CRC16_Init( &contex );
    return 0;
}


static int stm32l475_ota_write(hal_ota_module_t *m, volatile uint32_t* off_set, uint8_t* in_buf ,uint32_t in_buf_len)
{
    hal_partition_t pno = HAL_PARTITION_OTA_TEMP;
    if(ota_info.ota_len == 0) {
        _off_set = 0;
        CRC16_Init( &contex );
        memset(&ota_info, 0 , sizeof ota_info);
    }
    printf("set write len---------------%d\n", in_buf_len);
    CRC16_Update( &contex, in_buf, in_buf_len);
    if (!FLASH_bank1_enabled(FLASH_BANK_BOTH)) {
        pno = HAL_PARTITION_APPLICATION;
    }
    printf("stm32l475_ota_write: pno = %d\n", pno);
    int ret = hal_flash_write(pno, &_off_set, in_buf, in_buf_len);
    ota_info.ota_len += in_buf_len;
    printf(" ret :%d, size :%d\n", ret, ota_info.ota_len);
    return ret;
}

static int stm32l475_ota_read(hal_ota_module_t *m,  volatile uint32_t* off_set, uint8_t* out_buf, uint32_t out_buf_len)
{
    hal_flash_read(HAL_PARTITION_OTA_TEMP, off_set, out_buf, out_buf_len);
    return 0;
}

static int stm32l475_ota_set_boot(hal_ota_module_t *m, void *something)
{
    CRC16_Final( &contex, &ota_info.ota_crc );
    printf("set boot---------------\n");
    hal_ota_switch_to_new_fw(ota_info.ota_len, ota_info.ota_crc);
    memset(&ota_info, 0 , sizeof ota_info);
    return 0;
}

struct hal_ota_module_s stm32l475_ota_module = {
    .init = stm32l475_ota_init,
    .ota_write = stm32l475_ota_write,
    .ota_read = stm32l475_ota_read,
    .ota_set_boot = stm32l475_ota_set_boot,
};

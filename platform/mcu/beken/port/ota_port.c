#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 

#include <errno.h>
#include <hal/ota.h>
#include <aos/aos.h>
#include <hal/soc/soc.h>
#include <CheckSumUtils.h>

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

int hal_ota_switch_to_new_fw( int ota_data_len, uint16_t ota_data_crc )
{
    uint32_t offset;
    ota_hdl_t ota_hdl,ota_hdl_rb;
    hal_logic_partition_t* ota_partition;
    
    ota_partition = hal_flash_get_info( HAL_PARTITION_OTA_TEMP );

    memset( &ota_hdl, 0, sizeof(ota_hdl_t) );
    ota_hdl.size = ota_data_len;
    ota_hdl.addr = ota_partition->partition_start_addr;
    ota_hdl.crc = ota_data_crc;

    printf("OTA address = 0x%08x, size = 0x%08x, CRC = 0x%04x\r\n", ota_hdl.addr, ota_hdl.size, ota_hdl.crc);

    offset = 0x00;
    hal_flash_erase( HAL_PARTITION_PARAMETER_1, offset, sizeof(ota_hdl_t) );

    offset = 0x00;
    hal_flash_write( HAL_PARTITION_PARAMETER_1, &offset, (const void *)&ota_hdl, sizeof(ota_hdl_t));

    offset = 0x00;
    memset(&ota_hdl_rb, 0, sizeof(ota_hdl_t));
    hal_flash_read( HAL_PARTITION_PARAMETER_1, &offset, &ota_hdl_rb, sizeof(ota_hdl_t));

    if(memcmp(&ota_hdl, &ota_hdl_rb, sizeof(ota_hdl_t)) != 0)
    {
        printf("OTA header compare failed, OTA address = 0x%08x, size = 0x%08x, CRC = 0x%04x\r\n", ota_hdl_rb.addr, ota_hdl_rb.size, ota_hdl_rb.crc);
        return -1;
    }

    /* reboot */
    hal_reboot();

    return 0;
}

static  CRC16_Context contex;

unsigned int _off_set = 0;
static int moc108_ota_init(hal_ota_module_t *m, void *something)
{
    hal_logic_partition_t *partition_info;

    printf("set ota init---------------\n");
    
    partition_info = hal_flash_get_info( HAL_PARTITION_OTA_TEMP );
    hal_flash_erase(HAL_PARTITION_OTA_TEMP, 0 ,partition_info->partition_length);

    int _off_set = 0;
    CRC16_Init( &contex );
    return 0;
}


static int moc108_ota_write(hal_ota_module_t *m, volatile uint32_t* off_set, uint8_t* in_buf ,uint32_t in_buf_len)
{
    if(ota_info.ota_len == 0) {
        _off_set = 0;
        CRC16_Init( &contex );
        memset(&ota_info, 0 , sizeof ota_info);
    }
    printf("set write len---------------%d\n", in_buf_len);
    CRC16_Update( &contex, in_buf, in_buf_len);
    int ret = hal_flash_write(HAL_PARTITION_OTA_TEMP, &_off_set, in_buf, in_buf_len);
    ota_info.ota_len += in_buf_len;
    printf(" ret :%d, size :%d\n", ret, ota_info.ota_len);
    return ret;
}

static int moc108_ota_read(hal_ota_module_t *m,  volatile uint32_t* off_set, uint8_t* out_buf, uint32_t out_buf_len)
{
    hal_flash_read(HAL_PARTITION_OTA_TEMP, off_set, out_buf, out_buf_len);
    return 0;
}

static int moc108_ota_set_boot(hal_ota_module_t *m, void *something)
{
    CRC16_Final( &contex, &ota_info.ota_crc );
    printf("set boot---------------\n");
    hal_ota_switch_to_new_fw(ota_info.ota_len, ota_info.ota_crc);
    memset(&ota_info, 0 , sizeof ota_info);
    return 0;
}

struct hal_ota_module_s moc108_ota_module = {
    .init = moc108_ota_init,
    .ota_write = moc108_ota_write,
    .ota_read = moc108_ota_read,
    .ota_set_boot = moc108_ota_set_boot,
};

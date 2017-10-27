#include "hal/soc/soc.h"
#include "esp_spi_flash.h"

extern const hal_logic_partition_t hal_partitions[];

#define ROUND_DOWN(a,b) (((a) / (b)) * (b))

hal_logic_partition_t *hal_flash_get_info(hal_partition_t pno)
{
    hal_logic_partition_t *logic_partition;

    logic_partition = (hal_logic_partition_t *)&hal_partitions[ pno ];

    return logic_partition;
}

int32_t hal_flash_write(hal_partition_t pno, uint32_t* poff, const void* buf ,uint32_t buf_size)
{
    uint32_t addr;
    uint32_t start_addr, end_addr;
    hal_logic_partition_t *partition_info;

    partition_info = hal_flash_get_info( pno );
    start_addr = partition_info->partition_start_addr + *poff;
    if (0 != spi_flash_write(start_addr, buf, buf_size)) {
        printf("spi_flash_write failed!\n");
    }
    *poff += buf_size;
    return 0;
}

int32_t hal_flash_read(hal_partition_t pno, uint32_t* poff, void* buf, uint32_t buf_size)
{
    uint32_t start_addr;
    hal_logic_partition_t *partition_info;

    partition_info = hal_flash_get_info( pno );

    if(poff == NULL || buf == NULL || *poff + buf_size > partition_info->partition_length)
        return -1;
    start_addr = partition_info->partition_start_addr + *poff;
    spi_flash_read(start_addr, buf, buf_size);
    *poff += buf_size;

    return 0;
}

int32_t hal_flash_erase(hal_partition_t pno, uint32_t off_set,
                        uint32_t size)
{
    uint32_t addr;
    uint32_t start_addr, end_addr;
    hal_logic_partition_t *partition_info;

    partition_info = hal_flash_get_info( pno );
    if(size + off_set > partition_info->partition_length)
        return -1;

    start_addr = ROUND_DOWN((partition_info->partition_start_addr + off_set), SPI_FLASH_SEC_SIZE);

    spi_flash_erase_range(start_addr, size);
    return 0;
}

int32_t hal_flash_enable_secure(hal_partition_t partition, uint32_t off_set, uint32_t size)
{
    return 0;
}

int32_t hal_flash_dis_secure(hal_partition_t partition, uint32_t off_set, uint32_t size)
{
    return 0;
}



#include "hal/soc/soc.h"
#include "helloworld.h"

void application_start(void)
{
    uint32_t i, j;
    uint32_t offset;
    uint8_t buf[32],tmp_buf[sizeof(buf)];

    for(j = 0; j < 16; j++)
    {
        printf("erasing flash...");
        offset = 0;
        hal_flash_erase(HAL_PARTITION_PARAMETER_1, offset, 1024*4);
        printf("completed\r\n");

        printf("reading flash...");
        offset = 0;
        memset(tmp_buf, 0, sizeof(tmp_buf));
        hal_flash_read(HAL_PARTITION_PARAMETER_1, &offset, tmp_buf, sizeof(tmp_buf));
        printf("completed\r\n");

        printf("read data after erase: ");
        for(i = 0; i < sizeof(tmp_buf); i++)
        {
            printf("%02x ", tmp_buf[i]);
        }
        printf("\r\n");

        printf("writing flash...");
        offset = 0;
        memset(buf, 0x34, sizeof(buf));
        hal_flash_write(HAL_PARTITION_PARAMETER_1, &offset, buf, sizeof(buf));
        printf("completed\r\n");

        printf("reading flash...");
        offset = 0;
        memset(tmp_buf, 0, sizeof(tmp_buf));
        hal_flash_read(HAL_PARTITION_PARAMETER_1, &offset, tmp_buf, sizeof(tmp_buf));
        printf("completed\r\n");

        printf("read data after write: ");
        for(i = 0; i < sizeof(tmp_buf); i++)
        {
            printf("%02x ", tmp_buf[i]);
        }
        printf("\r\n");
    }
}


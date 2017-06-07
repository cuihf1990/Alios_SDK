/****************************************************************************
 *
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 ****************************************************************************/

/**
* @file
* @brief      dynamic loader porting interfaces
* @details
* @author     zhifang Xiu
* @date       2016-12-14
* @version    0.1
*/
#ifndef DLOAD_PORT_H_
#define DLOAD_PORT_H_

#include <stdint.h>
#include <dload_default_config.h>

/* api table */
#if defined(YUNOS_CONFIG_DLOAD_API_TABLE)
extern struct api_table_s g_api_table;
#endif


/* log interfaces */
extern int printf(const char *format, ...);

#define PRINT printf

#define LOG_D(format, ...) \
    do { \
        PRINT("[D][dload]%s-%d: " format "\n", \
            __func__,__LINE__, ##__VA_ARGS__ ); \
    } while (0)

#define LOG_E(format, ...) \
    do { \
        PRINT("[E][dload]%s-%d: " format "\n", \
                    __func__,__LINE__, ##__VA_ARGS__ ); \
    } while (0)


/* crc16 .The poly is 0x8005 (x^16 + x^15 + x^2 + 1) */
extern uint16_t crc16(const uint8_t *src, uint32_t len);

/* ram management interfaces */
extern void *dload_allocate_ram(uint32_t nbytes);
extern int   dload_free_ram(void *base, uint32_t nbytes);

/* norflash simple fs interfaces */
extern uint32_t nsfs_get_image_base (int32_t
                                     image_fd);  //get image base from image file descriptor

/* cmmu interface */
extern int cmmu_enable(uint8_t
                       en);                                      //enable cmmu functionality
extern int cmmu_map(uint32_t virt_addr, uint32_t phy_addr,
                    uint32_t len); //config cmmu mapping ,return cmmu mapping instance
extern int cmmu_unmap(int
                      inst);                                         //cmmu unmapping


#endif /* DLOAD_PORT_H_ */

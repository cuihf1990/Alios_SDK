/****************************************************************************
 *
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 ****************************************************************************/

/**
* @file
* @brief     dynamic module header
* @details
* @author    xzf
* @date      2016-11-16
* @version   0.1
*/
#ifndef YMOD_H_
#define YMOD_H_

#include <stdint.h>

typedef int32_t (*module_entry_t)(void);

/*
 * dynamic module format
--------------------------
|         header         |
--------------------------
|                        |
|   text section(xip)    |
|                        |
--------------------------
|                        |   --> first element is api table address base if YUNOS_CONFIG_DLOAD_API_TABLE
|     data section       |
|                        |
--------------------------

 */

#define YMOD_MAGIC "YM"

struct ymod_hdr {
    char     magic[2];
    uint16_t crc;
    uint16_t file_ver;
    uint16_t mod_ver;
    uint32_t uuid0;
    uint32_t uuid1;
    uint32_t text_len;
    uint32_t data_len;
    uint32_t bss_len;
    uint32_t init_base;
    uint32_t deinit_base;
};

#endif /* YMOD_H_ */


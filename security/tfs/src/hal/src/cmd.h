/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#ifndef _CMD_H
#define _CMD_H

#include <stdint.h>

/* TFS CMD */
#define CMD_GET_ID2          0X41
#define CMD_UPDATE_PRIKEY    0X42

#define CMD_RSA_GEN_KEY      0X51
#define CMD_RSA_GET_PUBKEY   0X52
#define CMD_RSA_SIGN         0X53
#define CMD_RSA_VERIFY       0X54
#define CMD_RSA_ENCRYPT      0X55
#define CMD_RSA_DECRYPT      0X56

#define CMD_3DES_ENCRYPT     0X57
#define CMD_3DES_DECRYPT     0X58

/* TFS RESPONSE*/
#define RES_OK 0X00

#define MAX_PACKAGE_SIZE 1024

/*
 * The CMD data structure:
 * ------------------------------------------------------------------
 * |  size   | 2 bytes | 1 byte | 2 bytes | 1 bytes | *** | 2 bytes |
 * ------------------------------------------------------------------
 * |  name   |  head   |  flag  | length  |   cmd   | *** |   sum   |
 * ------------------------------------------------------------------
 * | content |  EF01   |  CMD   | XXXX    |   XX    | *** |   SUM   |
 * ------------------------------------------------------------------
 */

/*
 * The RESPONSE data structure:
 * ------------------------------------------------------------------
 * |  size   | 2 bytes | 1 byte | 2 bytes | 1 bytes | *** | 2 bytes |
 * ------------------------------------------------------------------
 * |  name   |  head   |  flag  | length  |   stat  | *** |  sum    |
 * ------------------------------------------------------------------
 * | content |  EF01   |RESPONSE|   XXXX  |   XX    | *** |  SUM    |
 * ------------------------------------------------------------------
 */

int fill_package(uint8_t *package, uint32_t cmd, uint8_t *arg,
                 uint32_t arg_len);

int hal_cmd(uint32_t cmd, void *in, uint32_t in_len, void *out,
            uint32_t *out_len);

#endif /* _CMD_H */

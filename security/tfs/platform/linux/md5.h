/**
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 */

#ifndef _TFS_MD5_H
#define _TFS_MD5_H

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct MD5Context
{
  uint32_t buf[4];
  uint32_t bits[2];
  uint8_t in[64];
};

typedef struct MD5Context MD5_CTX;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, unsigned char const *buf,
               unsigned len);
void MD5Final(unsigned char digest[16], struct MD5Context *context);
void MD5Transform(uint32_t buf[4], uint32_t const in[16]);

void md5_sum(const uint8_t *addr, const size_t len, uint8_t *mac);
char *md5_hash(const uint8_t *addr, const size_t len);

#ifdef __cplusplus
}
#endif

#endif /* _TFS_MD5_H */

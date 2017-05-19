#ifndef _TFS_BASE64_H
#define _TFS_BASE64_H

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

unsigned char *base64_encode(const unsigned char *src, size_t len,
                             unsigned char *dst, size_t *out_len);
unsigned char *base64_decode(const unsigned char *src, size_t len,
                             unsigned char *dst, size_t *out_len);
unsigned char *base64w_encode(const unsigned char *src, size_t len,
                              unsigned char *dst, size_t *out_len);
unsigned char *base64w_decode(const unsigned char *src, size_t len,
                              unsigned char *dst, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* _TFS_BASE64_H */

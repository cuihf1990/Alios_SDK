#ifndef SAL_DEF_H
#define SAL_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#if BYTE_ORDER == BIG_ENDIAN
#define sal_htons(x) (x)
#define sal_ntohs(x) (x)
#define sal_htonl(x) (x)
#define sal_ntohl(x) (x)
#else
#define sal_htons(x) ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))
#define sal_ntohs(x) sal_htons(x)
#define sal_htonl(x) ((((x) & 0xff) << 24) | \
                     (((x) & 0xff00) << 8) | \
                     (((x) & 0xff0000UL) >> 8) | \
                     (((x) & 0xff000000UL) >> 24))
#define sal_ntohl(x) sal_htonl(x)
#endif

#define htons(h) sal_htons(h)

#define ntohs(n) sal_ntohs(n)

#ifdef __cplusplus
}
#endif

#endif


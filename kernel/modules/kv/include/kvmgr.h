#ifndef _key_value_h_
#define _key_value_h

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

#define HASH_TABLE_MAX_SIZE 1024

typedef struct HashNode_Struct HashNode;

struct HashNode_Struct {
    char *sKey;
    char *cValue;
    int nLength;
    int inflash;
    HashNode *pNext;
};

enum {
    INFLASH,
    INRAM,
    MEM_UNKOWN
};

#define INT2BYTE(p,value) \
    *p ++ = value & 0xff, \
*p ++ = (value >> 8) & 0xff, \
*p ++ = (value >> 16) & 0xff, \
*p ++ = (value >> 24) & 0xff

#define BYTE2INT(p, value) \
    value = (*p ++ & 0xff), \
value |= ((*p ++ << 8) & 0xff00), \
value |= ((*p ++ << 16) & 0xff0000), \
value |= ((*p ++ << 24) & 0xff000000)

#define MAX_KV_LEN 256

int init_kv();
void deinit_kv();
int set_kv_in_flash(const char *key, const void *value, int len, int sync);
int get_kv(const char *key, char *buffer, int *buffer_len);
int remove_kv(const char *key);
int set_kv_in_ram(const char *key, const void *value, int len);
void *alink_base64_decode_alloc(const char *str, int *len);
char *alink_base64_encode_alloc(void *data, int len);

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif

#endif

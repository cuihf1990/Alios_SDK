#include <stdint.h>
#include <string.h>

#define TFS_EMULATE_ID2_3DES_INDEX 0
#define TFS_EMULATE_ID2_RSA_INDEX 1

#ifdef TFS_ID2_RSA
#define TFS_EMULATE_ID2_INDEX TFS_EMULATE_ID2_RSA_INDEX
#else
#define TFS_EMULATE_ID2_INDEX TFS_EMULATE_ID2_3DES_INDEX
#endif

#ifdef TFS_ONLINE
static const char *tfs_id2[] = { "Y00F340010A3BDE05",
                                 "Y00F300016247B17C"
                               };
#else /* offline */
static const char *tfs_id2[] = { "Y007A401104D230A4",
                                 "Y009B0011BF09F4F3"
                               };
#endif /* end offline */

int tfs_emulate_id2_index = TFS_EMULATE_ID2_INDEX;

int emu_get_ID2(uint8_t *id2, uint32_t *len)
{
    memcpy(id2, tfs_id2[TFS_EMULATE_ID2_INDEX], 17);

    if (len != NULL) {
        *len = 17;
    }

    return 0;
}

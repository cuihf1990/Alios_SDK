#include <string.h>
#include "pal.h"
#include "log.h"

#if defined(TFS_TEE)
#include "tee_id2.h"
#elif defined(TFS_SW)
#include "sm_id2.h"
#else
#include "cmd.h"
#endif

#define TAG_HAL_ID2 "TFS_HAL_ID2"

int hal_get_ID2(uint8_t *id2, uint32_t *len)
{
    int ret = 0;

    LOGD(TAG_HAL_ID2, "[%s]: enter.\n", __func__);
#if defined(TFS_TEE)
    ret = tee_get_ID2(id2, len);
    if (ret != 0) {
        LOGE(TAG_HAL_ID2, "[%s]:tee env error!\n", __func__);
        return -1;
    }
#elif defined(TFS_SW)
    ret = tee_get_ID2(id2, len);
    if (ret != 0) {
        LOGE(TAG_HAL_ID2, "[%s]:sw env error!\n", __func__);
        return -1;
    }
#else
    uint32_t in_len = 8;
    uint8_t in[8] = {0};
    uint32_t out_len = 0;
    uint8_t out[27] = {0};

    fill_package(in, CMD_GET_ID2, NULL, 0);

    ret = hal_cmd(CMD_GET_ID2, in, in_len, out, &out_len);

    if (ret != 0 || out_len != 27) {
        LOGE(TAG_HAL_ID2, "[%s]: hal_cmd error!\n", __func__);
        return -1;
    }

    if (len != NULL) {
        *len = (out[23] & 0XFF);
    }

    memcpy(id2, out + 6, out[23] & 0XFF);
#endif
    return 0;
}

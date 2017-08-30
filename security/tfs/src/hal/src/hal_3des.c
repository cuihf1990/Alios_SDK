#include <string.h>
#include "cmd.h"
#include "pal.h"
#include "log.h"
#include "hal_3des.h"

#define MD5_SIZE      16
#define SIGN_IN_MAX   512
#define SIGN_OUT_MAX  256

#define TAG_HAL_3DES "TFS_HAL_3DES"

int hal_3DES_sign(uint8_t ID, const uint8_t *in, uint32_t in_len,
                  uint8_t *sign, uint32_t *sign_len, uint8_t mode)
{
    int ret = 0;
    uint8_t md5_buf[MD5_SIZE] = {0};

    LOGD(TAG_HAL_3DES, "[%s]: enter.\n", __func__);
    if (in_len > SIGN_IN_MAX) {
        LOGE(TAG_HAL_3DES, "[%s]: input data error!\n", __func__);
        return -1;
    }

    pal_md5_sum(in, in_len, md5_buf);

    ret = hal_3DES_encrypt(ID, md5_buf, MD5_SIZE, sign, sign_len, mode);
    if (ret != 0) {
        LOGE(TAG_HAL_3DES, "[%s]: hal_3DES_encrypt error!\n", __func__);
        return -1;
    }
    LOGD(TAG_HAL_3DES, "[%s]: sign_len = %d\n", __func__, *sign_len);
    if (*sign_len > SIGN_OUT_MAX) {
        LOGE(TAG_HAL_3DES, "[%s]: pub-key too long!\n", __func__);
        LOGE(TAG_HAL_3DES, "[%s]: sign_len = %d\n", __func__, *sign_len);
        return -1;
    }

    return 0;
}

int hal_3DES_verify(uint8_t ID, const uint8_t *in, uint32_t in_len,
                    uint8_t *sign, uint32_t sign_len, uint8_t mode)
{
    uint8_t md5_buf[MD5_SIZE];
    uint8_t dec_buf[SIGN_OUT_MAX];
    uint32_t dec_len;
    int ret = 0;

    LOGD(TAG_HAL_3DES, "[%s]: enter.\n", __func__);
    if (in_len > SIGN_IN_MAX || sign_len > SIGN_OUT_MAX) {
        LOGE(TAG_HAL_3DES, "[%s]: input data error!\n", __func__);
        return -1;
    }
    pal_md5_sum(in, in_len, md5_buf);

    ret = hal_3DES_decrypt(ID, sign, sign_len, dec_buf, &dec_len, mode);
    if (ret != 0) {
        LOGE(TAG_HAL_3DES, "[%s]: hal_3DES_decrypt error!\n", __func__);
        return -1;
    }

    if (dec_len != MD5_SIZE) {
        return -1;
    }

    return memcmp(md5_buf, dec_buf, dec_len);
}

static int hal_3DES_cmd(uint32_t cmd, uint8_t ID, const uint8_t *in,
                        uint32_t in_len,
                        uint8_t *out, uint32_t *out_len, uint8_t mode)
{
    int ret = 0;
    uint8_t *_in = NULL;
    uint8_t *arg = NULL;
    uint8_t _out[SIGN_OUT_MAX + 11] = {0};
    uint32_t _in_len = 0;
    uint32_t _out_len = 0;

    LOGD(TAG_HAL_3DES, "[%s]: enter.\n", __func__);
    if (in_len > SIGN_IN_MAX) {
        LOGE(TAG_HAL_3DES, "[%s]: input data error!\n", __func__);
        return -1;
    }

    _in_len = in_len + 12;
    _in = (uint8_t *)pal_memory_malloc(_in_len);
    if (_in == NULL) {
        LOGE(TAG_HAL_3DES, "[%s]: malloc error!\n", __func__);
        return -1;
    }

    arg = (uint8_t *)pal_memory_malloc(in_len + 4);
    if (arg == NULL) {
        LOGE(TAG_HAL_3DES, "[%s]: malloc error!\n", __func__);
        pal_memory_free(_in);
        return -1;
    }
    arg[0] = ID;
    arg[1] = (in_len >> 8) & 0XFF;
    arg[2] = in_len & 0XFF;
    memcpy(&arg[3], in, in_len);
    arg[3 + in_len] = mode;

    fill_package(_in, cmd, arg, in_len + 4);
    pal_memory_free(arg);

    ret = hal_cmd(cmd, _in, _in_len, _out, &_out_len);
    if (ret != 0) {
        LOGE(TAG_HAL_3DES, "[%s]: hal_cmd error!\n", __func__);
        pal_memory_free(_in);
        return -1;
    }
    pal_memory_free(_in);

    if (_out[_out_len - 3] != RES_OK) {
        LOGE(TAG_HAL_3DES, "[%s]: response error!\n", __func__);
        return -1;
    }

    *out_len = (_out[6] & 0XFF) << 8;
    *out_len |= _out[7] & 0XFF;

    memcpy(out, _out + 8, *out_len);

    return 0;
}

int hal_3DES_encrypt(uint8_t ID, const uint8_t *in, uint32_t in_len,
                     uint8_t *out, uint32_t *out_len, uint8_t mode)
{
    LOGD(TAG_HAL_3DES, "[%s]: enter.\n", __func__);
    return hal_3DES_cmd(CMD_3DES_ENCRYPT, ID, in, in_len, out, out_len, mode);
}

int hal_3DES_decrypt(uint8_t ID, uint8_t *in, uint32_t in_len,
                     uint8_t *out, uint32_t *out_len, uint8_t mode)
{
    LOGD(TAG_HAL_3DES, "[%s]: enter.\n", __func__);
    return hal_3DES_cmd(CMD_3DES_DECRYPT, ID, in, in_len, out, out_len, mode);
}

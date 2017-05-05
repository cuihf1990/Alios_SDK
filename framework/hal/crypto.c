/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <hal/hal.h>
#include <yos/log.h>

static hal_aes_module_t *current_aes_module;

int hal_aes_register(hal_aes_module_t *mod)
{
    if(mod == NULL ||
      !mod->aes_init ||
      !mod->aes_destroy ||
      !mod->aes_cbc_encrypt ||
      !mod->aes_cbc_decrypt) {
        return -1;
    }

    if (mod->priv_data_size < 0 || mod->priv_data_size > AES_PRIV_DATA_MAXSIZE) {
        return -2;
    }

    current_aes_module = mod;

    return 0;
}

int hal_aes_init(void **priv_data)
{
    int   ret  = 0;
    void *priv = NULL;

    if (current_aes_module == NULL) {
        return -1;
    }

    if (priv_data == NULL) {
        return -2;
    }

    if (current_aes_module->priv_data_size > 0) {
        priv = (void*)malloc(current_aes_module->priv_data_size);
        if(priv == NULL) {
            return -3;
        }
        *priv_data = priv;
    }

    if (current_aes_module != NULL && current_aes_module->aes_init) {
        ret = current_aes_module->aes_init(*priv_data);
    }

    return ret;
}

int hal_aes_destroy(void *priv_data)
{
    int ret = 0;

    if (current_aes_module == NULL) {
        return -1;
    }

    if (current_aes_module != NULL && current_aes_module->aes_destroy) {
        ret = current_aes_module->aes_destroy(priv_data);
    }

    if (current_aes_module->priv_data_size > 0) {
        free(priv_data);
    }

    return ret;
}

int hal_aes_cbc_encrypt(
        void *priv_data,
        const unsigned char *src, const unsigned int src_len,
        const unsigned char *key, const int key_len,
        const unsigned char *ivec,
        unsigned char *dst, unsigned int *dst_len)
{
    int ret = -1;

    if (current_aes_module == NULL) {
        return -1;
    }

    if (current_aes_module->aes_cbc_encrypt == NULL) {
        return -2;
    }

    ret = current_aes_module->aes_cbc_encrypt(priv_data,
                                              src, src_len,
                                              key, key_len,
                                              ivec,
                                              dst, dst_len);

    return ret;
}

int hal_aes_cbc_decrypt(
        void *priv_data,
        const unsigned char *src, const unsigned int src_len,
        const unsigned char *key, const int key_len,
        const unsigned char *ivec,
        unsigned char *dst, unsigned int *dst_len)
{
    int ret = -1;

    if (current_aes_module == NULL) {
        return -1;
    }

    if (current_aes_module->aes_cbc_decrypt == NULL) {
        return -2;
    }

    ret = current_aes_module->aes_cbc_decrypt(priv_data,
                                              src, src_len,
                                              key, key_len,
                                              ivec,
                                              dst, dst_len);
    return ret;
}

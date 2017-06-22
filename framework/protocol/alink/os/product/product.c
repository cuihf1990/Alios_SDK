/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
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

#include <stdio.h>
#include <string.h>
#include "product.h"

//TODO: update these product info
#ifndef MESH_GATEWAY_SERVICE
#define product_model           "ALINKTEST_LIVING_LIGHT_ALINK_TEST"
#define product_key             "5gPFl8G4GyFZ1fPWk20m"
#define product_secret          "ngthgTlZ65bX5LpViKIWNsDPhOf2As9ChnoL9gQb"
#define product_debug_key       "dpZZEpm9eBfqzK7yVeLq"
#define product_debug_secret    "THnfRRsU5vu6g6m9X6uFyAjUWflgZ0iyGjdEneKm"
#else
#define product_model           "ALINKTEST_SECURITY_GATEWAY_QUANWU_001"
#define product_key             "V2hpRG0k7Pbr1bmxDCat"
#define product_secret          "O71DlsrrTkImG0NxowxaA5oFFjyxTj1n8FwWzOJv"
#define product_debug_key       "dpZZEpm9eBfqzK7yVeLq"
#define product_debug_secret    "THnfRRsU5vu6g6m9X6uFyAjUWflgZ0iyGjdEneKm"
#define PRODUCT_ASR_APP_KEY     "box2015product01"
#endif

char *product_get_name(char name_str[PRODUCT_NAME_LEN])
{
    return strncpy(name_str, "alink_product", PRODUCT_NAME_LEN);
}

char *product_get_version(char ver_str[PRODUCT_VERSION_LEN])
{
    return strncpy(ver_str, (const char *)get_yos_os_version(), PRODUCT_VERSION_LEN);
}

char *product_get_model(char model_str[PRODUCT_MODEL_LEN])
{
    return strncpy(model_str, product_model, PRODUCT_MODEL_LEN);
}

char *product_get_key(char key_str[PRODUCT_KEY_LEN])
{
    return strncpy(key_str, product_key, PRODUCT_KEY_LEN);
}

char *product_get_secret(char secret_str[PRODUCT_SECRET_LEN])
{
    return strncpy(secret_str, product_secret, PRODUCT_SECRET_LEN);
}

char *product_get_debug_key(char key_str[PRODUCT_KEY_LEN])
{
    return strncpy(key_str, product_debug_key, PRODUCT_KEY_LEN);
}

char *product_get_debug_secret(char secret_str[PRODUCT_SECRET_LEN])
{
    return strncpy(secret_str, product_debug_secret, PRODUCT_SECRET_LEN);
}

char *product_get_sn(char sn_str[PRODUCT_SN_LEN])
{
    char *p = sn_str;
    int i = 0;

    os_wifi_get_mac_str(sn_str);
    while (*p != '\0' && i < (PRODUCT_SN_LEN - 1)) {
        if (*p == ':') *p = '0';
        p++; i++;
    }

    return sn_str;
}

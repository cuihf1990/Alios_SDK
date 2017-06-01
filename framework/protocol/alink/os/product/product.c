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
#define product_model           "ALINKTEST_LIVING_LIGHT_ALINK_TEST"
#define product_key             "5gPFl8G4GyFZ1fPWk20m"
#define product_secret          "ngthgTlZ65bX5LpViKIWNsDPhOf2As9ChnoL9gQb"
#define product_debug_key       "dpZZEpm9eBfqzK7yVeLq"
#define product_debug_secret    "THnfRRsU5vu6g6m9X6uFyAjUWflgZ0iyGjdEneKm"

char *product_get_name(char name_str[PRODUCT_NAME_LEN])
{
	return strncpy(name_str, "alink_product", PRODUCT_NAME_LEN);
}

char *product_get_version(char ver_str[PRODUCT_VERSION_LEN])
{
	return strncpy(ver_str, "1.0", PRODUCT_VERSION_LEN);
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
	return strncpy(sn_str, "1923450656869", PRODUCT_SN_LEN);
}

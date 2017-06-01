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

#include "digest_algorithm.h"
#include <string.h>

static int digest_hmac_md5(const unsigned char *msg, uint32_t msg_len, const unsigned char *key, uint32_t key_len, unsigned char *digest)
{
	void *context;
	unsigned char k_ipad[65];			/* inner padding key XORd with ipad */
	unsigned char k_opad[65];			/* outer padding* key XORd with opad */
	unsigned char tk[16];
	int i;

	if (key_len > 64) {
		void *ctx;
		ctx = digest_md5_init();
		digest_md5_update(ctx, (uint8_t *) key, key_len);
		digest_md5_final(&ctx, (uint8_t *) tk);

		key = tk;
		key_len = 16;
	}

	memset(k_ipad, 0, sizeof(k_ipad));
	memset(k_opad, 0, sizeof(k_opad));
	memcpy(k_ipad, key, key_len);
	memcpy(k_opad, key, key_len);

	for (i = 0; i < 64; i++) {
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}

	context = digest_md5_init();
	digest_md5_update(context, k_ipad, 64);
	digest_md5_update(context, (uint8_t *) msg, msg_len);
	digest_md5_final(context, (uint8_t *) digest);

	context = digest_md5_init();
	digest_md5_update(context, k_opad, 64);
	digest_md5_update(context, (uint8_t *) digest, 16);
	digest_md5_final(context, (uint8_t *) digest);

	return 0;
}

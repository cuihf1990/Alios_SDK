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
#include <stdlib.h>
#include "digest_algorithm.h"
#include "yos/kernel.h"
#include "md5.c"
#include "sha2.c"
#include "hmac.c"

void *digest_md5_init(void)
{
	MD5_CTX *ctx = (MD5_CTX *) yos_malloc(sizeof(MD5_CTX));
	if (NULL == ctx) {
		return NULL;
	}

	MD5_Init(ctx);
	return ctx;
}

int digest_md5_update(void *md5, const void *data, uint32_t length)
{
	MD5_Update(md5, data, length);
	return 0;
}

int digest_md5_final(void *md5, unsigned char *digest)
{
	MD5_Final(digest, md5);
	yos_free(md5);
	return 0;
}

int digest_md5(const void *data, uint32_t length, unsigned char *digest)
{
	MD5_CTX *ctx = (MD5_CTX *) yos_malloc(sizeof(MD5_CTX));
	if (NULL == ctx) {
		return -1;
	}

	MD5_Init(ctx);
	MD5_Update(ctx, data, length);
	MD5_Final(digest, ctx);
	yos_free(ctx);
	return 0;
}

int digest_md5_file(const char *path, unsigned char *md5)
{
	FILE *fp = fopen(path, "rb");
	int bytes;
	unsigned char data[512];
	unsigned char digest[16];
	int i;

	MD5_CTX *ctx = (MD5_CTX *) yos_malloc(sizeof(MD5_CTX));
	if (NULL == ctx) {
		return -1;
	}

	if (fp == NULL) {
        yos_free(ctx);
		return -1;
	}

	MD5_Init(ctx);

	while ((bytes = fread(data, 1, 512, fp)) != 0) {
		MD5_Update(ctx, data, bytes);
	}

	MD5_Final(digest, ctx);

	for (i = 0; i < 16; i++) {
		sprintf((char *)&md5[i * 2], "%02x", digest[i]);
	}

	fclose(fp);
	yos_free(ctx);
	return 0;
}

void *digest_sha256_init(void)
{
	SHA256_CTX *ctx = (SHA256_CTX *) yos_malloc(sizeof(SHA256_CTX));
	if (NULL == ctx) {
		return NULL;
	}

	SHA256_Init(ctx);

	return ctx;
}

int digest_sha256_update(void *sha256, const void *data, uint32_t length)
{
	SHA256_Update(sha256, data, length);
	return 0;
}

int digest_sha256_final(void *sha256, unsigned char *digest)
{
	SHA256_Final(digest, sha256);
	yos_free(sha256);
	return 0;
}

int digest_sha256(const void *data, uint32_t length, unsigned char *digest)
{
	SHA256_CTX *ctx = (SHA256_CTX *) yos_malloc(sizeof(SHA256_CTX));
	if (NULL == ctx) {
		return -1;
	}

	memset(ctx, 0, sizeof(SHA256_CTX));

	SHA256_Init(ctx);
	SHA256_Update(ctx, data, length);
	SHA256_Final(digest, ctx);
	yos_free(ctx);

	return 0;
}

void *digest_sha384_init(void)
{
	SHA384_CTX *ctx = (SHA384_CTX *) yos_malloc(sizeof(SHA384_CTX));
	if (NULL == ctx) {
		return NULL;
	}

	SHA384_Init(ctx);

	return ctx;
}

int digest_sha384_update(void *sha384, const void *data, uint32_t length)
{
	SHA384_Update(sha384, data, length);
	return 0;
}

int digest_sha384_final(void *sha384, unsigned char *digest)
{
	SHA384_Final(digest, sha384);
	yos_free(sha384);
	return 0;
}

int digest_sha384(const void *data, uint32_t length, unsigned char *digest)
{
	SHA384_CTX *ctx = (SHA384_CTX *) yos_malloc(sizeof(SHA384_CTX));
	if (NULL == ctx) {
		return -1;
	}

	SHA384_Init(ctx);
	SHA384_Update(ctx, data, length);
	SHA384_Final(digest, ctx);
	yos_free(ctx);

	return 0;
}

void *digest_sha512_init(void)
{
	SHA512_CTX *ctx = (SHA512_CTX *) yos_malloc(sizeof(SHA512_CTX));
	if (NULL == ctx) {
		return NULL;
	}

	SHA512_Init(ctx);

	return ctx;
}

int digest_sha512_update(void *sha512, const void *data, uint32_t length)
{
	SHA512_Update(sha512, data, length);
	return 0;
}

int digest_sha512_final(void *sha512, unsigned char *digest)
{
	SHA512_Final(digest, sha512);
	yos_free(sha512);
	return 0;
}

int digest_sha512(const void *data, uint32_t length, unsigned char *digest)
{
	SHA512_CTX *ctx = (SHA512_CTX *) yos_malloc(sizeof(SHA512_CTX));
	if (NULL == ctx) {
		return -1;
	}

	SHA512_Init(ctx);
	SHA512_Update(ctx, data, length);
	SHA512_Final(digest, ctx);
	yos_free(ctx);

	return 0;
}

int digest_hmac(enum digest_type type, const unsigned char *data, uint32_t data_len, const unsigned char *key, uint32_t key_len, unsigned char *digest)
{
	switch (type) {
	case DIGEST_TYPE_MD5:
		return digest_hmac_md5(data, data_len, key, key_len, digest);

	case DIGEST_TYPE_SHA256:
		break;

	case DIGEST_TYPE_SHA384:
		break;

	case DIGEST_TYPE_SHA512:
		break;

	default:
		break;
	}

	return -1;
}

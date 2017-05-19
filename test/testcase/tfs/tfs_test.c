#include <yunit.h>
#include <yts.h>
#include <tfs.h>

#define IN_DATA_SIZE 117
char in_data[IN_DATA_SIZE + 1];
#define BUF_MAX 256
static uint8_t out_data[BUF_MAX] = {0};
static uint8_t dec_out[BUF_MAX] = {0};

static void prepare_test_data() {
    int i = 0;
    for (i = 0;i < IN_DATA_SIZE;i ++) {
        in_data[i] = i%10 + '0';
    }
    in_data[IN_DATA_SIZE] = '\0';
}

static void test_tfs_get_ID2(void)
{
    int ret = 0;
    uint32_t len = TFS_ID2_LEN + 1;
    uint8_t id2[TFS_ID2_LEN + 1] = {0};

    ret = tfs_get_ID2(id2, &len);

	YUNIT_ASSERT(ret == 0);
}

static void test_tfs_id2_sign(void)
{
    int ret = 0;
    uint32_t len = BUF_MAX;
    uint32_t in_len = 0;

	prepare_test_data();
	in_len = strlen(in_data);
    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_sign((const uint8_t *)in_data, strlen(in_data), out_data, &len);

	YUNIT_ASSERT(ret == 0);
}

static void test_tfs_id2_verify(void)
{
    int ret = 0;
    uint32_t len = BUF_MAX;
    uint32_t in_len = 0;

	prepare_test_data();
	in_len = strlen(in_data);
    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_sign((const uint8_t *)in_data, in_len, out_data, &len); //sign first

    ret = tfs_id2_verify((const uint8_t *)in_data, in_len, out_data, len); //then verify
	YUNIT_ASSERT(ret == 0);
}

static void test_tfs_id2_encrypt(void)
{
    int ret = 0;
    uint32_t len = BUF_MAX;
    uint32_t in_len = 0;

	prepare_test_data();
	in_len = strlen(in_data);

    memset(out_data, 0, BUF_MAX);

    ret = tfs_id2_encrypt((uint8_t *)in_data, in_len, out_data, &len);
	YUNIT_ASSERT(ret == 0);
}

static void test_tfs_id2_decrypt(void)
{
    int ret = 0;
    uint32_t enc_len = BUF_MAX;
    uint32_t dec_len = BUF_MAX;
    uint32_t in_len = 0;

	prepare_test_data();
	in_len = strlen(in_data);

    memset(out_data, 0, BUF_MAX);
    memset(dec_out, 0, BUF_MAX);
    ret = tfs_id2_encrypt((uint8_t *)in_data, in_len, out_data, &enc_len); //encrypt first
    ret = tfs_id2_decrypt(out_data, enc_len, dec_out, &dec_len); //then decrypt
	YUNIT_ASSERT(ret == 0);
}

static void test_tfs_get_auth_code(void)
{
    int ret = 0;
    uint32_t len = BUF_MAX;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_get_auth_code(out_data, &len);
	YUNIT_ASSERT(ret == 0);
}

static void test_tfs_activate_device(void) {
    int ret = -1;

    ret = tfs_activate_device();
	YUNIT_ASSERT(ret == 0);
}

static void test_tfs_id2_get_auth_code(void)
{
    int ret = -1;
    int len = BUF_MAX;
    uint64_t timestamp = 0;

    timestamp = 1487922356796ULL;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_get_auth_code(timestamp, out_data, &len);
	YUNIT_ASSERT(ret == 0);
}

static void test_tfs_id2_get_digest_auth_code(void)
{
    int ret = -1;
    int len = BUF_MAX;
    uint64_t timestamp = 0; // in ms
    uint8_t *digest = "abcd";

    timestamp = 1487922356796ULL;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_get_digest_auth_code(timestamp, digest, strlen(digest), out_data, &len);
	YUNIT_ASSERT(ret == 0);
}

static int test_tfs_aes128_enc_dec_unit(int size, uint8_t padding)
{
    int ret = -1;
    const uint8_t in[64] = "Hello World!1234567890123456789012345678901234567890";
    uint8_t out[128];
    uint8_t iv_enc[16] = {0};
    uint8_t iv_dec[16] = {0};
    const uint8_t key[16] = "Demo-Test";
    uint8_t dec[128] = {0};
    int32_t in_len = size;
    int32_t out_len = 0;
    int32_t dec_len = 0;

    ret = tfs_aes128_cbc_enc(key, iv_enc, in_len, in, &out_len, out, padding);
	if (ret != 0) {
	    return -1;
	}

    ret = tfs_aes128_cbc_dec(key, iv_dec, out_len, out, &dec_len, dec, padding);
	if (ret != 0) {
	    return -1;
	}

	return 0;
}

static void test_tfs_aes128_enc_dec(void) {
	int ret = -1;
    ret = test_tfs_aes128_enc_dec_unit(50, TFS_AES_PKCS7_PADDING);
	YUNIT_ASSERT(ret == 0);
    ret = test_tfs_aes128_enc_dec_unit(64, TFS_AES_PKCS7_PADDING);
	YUNIT_ASSERT(ret == 0);

    ret = test_tfs_aes128_enc_dec_unit(50, TFS_AES_ZERO_PADDING);
	YUNIT_ASSERT(ret == 0);
    ret = test_tfs_aes128_enc_dec_unit(64, TFS_AES_ZERO_PADDING);
	YUNIT_ASSERT(ret == 0);

    ret = test_tfs_aes128_enc_dec_unit(50, TFS_AES_NO_PADDING);
	YUNIT_ASSERT(ret != 0);
    ret = test_tfs_aes128_enc_dec_unit(64, TFS_AES_NO_PADDING);
	YUNIT_ASSERT(ret == 0);
}

static int init(void)
{
    return 0;
}

static int cleanup(void)
{
    return 0;
}

static void setup(void)
{

}

static void teardown(void)
{

}

static yunit_test_case_t yunos_tfs_testcases[] = {
    { "tfs_get_ID2", test_tfs_get_ID2},
    { "tfs_id2_sign", test_tfs_id2_sign},
    { "tfs_id2_verify", test_tfs_id2_verify},
    { "tfs_id2_encrypt", test_tfs_id2_encrypt},
    { "tfs_id2_decrypt", test_tfs_id2_decrypt},
    { "tfs_get_auth_code", test_tfs_get_auth_code},
    { "tfs_activate_device", test_tfs_activate_device},
    { "tfs_id2_get_auth_code", test_tfs_id2_get_auth_code},
    { "tfs_id2_get_digest_auth_code", test_tfs_id2_get_digest_auth_code},
    { "tfs_es128_enc_dec", test_tfs_aes128_enc_dec},
    YUNIT_TEST_CASE_NULL
};

static yunit_test_suite_t suites[] = {
    { "tfs", init, cleanup, setup, teardown, yunos_tfs_testcases },
    YUNIT_TEST_SUITE_NULL
};

void test_tfs(void)
{
    yunit_add_test_suites(suites);
}


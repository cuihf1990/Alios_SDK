#include <yunit.h>
#include <yts.h>
#include <tfs.h>
#include <stdio.h>

extern int tfs_id2_encrypt(const uint8_t *in, uint32_t in_len,uint8_t *out, uint32_t *out_len);

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

static void test_tfs_get_ID2_param_id2_null(void)
{
    int ret = 0;
    uint32_t len = TFS_ID2_LEN + 1;

    ret = tfs_get_ID2(NULL, &len);

	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_get_ID2_param_len_null(void)
{
    int ret = 0;
    uint8_t id2[TFS_ID2_LEN + 1] = {0};

    ret = tfs_get_ID2(id2, NULL);

	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_get_ID2_all_param_null(void)
{
    int ret = 0;

    ret = tfs_get_ID2(NULL, NULL);

	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_sign(void)
{
    int ret = 0;
    uint32_t len = BUF_MAX;
    uint32_t in_len = 0;

	prepare_test_data();
	in_len = strlen(in_data);
    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_sign((const uint8_t *)in_data, in_len, out_data, &len);

	YUNIT_ASSERT(ret == 0);
}

static void test_tfs_id2_sign_param_in_null(void)
{
    int ret = 0;
    uint32_t len = BUF_MAX;
    uint32_t in_len = 0;

	prepare_test_data();
	in_len = strlen(in_data);
    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_sign(NULL, in_len, out_data, &len);

	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_sign_param_in_len_zero(void)
{
    int ret = 0;
    uint32_t len = BUF_MAX;
    uint32_t in_len = 0;

	prepare_test_data();
    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_sign((const uint8_t *)in_data, in_len, out_data, &len);

	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_sign_param_sign_null(void)
{
    int ret = 0;
    uint32_t len = BUF_MAX;
    uint32_t in_len = 0;

	prepare_test_data();
	in_len = strlen(in_data);
    ret = tfs_id2_sign((const uint8_t *)in_data, in_len, NULL, &len);

	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_sign_param_sign_len_null(void)
{
    int ret = 0;
    uint32_t in_len = 0;

	prepare_test_data();
	in_len = strlen(in_data);
    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_sign((const uint8_t *)in_data, in_len, out_data, NULL);

	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_sign_all_param_null(void)
{
    int ret = 0;

    ret = tfs_id2_sign(NULL, 0, NULL, NULL);

	YUNIT_ASSERT(ret != 0);
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

static void test_tfs_id2_decrypt_param_in_null(void)
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
    ret = tfs_id2_decrypt(NULL, enc_len, dec_out, &dec_len); //then decrypt
	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_decrypt_param_in_len_zero(void)
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
    ret = tfs_id2_decrypt(out_data, 0, dec_out, &dec_len); //then decrypt
	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_decrypt_param_out_null(void)
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
    ret = tfs_id2_decrypt(out_data, enc_len, NULL, &dec_len); //then decrypt
	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_decrypt_param_out_len_null(void)
{
    int ret = 0;
    uint32_t enc_len = BUF_MAX;
    uint32_t in_len = 0;

	prepare_test_data();
	in_len = strlen(in_data);

    memset(out_data, 0, BUF_MAX);
    memset(dec_out, 0, BUF_MAX);
    ret = tfs_id2_encrypt((uint8_t *)in_data, in_len, out_data, &enc_len); //encrypt first
    ret = tfs_id2_decrypt(out_data, enc_len, dec_out, NULL); //then decrypt
	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_decrypt_all_param_null(void)
{
    int ret = 0;
    uint32_t enc_len = BUF_MAX;
    uint32_t in_len = 0;

	prepare_test_data();
	in_len = strlen(in_data);

    memset(out_data, 0, BUF_MAX);
    memset(dec_out, 0, BUF_MAX);
    ret = tfs_id2_encrypt((uint8_t *)in_data, in_len, out_data, &enc_len); //encrypt first
    ret = tfs_id2_decrypt(NULL, 0, NULL, NULL); //then decrypt
	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_get_auth_code(void)
{
    int ret = 0;
    uint32_t len = BUF_MAX;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_get_auth_code(out_data, &len);
	YUNIT_ASSERT(ret == 0);
}

static void test_tfs_get_auth_code_param_auth_code_null(void)
{
    int ret = 0;
    uint32_t len = BUF_MAX;

    ret = tfs_get_auth_code(NULL, &len);
	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_get_auth_code_param_len_null(void)
{
    int ret = 0;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_get_auth_code(out_data, NULL);
	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_get_auth_code_all_param_null(void)
{
    int ret = 0;

    ret = tfs_get_auth_code(NULL, NULL);
	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_activate_device(void) {
    int ret = -1;

    ret = tfs_activate_device();
	YUNIT_ASSERT(ret == 0);
}

static void test_tfs_id2_get_auth_code(void)
{
    int ret = -1;
    uint32_t len = BUF_MAX;
    uint64_t timestamp = 0;

    timestamp = 1487922356796ULL;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_get_auth_code(timestamp, out_data, &len);
	YUNIT_ASSERT(ret == 0);
}

static void test_tfs_id2_get_auth_code_param_auth_code_null(void)
{
    int ret = -1;
    uint32_t len = BUF_MAX;
    uint64_t timestamp = 0;

    timestamp = 1487922356796ULL;

    ret = tfs_id2_get_auth_code(timestamp, NULL, &len);
	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_get_auth_code_param_auth_len_null(void)
{
    int ret = -1;
    uint64_t timestamp = 0;

    timestamp = 1487922356796ULL;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_get_auth_code(timestamp, out_data, NULL);
	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_get_auth_code_all_param_null(void)
{
    int ret = -1;
    uint64_t timestamp = 0;

    timestamp = 1487922356796ULL;

    ret = tfs_id2_get_auth_code(timestamp, NULL, NULL);
	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_get_digest_auth_code(void)
{
    int ret = -1;
    uint32_t len = BUF_MAX;
    uint64_t timestamp = 0; // in ms
    char *digest = "abcd";

    timestamp = 1487922356796ULL;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_get_digest_auth_code(timestamp, (uint8_t *)digest, strlen(digest), out_data, &len);
	YUNIT_ASSERT(ret == 0);
}

static void test_tfs_id2_get_digest_auth_code_param_digest_null(void)
{
    int ret = -1;
    uint32_t len = BUF_MAX;
    uint64_t timestamp = 0; // in ms
    char *digest = "abcd";

    timestamp = 1487922356796ULL;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_get_digest_auth_code(timestamp, NULL, strlen(digest), out_data, &len);
	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_get_digest_auth_code_param_digest_len_zero(void)
{
    int ret = -1;
    uint32_t len = BUF_MAX;
    uint64_t timestamp = 0; // in ms
    char *digest = "abcd";

    timestamp = 1487922356796ULL;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_get_digest_auth_code(timestamp, (uint8_t *)digest, 0, out_data, &len);
	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_get_digest_auth_code_param_auth_code_null(void)
{
    int ret = -1;
    uint32_t len = BUF_MAX;
    uint64_t timestamp = 0; // in ms
    char *digest = "abcd";

    timestamp = 1487922356796ULL;

    ret = tfs_id2_get_digest_auth_code(timestamp, (uint8_t *)digest, strlen(digest), NULL, &len);
	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_get_digest_auth_code_param_auth_len_null(void)
{
    int ret = -1;
    uint64_t timestamp = 0; // in ms
    char *digest = "abcd";

    timestamp = 1487922356796ULL;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_get_digest_auth_code(timestamp, (uint8_t *)digest, strlen(digest), out_data, NULL);
	YUNIT_ASSERT(ret != 0);
}

static void test_tfs_id2_get_digest_auth_code_all_param_null(void)
{
    int ret = -1;
    uint64_t timestamp = 1487922356796ULL;

    ret = tfs_id2_get_digest_auth_code(timestamp, NULL, 0, NULL, NULL);
	YUNIT_ASSERT(ret != 0);
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
	{ "tfs_get_ID2_param_id2_null", test_tfs_get_ID2_param_id2_null},
	{ "tfs_get_ID2_param_len_null", test_tfs_get_ID2_param_len_null},
	{ "tfs_get_ID2_all_param_null", test_tfs_get_ID2_all_param_null},
    { "tfs_id2_sign", test_tfs_id2_sign},
	{ "tfs_id2_sign_param_in_null", test_tfs_id2_sign_param_in_null},
	{ "tfs_id2_sign_param_in_len_zero", test_tfs_id2_sign_param_in_len_zero},
	{ "tfs_id2_sign_param_sign_null", test_tfs_id2_sign_param_sign_null},
	{ "tfs_id2_sign_param_sign_len_null", test_tfs_id2_sign_param_sign_len_null},
	{ "tfs_id2_sign_all_param_null", test_tfs_id2_sign_all_param_null},
	{ "tfs_id2_decrypt", test_tfs_id2_decrypt},
    { "tfs_id2_decrypt_param_in_null", test_tfs_id2_decrypt_param_in_null},
	{ "tfs_id2_decrypt_param_in_len_zero", test_tfs_id2_decrypt_param_in_len_zero},
	{ "tfs_id2_decrypt_param_out_null", test_tfs_id2_decrypt_param_out_null},
	{ "tfs_id2_decrypt_param_out_len_null", test_tfs_id2_decrypt_param_out_len_null},
	{ "tfs_id2_decrypt_all_param_null", test_tfs_id2_decrypt_all_param_null},
    { "tfs_activate_device", test_tfs_activate_device},
    { "tfs_get_auth_code", test_tfs_get_auth_code},
	{ "tfs_get_auth_code_param_auth_code_null", test_tfs_get_auth_code_param_auth_code_null},
	{ "tfs_get_auth_code_param_len_null", test_tfs_get_auth_code_param_len_null},
	{ "tfs_get_auth_code_all_param_null", test_tfs_get_auth_code_all_param_null},
    { "tfs_id2_get_auth_code", test_tfs_id2_get_auth_code},
	{ "tfs_id2_get_auth_code_param_auth_code_null", test_tfs_id2_get_auth_code_param_auth_code_null},
	{ "tfs_id2_get_auth_code_param_auth_len_null", test_tfs_id2_get_auth_code_param_auth_len_null},
	{ "tfs_id2_get_auth_code_all_param_null", test_tfs_id2_get_auth_code_all_param_null},
    { "tfs_id2_get_digest_auth_code", test_tfs_id2_get_digest_auth_code},
	{ "tfs_id2_get_digest_auth_code_param_digest_null", test_tfs_id2_get_digest_auth_code_param_digest_null},
	{ "tfs_id2_get_digest_auth_code_param_digest_len_zero", test_tfs_id2_get_digest_auth_code_param_digest_len_zero},
	{ "tfs_id2_get_digest_auth_code_param_auth_code_null", test_tfs_id2_get_digest_auth_code_param_auth_code_null},
	{ "tfs_id2_get_digest_auth_code_param_auth_len_null", test_tfs_id2_get_digest_auth_code_param_auth_len_null},
	{ "tfs_id2_get_digest_auth_code_all_param_null", test_tfs_id2_get_digest_auth_code_all_param_null},
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


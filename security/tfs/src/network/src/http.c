#include <string.h>
#include "pal.h"
#include "log.h"

#define TAG_HTTP "TFS_HTTP"

#ifdef TFS_ONLINE
#define ID2_SERVER        "id2server.yunos.com"
//#define ID2_SERVER "140.205.172.1"
#else
#define ID2_SERVER        "id2server.daily.alibaba.net"
//#define ID2_SERVER "42.156.239.251"
#endif

#define ID2_PORT        80
#define HTTP_PACKET_MAX 1280
#define FUN_LEN_MAX     32
#define ARGU_LEN_MAX    1024

static int _http_fill_package(char *pack, const char *server,
                              const char *func, const char *arg);
static char *_http_get_json(const char *package);

int http_get_seed(const char *func, const char *arg, char *seed)
{
    int ret = -1;
    int sockfd = -1;
    char *json = NULL;
    int code;
    char send_pack[HTTP_PACKET_MAX] = {0};
    char recv_pack[HTTP_PACKET_MAX] = {0};
    const char *tokens[2];
    int tokens_size;

    LOGD(TAG_HTTP, "[%s]: enter.\n", __func__);
    if (strlen(func) > FUN_LEN_MAX || strlen(arg) > ARGU_LEN_MAX) {
        LOGE(TAG_HTTP, "[%s]: input too long\n", __func__);
        return -1;
    }

    sockfd = pal_network_create(ID2_SERVER, ID2_PORT);
    if (sockfd < 0) {
        LOGE(TAG_HTTP, "[%s]: connect id2server error\n", __func__);
        return -1;
    }

    _http_fill_package(send_pack, ID2_SERVER, func, arg);

    LOGD(TAG_HTTP, "[%s]: http post packet is:\n%s\n", __func__, send_pack);

    ret = pal_network_send(sockfd, send_pack, strlen(send_pack));
    if (ret != 0) {
        LOGE(TAG_HTTP, "[%s]: http send error\n", __func__);
        goto error_http;
    }

    ret = pal_network_recv(sockfd, recv_pack, NULL);
    if (ret != 0) {
        LOGE(TAG_HTTP, "[%s]: http recv error\n", __func__);
        goto error_http;
    }

    LOGD(TAG_HTTP, "[%s]: http recv packet is:\n%s\n", __func__, recv_pack);

    ret = -1;
    json = _http_get_json(recv_pack);
    if (json == NULL) {
        LOGE(TAG_HTTP, "[%s]: get json error\n", __func__);
        goto error_http;
    }
    LOGD(TAG_HTTP, "[%s]: the response json is:\n%s\n", __func__, json);

    tokens[0] = "code";
    tokens_size = 1;
    ret = pal_json_get_number_value(json, tokens, tokens_size, &code);
    if (ret == -1) {
        LOGE(TAG_HTTP, "[%s]: get code from json error\n", __func__);
        goto error_http;
    }

    if (code != 200) {
        ret = -1;
        LOGE(TAG_HTTP, "[%s]: return: %d error\n", __func__, code);
        goto error_http;
    }

    tokens[0] = "value";
    tokens[1] = "seed";
    tokens_size = 2;
    ret = pal_json_get_string_value(json, tokens, tokens_size, seed);
    if (ret == -1) {
        LOGE(TAG_HTTP, "[%s]: get seed item error\n", __func__);
        goto error_http;
    }

    ret = 0;
error_http:

    pal_network_close(sockfd);
    return ret;
}

int http_activate_dev(const char *func, const char *arg)
{
    int ret = -1;
    int sockfd = -1;
    char *json = NULL;
    int code;
    char send_pack[HTTP_PACKET_MAX] = {0};
    char recv_pack[HTTP_PACKET_MAX] = {0};
    const char *tokens[2];
    int tokens_size;

    LOGD(TAG_HTTP, "[%s]: enter.\n", __func__);
    if (strlen(func) > FUN_LEN_MAX || strlen(arg) > ARGU_LEN_MAX) {
        LOGE(TAG_HTTP, "[%s]: input too long\n", __func__);
        return -1;
    }

    sockfd = pal_network_create(ID2_SERVER, ID2_PORT);
    if (sockfd < 0) {
        LOGE(TAG_HTTP, "[%s]: connect id2server error\n", __func__);
        return -1;
    }

    _http_fill_package(send_pack, ID2_SERVER, func, arg);

    LOGD(TAG_HTTP, "[%s]: http post packet is:\n%s\n", __func__, send_pack);

    ret = pal_network_send(sockfd, send_pack, strlen(send_pack));
    if (ret != 0) {
        LOGE(TAG_HTTP, "[%s]: http send error\n", __func__);
        goto error_exit;
    }

    ret = pal_network_recv(sockfd, recv_pack, NULL);
    if (ret != 0) {
        LOGE(TAG_HTTP, "[%s]: http recv error\n", __func__);
        goto error_exit;
    }

    LOGD(TAG_HTTP, "[%s]: http recv packet is:\n%s\n", __func__, recv_pack);

    ret = -1;
    json = _http_get_json(recv_pack);
    if (json == NULL) {
        LOGE(TAG_HTTP, "[%s]: get json error\n", __func__);
        goto error_exit;
    }

    LOGD(TAG_HTTP, "[%s]: the response json is:\n%s\n", __func__, json);

    tokens[0] = "code";
    tokens_size = 1;
    ret = pal_json_get_number_value(json, tokens, tokens_size, &code);
    if (ret == -1) {
        LOGE(TAG_HTTP, "[%s]: get json code error\n", __func__);
        goto error_exit;
    }

    if (code != 200 && code != 12) {
        ret = -1;
        LOGE(TAG_HTTP, "[%s]: return: %d error\n", __func__, code);
        goto error_exit;
    }
    ret = 0;
error_exit:
    pal_network_close(sockfd);
    return ret;
}

int _http_fill_package(char *pack, const char *server,
                       const char *func, const char *arg)
{
    LOGD(TAG_HTTP, "[%s]: enter.\n", __func__);
    // http-head
    sprintf(pack, "POST %s HTTP/1.1\n", func);
    sprintf((pack + strlen(pack)), "Host: %s\n", server);
    strcat(pack, "Content-Type: application/x-www-form-urlencoded\n");
    strcat(pack, "Content-Length: ");
    sprintf((pack + strlen(pack)), "%d", (int)strlen(arg));
    strcat(pack, "\n\n");

    // http-body
    strcat(pack, arg);
    strcat(pack, "\r\n\r\n");

    return 0;
}

char *_http_get_json(const char *package)
{
    char *begin = NULL;
    char *end = NULL;

    LOGD(TAG_HTTP, "[%s]: enter.\n", __func__);
    begin = strchr(package, '{');
    end = strrchr(package, '}');
    if (end == NULL) {
        LOGE(TAG_HTTP, "[%s]: json end error\n", __func__);
        return NULL;
    }

    *(end + 1) = '\0';

    return begin;
}


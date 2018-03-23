/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 *
 * Alibaba Group retains all right, title and interest (including all
 * intellectual property rights) in and to this computer program, which is
 * protected by applicable intellectual property laws.  Unless you have
 * obtained a separate written license from Alibaba Group., you are not
 * authorized to utilize all or a part of this computer program for any
 * purpose (including reproduction, distribution, modification, and
 * compilation into object code), and you must immediately destroy or
 * return to Alibaba Group all copies of this computer program.  If you
 * are licensed by Alibaba Group, your rights to utilize this computer
 * program are limited by the terms of that license.  To obtain a license,
 * please contact Alibaba Group.
 *
 * This computer program contains trade secrets owned by Alibaba Group.
 * and, unless unauthorized by Alibaba Group in writing, you agree to
 * maintain the confidentiality of this computer program and related
 * information and to not disclose this computer program and related
 * information to any other person or entity.
 *
 * THIS COMPUTER PROGRAM IS PROVIDED AS IS WITHOUT ANY WARRANTIES, AND
 * Alibaba Group EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * INCLUDING THE WARRANTIES OF MERCHANTIBILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, TITLE, AND NONINFRINGEMENT.
 */

#include <stdint.h>
#include "os.h"
#include "passwd.h"
#include "awss_cmp.h"
#include "awss_wifimgr.h"
#include "awss_notify.h"
#include "awss_main.h"

#define LCMP_TOPIC_CNT_MAX  (10)

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

static char local_init = 0;
static char online_init = 0;

char *awss_build_sign_src(char *sign_src, int *sign_src_len)
{
    char *pk = NULL, *dev_name = NULL;
    int dev_name_len, pk_len, text_len;

    if (sign_src == NULL || sign_src_len == NULL)
        goto build_sign_src_err;

    pk = os_zalloc(OS_PRODUCT_KEY_LEN + 1);
    dev_name = os_zalloc(OS_PRODUCT_NAME_LEN + 1);
    if (pk == NULL || dev_name == NULL)
        goto build_sign_src_err;

    os_product_get_key(pk);
    os_product_get_name(dev_name);

    pk_len = strlen(pk);
    dev_name_len = strlen(dev_name);

    text_len = RANDOM_MAX_LEN + dev_name_len + pk_len;
    if (*sign_src_len < text_len)
        goto build_sign_src_err;

    *sign_src_len = text_len;

    memcpy(sign_src, aes_random, RANDOM_MAX_LEN);
    memcpy(sign_src + RANDOM_MAX_LEN, dev_name, dev_name_len);
    memcpy(sign_src + RANDOM_MAX_LEN + dev_name_len, pk, pk_len);

    os_free(pk);
    os_free(dev_name);

    return sign_src;

build_sign_src_err:
    if (pk) os_free(pk);
    if (dev_name) os_free(dev_name);
    return NULL;
}

const char *awss_build_topic(const char *topic_fmt, char *topic, unsigned int tlen)
{
    if (topic == NULL || topic_fmt == NULL || tlen == 0)
        return NULL;

    char pk[PRODUCT_KEY_LEN + 1] = {0};
    char dev_name[PRODUCT_NAME_LEN + 1] = {0};

    os_product_get_key(pk);
    os_product_get_name(dev_name);

    snprintf(topic, tlen - 1, topic_fmt, pk, dev_name);

    return topic;
}

int awss_build_packet(int type, void *id, void *ver, void *method,void *data, int code, void *packet, int *packet_len)
{

    if (packet_len == NULL || data == NULL || packet == NULL)
        return -1;

    int len = *packet_len;
    if (len <= 0)
        return -1;

    if (type == AWSS_CMP_PKT_TYPE_REQ) {
        if (ver == NULL || method == NULL)
            return -1;

        len = snprintf (packet, len - 1, AWSS_REQ_FMT, (char *)id, (char *)ver, (char *)method, (char *)data);
        return 0;
    } else if (type == AWSS_CMP_PKT_TYPE_RSP) {
        len = snprintf (packet, len - 1, AWSS_ACK_FMT, (char *)id, code, (char *)data);
        return 0;
    }
    return -1;
}

struct awss_cmp_couple {
    char *topic;
    void *cb;
};

#define AWSS_LOCAL_COUPLE_CNT  (6)
const struct awss_cmp_couple awss_local_couple[AWSS_LOCAL_COUPLE_CNT] = {
    {TOPIC_AWSS_SWITCHAP,            wifimgr_process_switch_ap_request},
    {TOPIC_AWSS_WIFILIST,            wifimgr_process_get_wifilist_request},
    {TOPIC_AWSS_GETDEVICEINFO_MCAST, wifimgr_process_mcast_get_device_info},
    {TOPIC_AWSS_GETDEVICEINFO_UCAST, wifimgr_process_ucast_get_device_info},
    {TOPIC_GETDEVICEINFO_MCAST,      online_mcast_get_device_info},
    {TOPIC_GETDEVICEINFO_UCAST,      online_ucast_get_device_info}
};

int awss_cmp_local_init()
{
    if (local_init)
        return 0;

    local_init = 1;

    char topic[TOPIC_LEN_MAX] = {0};
    int i;

    for (i = 0; i < sizeof(awss_local_couple) / sizeof(awss_local_couple[0]); i ++) {
        memset(topic, 0, sizeof(topic));
        awss_build_topic(awss_local_couple[i].topic, topic, TOPIC_LEN_MAX);
        awss_cmp_coap_register_cb(topic, awss_local_couple[i].cb);
    }

    awss_cmp_coap_loop(NULL);

    return 0;
}



#define AWSS_ONLINE_COUPLE_CNT  (5)
const struct awss_cmp_couple awss_online_couple[AWSS_ONLINE_COUPLE_CNT] = {
    {TOPIC_ZC_CHECKIN,         awss_enrollee_checkin},
    {TOPIC_ZC_ENROLLEE_REPLY,  awss_report_enrollee_reply},
    {TOPIC_ZC_CIPHER_REPLY,    awss_get_cipher_reply},
    {TOPIC_MATCH_REPORT_REPLY, awss_report_token_reply},
    {TOPIC_RESET_REPORT_REPLY, awss_report_reset_reply}
};

int awss_cmp_online_init()
{
    if (online_init)
        return 0;

    char topic[TOPIC_LEN_MAX] = {0};
    int i;

    for (i = 0; i < sizeof(awss_online_couple) / sizeof(awss_online_couple[0]); i ++) {
        memset(topic, 0, sizeof(topic));
        awss_build_topic(awss_online_couple[i].topic, topic, TOPIC_LEN_MAX);
        awss_cmp_mqtt_register_cb(topic, awss_online_couple[i].cb);
    }

    online_init = 1;

    return 0;
}

void awss_cmp_unrgist_topic()
{
    char topic[TOPIC_LEN_MAX] = { 0 };
    int i;
    int topic_len = 0;

    extern char awss_report_token_flag;
    awss_report_token_flag = 0;// don't report token when awss finished
    if (local_init){
        //TODO unregist local topic
    }

    if (online_init){
        topic_len = sizeof(awss_online_couple) / sizeof(awss_online_couple[0]);
        for (i = 0; i < topic_len - 1;i++) {//"reset_reply don't need unregister for reset"
            memset(topic, 0, sizeof(topic));
            awss_build_topic(awss_online_couple[i].topic, topic, TOPIC_LEN_MAX);
            awss_cmp_mqtt_unregister_cb(topic);
        }
    }

}

int awss_cmp_deinit()
{
    int i;
    char topic[TOPIC_LEN_MAX] = {0};

    if (!local_init && !online_init)
        return 0;
    awss_connectap_notify_stop();
    awss_devinfo_notify_stop();
    awss_cmp_coap_deinit();

    for (i = 0; i < sizeof(awss_online_couple) / sizeof(awss_online_couple[0]); i ++) {
        memset(topic, 0, sizeof(topic));
        awss_build_topic(awss_online_couple[i].topic, topic, TOPIC_LEN_MAX);
        awss_cmp_mqtt_unregister_cb(topic);
    }

    local_init = 0;
    online_init = 0;

    return 0;
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif

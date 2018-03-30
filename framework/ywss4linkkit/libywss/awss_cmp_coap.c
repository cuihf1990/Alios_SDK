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

#include <stdio.h>
#include "os.h"
#include "awss_main.h"
#include "awss_cmp.h"
#include "json_parser.h"

#include "CoAPExport.h"
#include "CoAPServer.h"

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

static void *g_coap_ctx = NULL;

unsigned char awss_cmp_get_coap_code(void *request)
{
    if (request == NULL)
        return 0x60;
    struct CoAPMessage *msg = (struct CoAPMessage *)request;
    return msg->header.code;
}

char *awss_cmp_get_coap_payload(void *request, int *payload_len)
{
    if (request == NULL)
        return NULL;

    struct CoAPMessage *msg = (struct CoAPMessage *)request;
    if (payload_len)
        *payload_len = msg->payloadlen;
    return (char *)msg->payload;
}

int awss_cmp_coap_register_cb(char *topic, void* cb)
{
    if (g_coap_ctx == NULL)
        g_coap_ctx = CoAPServer_init();

    if (g_coap_ctx == NULL)
        return -1;
    if (topic == NULL)
        return -1;

    CoAPServer_register(g_coap_ctx, (const char *)topic, (CoAPRecvMsgHandler)cb);
    return 0;
}

int awss_cmp_coap_loop(void *param)
{
    if (g_coap_ctx == NULL) g_coap_ctx = CoAPServer_init();
#ifndef COAP_WITH_YLOOP
    awss_debug("create thread\r\n");
    CoAPServer_loop(g_coap_ctx);
#endif
    return 0;
}

int awss_cmp_coap_send(void *buf, unsigned int len, platform_netaddr_t *sa, const char *uri, void *cb, unsigned short *msgid)
{
    if (g_coap_ctx == NULL) {
        g_coap_ctx = CoAPServer_init();
    } else {
        awss_debug("cancal msg id:%u\r\n", *msgid);
        CoAPMessageId_cancel(g_coap_ctx, *msgid);
    }
    return CoAPServerMultiCast_send(g_coap_ctx, (NetworkAddr *)sa, uri, (unsigned char *)buf,
                                    (unsigned short)len, (CoAPSendMsgHandler)cb, msgid);
}

int awss_cmp_coap_send_resp(void *buf, unsigned int len, platform_netaddr_t *sa, const char *uri, void *req)
{
    if (g_coap_ctx == NULL) g_coap_ctx = CoAPServer_init();

    return CoAPServerResp_send(g_coap_ctx, (NetworkAddr *)sa, (unsigned char *)buf, (unsigned short)len, req, uri);
}

int awss_cmp_coap_ob_send(void *buf, unsigned int len, platform_netaddr_t *sa, const char *uri, void *cb)
{
    if (g_coap_ctx == NULL) g_coap_ctx = CoAPServer_init();
    return CoAPObsServer_notify(g_coap_ctx, uri, (unsigned char *)buf, (unsigned short)len, cb);
}

int awss_cmp_coap_deinit()
{
    if (g_coap_ctx) CoAPServer_deinit(g_coap_ctx);
    g_coap_ctx = NULL;
    return 0;
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif

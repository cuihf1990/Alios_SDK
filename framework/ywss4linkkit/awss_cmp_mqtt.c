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
#include "mqtt_instance.h"
#include "awss_main.h"
#include "awss_cmp.h"

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

static char user_data;

int awss_cmp_mqtt_register_cb(char *topic, void* cb)
{
    if (topic == NULL)
        return -1;

    return mqtt_subscribe(topic, (void (*)(char *, int, void *, int, void *))cb, &user_data);
}

int awss_cmp_mqtt_unregister_cb(char *topic)
{
    return mqtt_unsubscribe(topic);
}

int awss_cmp_mqtt_send(char *topic, void *data, int len)
{
    awss_debug("%s\r\n", __func__);
    return mqtt_publish(topic, 1, data, len);  // IOTX_MQTT_QOS1
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif

/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef _TFS_HTTP_H
#define _TFS_HTTP_H

#ifdef __cplusplus
extern "C" {
#endif

#define ID2_GET_SEED     "/getSeed"
#define ID2_ACTIVATE_DEV "/returnId2"

int http_get_seed(const char *func, const char *arg, char *seed);
int http_activate_dev(const char *func, const char *arg);

#ifdef __cplusplus
}
#endif

#endif

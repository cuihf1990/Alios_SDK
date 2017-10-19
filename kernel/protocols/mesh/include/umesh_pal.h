/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef UMESH_PAL_H
#define UMESH_PAL_H

int umesh_pal_kv_get(const char *key, void *buf, int *len);
void *umesh_pal_malloc(int sz);
void umesh_pal_free(void *);
uint32_t umesh_pal_now_ms(void);
void umesh_pal_post_event(int code, unsigned long value);
void umesh_pal_log(const char *fmt, ...);

#endif

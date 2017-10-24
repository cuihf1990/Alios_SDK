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

typedef long pal_sem_hdl_t;
int umesh_pal_sem_new(pal_sem_hdl_t *hdl, int count);
int umesh_pal_sem_wait(pal_sem_hdl_t *hdl, int ms);
void umesh_pal_sem_signal(pal_sem_hdl_t *hdl);
void umesh_pal_sem_free(pal_sem_hdl_t *hdl);
int umesh_pal_schedule_call(void (*task)(void *), void *arg);

void umesh_pal_init(void);
void umesh_pal_ready(void);
#endif

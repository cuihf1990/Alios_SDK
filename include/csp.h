/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
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

#ifndef K_SYS_CSP_H
#define K_SYS_CSP_H

#include <yos/sysdep.h>

#ifndef weak_attr
#define weak_attr __attribute__ ((weak))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
    void *hdl;
} csp_hdl_t;
typedef csp_hdl_t csp_mutex_t;
typedef csp_hdl_t csp_sem_t;

int      csp_mutex_new(csp_mutex_t *mutex);
uint32_t csp_mutex_lock(csp_mutex_t mutex);
uint32_t csp_mutex_unlock(csp_mutex_t mutex);
void     csp_mutex_free(csp_mutex_t *mutex);

int      csp_sem_new(csp_sem_t *sem, int32_t count);
uint32_t csp_sem_wait(csp_sem_t sem, uint32_t timeout);
void     csp_sem_signal(csp_sem_t sem);
void     csp_sem_free(csp_sem_t *sem);

uint64_t csp_now(void);

int      csp_task_new(const char *name, void (*fn)(void *),
                      void *arg, int stacksize);
int      csp_task_new_ext(const char *name, void (*fn)(void *),
                          void *arg, int stacksize, int prio);
void     csp_task_exit(int code);

int      csp_poll(struct pollfd *pollfds, int nfds, csp_sem_t sem,
                  uint32_t timeout);

int      csp_sys_reset(void);
int      csp_sys_free(uint32_t *free);
int      csp_net_errno(int fd);

int      csp_get_args(const char ***pargv);

#ifdef __cplusplus
}
#endif

#endif /* K_SYS_CSP_H */


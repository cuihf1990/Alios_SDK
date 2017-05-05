/**
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

#include <sys/time.h>
#include <string.h>

#include <yos/kernel.h>
#include <hal/hal.h>
#include <yos/framework.h>
#include <yos/log.h>

#include "debug_mem.h"
#include "vfs.h"

#include "yloop.h"

#define TAG "yloop"

#ifndef timercmp
#define timercmp(a, b, op) ({ \
        unsigned long long _v1 = (a)->tv_sec * 1000000ULL + (a)->tv_usec; \
        unsigned long long _v2 = (b)->tv_sec * 1000000ULL + (b)->tv_usec; \
        _v1 op _v2; \
    })

void timersub(struct timeval *a, struct timeval *b, struct timeval *res)
{
    res->tv_usec = a->tv_usec - b->tv_usec;
    res->tv_sec = a->tv_sec - b->tv_sec;

    if (res->tv_usec < 0) {
        res->tv_usec += 1000000;
        res->tv_sec -= 1;
    }
}
#endif

typedef struct {
    int           sock;
    void         *yloop_data;
    void         *private_data;
    yloop_sock_cb cb;
} yloop_sock_t;

typedef struct yloop_timeout_s {
    struct yloop_timeout_s* next;
    struct timeval          time;
    void                   *yloop_data;
    void                   *private_data;
    yloop_timeout_cb        cb;
    yloop_free_cb           free_cb;
} yloop_timeout_t;

typedef struct {
    struct pollfd   *pollfds;
    yloop_sock_t    *readers;
    yloop_timeout_t *timeout;
    int              eventfd;
    uint8_t          max_sock;
    uint8_t          reader_count;
    bool             pending_terminate;
    bool             terminate;
} yloop_ctx_t;

static yloop_ctx_t    *g_main_ctx;
static yos_task_key_t  g_loop_key;

static inline void _set_context(yloop_ctx_t *ctx)
{
    yos_task_setspecific(g_loop_key, ctx);
}

static inline yloop_ctx_t *_get_context(void)
{
    return yos_task_getspecific(g_loop_key);
}

static inline yloop_ctx_t *get_context(void)
{
    yloop_ctx_t *ctx = _get_context();
    if (!ctx) {
        _set_context(g_main_ctx);
        return g_main_ctx;
    }
    return ctx;
}

void yloop_set_eventfd(int fd)
{
    yloop_ctx_t *ctx = get_context();
    ctx->eventfd = fd;
}

int yloop_get_eventfd(void)
{
    yloop_ctx_t *ctx = get_context();
    return ctx->eventfd;
}

void yloop_init(void)
{
    yloop_ctx_t *ctx = _get_context();

    if (!g_main_ctx)
        yos_task_key_create(&g_loop_key);
    else if (ctx && g_main_ctx != ctx) {
        LOGE(TAG, "yloop already inited");
        return;
    }

    ctx = DEBUG_CALLOC(1, sizeof(*g_main_ctx));
    if (!g_main_ctx)
        g_main_ctx = ctx;

    ctx->eventfd = -1;
    _set_context(ctx);
}

int yloop_register_read_sock(int sock,
        yloop_sock_cb cb, void *yloop_data, void *private_data)
{
    yloop_ctx_t *ctx = get_context();
    if (sock  < 0) {
        return -1;
    }

    yloop_sock_t *new_sock;

    new_sock = DEBUG_REALLOC(ctx->readers,
                     (ctx->reader_count + 1) * sizeof(yloop_sock_t));

    if (new_sock == NULL) {
        return -1;
    }

    int status = yunos_fcntl(sock, F_GETFL, 0);
    yunos_fcntl(sock, F_SETFL, status | O_NONBLOCK);

    new_sock[ctx->reader_count].sock = sock;
    new_sock[ctx->reader_count].yloop_data = yloop_data;
    new_sock[ctx->reader_count].private_data = private_data;
    new_sock[ctx->reader_count].cb = cb;
    ctx->reader_count++;
    ctx->readers = new_sock;

    if (sock > ctx->max_sock) {
        ctx->max_sock = sock;
    }

    struct pollfd *new_loop_pollfds;

    new_loop_pollfds = DEBUG_REALLOC(ctx->pollfds,
                                     ctx->reader_count * sizeof(struct pollfd));
    if (new_loop_pollfds == NULL) {
        LOGE(TAG, "out of memory");
        return -1;
    }

    ctx->pollfds = new_loop_pollfds;

    return 0;
}

void yloop_unregister_read_sock(int sock)
{
    yloop_ctx_t *ctx = get_context();
    if (ctx->readers == NULL || ctx->reader_count == 0) {
        return;
    }

    int i;
    for (i = 0; i < ctx->reader_count; i++) {
        if (ctx->readers[i].sock == sock) {
            break;
        }
    }

    if (i == ctx->reader_count) {
        return;
    }

    if (i != ctx->reader_count - 1) {
        memmove(&ctx->readers[i], &ctx->readers[i + 1],
                (ctx->reader_count - i - 1) *
                sizeof(yloop_sock_t));
    }

    ctx->reader_count--;

    if (ctx->reader_count) {
        struct pollfd *new_loop_pollfds =
                           DEBUG_REALLOC(ctx->pollfds,
                                         ctx->reader_count * sizeof(struct pollfd));
        if (new_loop_pollfds == NULL) {
            LOGE(TAG, "out of memory");
            return;
        }
        ctx->pollfds = new_loop_pollfds;
    } else {
        DEBUG_FREE(ctx->pollfds);
        ctx->pollfds = NULL;
    }
}


int yloop_register_timeout(unsigned int secs, unsigned int usecs,
                           yloop_timeout_cb cb, yloop_free_cb free_cb,
                           void *yloop_data, void *private_data)
{
    yloop_ctx_t *ctx = get_context();
    yloop_timeout_t *timeout = DEBUG_MALLOC(sizeof(*timeout));
    if (timeout == NULL) {
        return -1;
    }

    hal_time_gettimeofday(&timeout->time, NULL);
    timeout->time.tv_sec += secs;
    timeout->time.tv_usec += usecs;

    while (timeout->time.tv_usec >= 1000000) {
        timeout->time.tv_sec++;
        timeout->time.tv_usec -= 1000000;
    }

    timeout->yloop_data = yloop_data;
    timeout->private_data = private_data;
    timeout->cb = cb;
    timeout->free_cb = free_cb;
    timeout->next = NULL;

    if (ctx->timeout == NULL) {
        ctx->timeout = timeout;
        return 0;
    }

    yloop_timeout_t *prev = NULL;
    yloop_timeout_t *tmp = ctx->timeout;

    while (tmp != NULL) {
        if (timercmp(&timeout->time, &tmp->time, <)) {
            break;
        }

        prev = tmp;
        tmp = tmp->next;
    }

    if (prev == NULL) {
        timeout->next = ctx->timeout;
        ctx->timeout = timeout;
    } else {
        timeout->next = prev->next;
        prev->next = timeout;
    }

    return 0;
}

int yloop_cancel_timeout(
        yloop_timeout_cb cb, void *yloop_data, void *private_data)
{
    yloop_ctx_t *ctx = get_context();
    yloop_timeout_t *prev = NULL;
    yloop_timeout_t *timeout = ctx->timeout;

    int removed = 0;
    while (timeout != NULL) {
        yloop_timeout_t *next = timeout->next;

        if (timeout->cb == cb &&
            (timeout->yloop_data == yloop_data ||
             yloop_data == YOC_LOOP_ALL_DATA) &&
            (timeout->private_data == private_data ||
             private_data == YOC_LOOP_ALL_DATA)) {
            if (prev == NULL) {
                ctx->timeout = next;
            } else {
                prev->next = next;
            }

            if (timeout->free_cb != NULL) {
                timeout->free_cb(timeout->yloop_data, timeout->private_data);
            }
            DEBUG_FREE(timeout);
            removed++;
        } else {
            prev = timeout;
        }

        timeout = next;
    }

    return removed;
}

void yloop_run(void)
{
    yloop_ctx_t *ctx = get_context();
    struct timeval tv, now;

    while (!ctx->terminate &&
           (ctx->timeout || ctx->reader_count > 0)) {
        int readers = ctx->reader_count;
        if (ctx->timeout) {
            hal_time_gettimeofday(&now, NULL);

            if (timercmp(&now, &ctx->timeout->time, <)) {
                timersub(&ctx->timeout->time, &now, &tv);
            } else {
                tv.tv_sec = tv.tv_usec = 0;
            }
        }

        for (int i = 0; i < readers; i++) {
            ctx->pollfds[i].fd = ctx->readers[i].sock;
            ctx->pollfds[i].events = POLLIN;
        }

        int res = yunos_poll(ctx->pollfds, readers,
                             ctx->timeout ? (tv.tv_sec * 1000 + tv.tv_usec / 1000) : -1);

        if (res < 0 && errno != EINTR) {
            LOGE(TAG, "yunos_poll");
            return;
        }

        /* check if some registered timeouts have occurred */
        if (ctx->timeout) {
            hal_time_gettimeofday(&now, NULL);

            if (!timercmp(&now, &ctx->timeout->time, <)) {
                yloop_timeout_t *timeout = ctx->timeout;
                ctx->timeout = ctx->timeout->next;
                timeout->cb(timeout->yloop_data, timeout->private_data);
                if (timeout->free_cb != NULL) {
                    timeout->free_cb(timeout->yloop_data, timeout->private_data);
                }
                DEBUG_FREE(timeout);
            }

        }

        if (res <= 0) {
            continue;
        }

        for (int i = 0; i < readers; i++) {
            if (ctx->pollfds[i].revents & POLLIN) {
                ctx->readers[i].cb(
                    ctx->readers[i].sock,
                    ctx->readers[i].yloop_data,
                    ctx->readers[i].private_data);
            }
        }
    }

    ctx->terminate = 0;
}

void yloop_terminate(void)
{
    yloop_ctx_t *ctx = get_context();
    ctx->terminate = 1;
}

void yloop_destroy(void)
{
    yloop_ctx_t *ctx = get_context();
    yloop_timeout_t *timeout = ctx->timeout;
    while (timeout != NULL) {
        yloop_timeout_t *prev = timeout;
        timeout = timeout->next;
        if (prev->free_cb != NULL) {
            prev->free_cb(prev->yloop_data, prev->private_data);
        }
        DEBUG_FREE(prev);
    }

    if (ctx->readers) {
        DEBUG_FREE(ctx->readers);
    }

    _set_context(NULL);
    if (ctx == g_main_ctx)
        g_main_ctx = NULL;
    DEBUG_FREE(ctx);
}

int yloop_terminated(void)
{
    yloop_ctx_t *ctx = get_context();
    return ctx->terminate;
}

static void poll_action(int fd, void *yloop_data, void *private_data);

void yos_poll_read_fd(int fd, yos_call_t action, void *param)
{
    yloop_register_read_sock(fd, poll_action, action, param);
}

void yos_cancel_poll_read_fd(int fd, yos_call_t action, void *param)
{
    yloop_unregister_read_sock(fd);
}

void poll_action(int fd, void *yloop_data, void *private_data)
{
    yos_call_t action = yloop_data;

    action(private_data);
}

void yos_event_loop_run(void)
{
    yloop_run();
}

static void delayed_action(void *yloop_data, void *private_data);

void yos_post_delayed_action(int ms, yos_call_t action, void *param)
{
    yloop_register_timeout(ms / 1000, (ms % 1000) * 1000,
        delayed_action, NULL, action, param);
}

void yos_cancel_delayed_action(yos_call_t action, void *param)
{
    yloop_cancel_timeout(delayed_action, action, param);
}

void delayed_action(void *yloop_data, void *private_data)
{
    yos_call_t action = yloop_data;

    action(private_data);
}


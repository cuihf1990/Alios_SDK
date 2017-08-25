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
#include <stdbool.h>
#include <stdlib.h>

#include <yos/kernel.h>
#include <yos/framework.h>
#include <yos/network.h>
#include <yos/log.h>
#include <yos/list.h>

#include "yloop.h"

#define TAG "yloop"

typedef struct {
    int              sock;
    void            *private_data;
    yos_poll_call_t  cb;
} yloop_sock_t;

typedef struct yloop_timeout_s {
    dlist_t          next;
    long long        timeout_ms;
    void            *private_data;
    yos_call_t       cb;
    int              ms;
} yloop_timeout_t;

typedef struct {
    dlist_t          timeouts;
    struct pollfd   *pollfds;
    yloop_sock_t    *readers;
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

void yos_loop_set_eventfd(int fd)
{
    yloop_ctx_t *ctx = get_context();
    ctx->eventfd = fd;
}

int yos_loop_get_eventfd(void *loop)
{
    yloop_ctx_t *ctx = loop ? loop : get_context();
    return ctx->eventfd;
}

yos_loop_t yos_current_loop(void)
{
    return get_context();
}

yos_loop_t yos_loop_init(void)
{
    yloop_ctx_t *ctx = _get_context();

    if (!g_main_ctx) {
        yos_task_key_create(&g_loop_key);
    } else if (ctx && g_main_ctx != ctx) {
        LOGE(TAG, "yloop already inited");
        return ctx;
    }

    ctx = yos_zalloc(sizeof(*g_main_ctx));
    if (!g_main_ctx) {
        g_main_ctx = ctx;
    }

    dlist_init(&ctx->timeouts);
    ctx->eventfd = -1;
    _set_context(ctx);

    yos_event_service_init();

    return ctx;
}

int yos_poll_read_fd(int sock, yos_poll_call_t cb, void *private_data)
{
    yloop_ctx_t *ctx = get_context();
    if (sock  < 0) {
        return -1;
    }

    yloop_sock_t *new_sock;
    struct pollfd *new_loop_pollfds;
    int cnt = ctx->reader_count + 1;

    new_sock = yos_malloc(cnt * sizeof(yloop_sock_t));
    new_loop_pollfds = yos_malloc(cnt * sizeof(struct pollfd));

    if (new_sock == NULL || new_loop_pollfds == NULL) {
        LOGE(TAG, "out of memory");
        return -1;
    }

    int status = yos_fcntl(sock, F_GETFL, 0);
    yos_fcntl(sock, F_SETFL, status | O_NONBLOCK);

    ctx->reader_count++;

    memcpy(new_sock, ctx->readers, (cnt - 1) * sizeof(yloop_sock_t));
    yos_free(ctx->readers);
    ctx->readers = new_sock;

    memcpy(new_loop_pollfds, ctx->pollfds, (cnt - 1) * sizeof(struct pollfd));
    yos_free(ctx->pollfds);
    ctx->pollfds = new_loop_pollfds;

    new_sock += cnt - 1;
    new_sock->sock = sock;
    new_sock->private_data = private_data;
    new_sock->cb = cb;

    if (sock > ctx->max_sock) {
        ctx->max_sock = sock;
    }

    return 0;
}

void yos_cancel_poll_read_fd(int sock, yos_poll_call_t action, void *param)
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
}


int yos_post_delayed_action(int ms, yos_call_t action, void *param)
{
    if (action == NULL) {
        return -1;
    }

    yloop_ctx_t *ctx = get_context();
    yloop_timeout_t *timeout = yos_malloc(sizeof(*timeout));
    if (timeout == NULL) {
        return -1;
    }

    timeout->timeout_ms = yos_now_ms() + ms;
    timeout->private_data = param;
    timeout->cb = action;
    timeout->ms = ms;

    yloop_timeout_t *tmp;

    dlist_for_each_entry(&ctx->timeouts, tmp, yloop_timeout_t, next) {
        if (timeout->timeout_ms < tmp->timeout_ms) {
            break;
        }
    }

    dlist_add_tail(&timeout->next, &tmp->next);

    return 0;
}

void yos_cancel_delayed_action(int ms, yos_call_t cb, void *private_data)
{
    yloop_ctx_t *ctx = get_context();
    yloop_timeout_t *tmp;

    dlist_for_each_entry(&ctx->timeouts, tmp, yloop_timeout_t, next) {
        if (ms != -1 && tmp->ms != ms) {
            continue;
        }

        if (tmp->cb != cb) {
            continue;
        }

        if (tmp->private_data != private_data) {
            continue;
        }

        dlist_del(&tmp->next);
        yos_free(tmp);
        return;
    }
}

void yos_loop_run(void)
{
    yloop_ctx_t *ctx = get_context();

    while (!ctx->terminate &&
           (!dlist_empty(&ctx->timeouts) || ctx->reader_count > 0)) {
        int delayed_ms = -1;
        int readers = ctx->reader_count;
        int i;

        if (!dlist_empty(&ctx->timeouts)) {
            yloop_timeout_t *tmo = dlist_first_entry(&ctx->timeouts, yloop_timeout_t, next);
            long long now = yos_now_ms();

            if (now < tmo->timeout_ms) {
                delayed_ms = tmo->timeout_ms - now;
            } else {
                delayed_ms = 0;
            }
        }

        for (i = 0; i < readers; i++) {
            ctx->pollfds[i].fd = ctx->readers[i].sock;
            ctx->pollfds[i].events = POLLIN;
        }

        int res = yos_poll(ctx->pollfds, readers, delayed_ms);

        if (res < 0 && errno != EINTR) {
            LOGE(TAG, "yos_poll");
            return;
        }

        /* check if some registered timeouts have occurred */
        if (!dlist_empty(&ctx->timeouts)) {
            yloop_timeout_t *tmo = dlist_first_entry(&ctx->timeouts, yloop_timeout_t, next);
            long long now = yos_now_ms();

            if (now >= tmo->timeout_ms) {
                dlist_del(&tmo->next);
                tmo->cb(tmo->private_data);
                yos_free(tmo);
            }
        }

        if (res <= 0) {
            continue;
        }

        for (i = 0; i < readers; i++) {
            if (ctx->pollfds[i].revents & POLLIN) {
                ctx->readers[i].cb(
                    ctx->readers[i].sock,
                    ctx->readers[i].private_data);
            }
        }
    }

    ctx->terminate = 0;
}

void yos_loop_exit(void)
{
    yloop_ctx_t *ctx = get_context();
    ctx->terminate = 1;
}

void yos_loop_destroy(void)
{
    yloop_ctx_t *ctx = _get_context();

    if (ctx == NULL) {
        return;
    }

    yos_event_service_deinit(ctx->eventfd);

    while (!dlist_empty(&ctx->timeouts)) {
        yloop_timeout_t *timeout = dlist_first_entry(&ctx->timeouts, yloop_timeout_t,
                                                     next);
        dlist_del(&timeout->next);
        yos_free(timeout);
    }

    yos_free(ctx->readers);
    yos_free(ctx->pollfds);

    _set_context(NULL);
    if (ctx == g_main_ctx) {
        g_main_ctx = NULL;
    }
    yos_free(ctx);
}

/*
* Copyright (C) 2016 YunOS Project. All rights reserved.
*/
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

#include <k_api.h>
#include <yos/kernel.h>
#include <yos/framework.h>
#include <yos/list.h>

#include "debug_mem.h"
#include "vfs.h"
#include "yloop.h"

typedef struct {
    dlist_t       node;
    yos_event_cb  cb;
    void         *private_data;
} local_event_list_node_t;

static struct {
    void *handle;
    int   fd;
} local_event = {
    .fd = -1,
};

static dlist_t g_local_event_list = YOS_DLIST_INIT(g_local_event_list);

static void handle_events(input_event_t *event);
static int  input_add_event(int fd, input_event_t *event);
static void event_read_cb(int fd, void *param);

/* Handle events
 * just dispatch
 */
void handle_events(input_event_t *event)
{
    if (event->type == EV_RPC) {
        yos_call_t handler = (yos_call_t)event->value;
        void *arg = (void *)event->extra;
        handler(arg);

        return;
    }

    local_event_list_node_t *event_node = NULL;
    dlist_for_each_entry(&g_local_event_list, event_node,
                        local_event_list_node_t, node) {
        (event_node->cb)(event, event_node->private_data);
    }
}

static int input_add_event(int fd, input_event_t *event)
{
    return yunos_write(fd, event, sizeof(*event));
}

void event_read_cb(int fd, void *param)
{
    input_event_t event;
    int ret = yunos_read(fd, &event, sizeof(event));
    if (ret == sizeof(event)) {
        handle_events(&event);
    }
}

int yos_event_service_init(void)
{
    int fd = yunos_open("/dev/event", 0);

    if (local_event.fd < 0)
        local_event.fd = fd;
    yos_poll_read_fd(fd, event_read_cb, NULL);
    yos_loop_set_eventfd(fd);

    return 0;
}

int yos_event_service_deinit(int fd)
{
    yos_cancel_poll_read_fd(fd, event_read_cb, NULL);
}

int yos_local_event_post(uint16_t type, uint16_t code, unsigned long value)
{
    input_event_t event = {
        .type  = type,
        .code  = code,
        .value = value,
    };

    return input_add_event(local_event.fd, &event);
}

void yos_local_event_listener_register(yos_event_cb cb, void *priv)
{
    local_event_list_node_t* event_node = DEBUG_MALLOC(sizeof(local_event_list_node_t));
    if(NULL == event_node){
        return;
    }

    event_node->cb           = cb;
    event_node->private_data = priv;

    dlist_add_tail(&event_node->node, &g_local_event_list);
}

void yos_local_event_listener_unregister(yos_event_cb cb, void *priv)
{
    local_event_list_node_t* event_node = NULL;
    dlist_for_each_entry(&g_local_event_list, event_node, local_event_list_node_t, node) {
        if(event_node->cb == cb && event_node->private_data == priv) {
            dlist_del(&event_node->node);
            DEBUG_FREE(event_node);
            return;
        }
    }
}

/*
 * schedule a callback in yos loop main thread
 */
int yos_loop_schedule_call(yos_loop_t *loop, yos_call_t fun, void *arg)
{
    input_event_t event = {
        .type = EV_RPC,
        .value = (unsigned long)fun,
        .extra = (unsigned long)arg,
    };
    int fd = yos_loop_get_eventfd(loop);
    if (fd < 0)
        fd = local_event.fd;

    return input_add_event(fd, &event);
}

int yos_schedule_call(yos_call_t fun, void *arg)
{
    return yos_loop_schedule_call(NULL, fun, arg);
}

typedef struct work_para {
    work_t *work;
    yos_loop_t loop;
    yos_call_t action;
    void *arg1;
    yos_call_t fini_cb;
    void *arg2;
} work_par_t;

static void run_my_work(void *arg)
{
    work_par_t *wpar = arg;

    wpar->action(wpar->arg1);

    yos_loop_schedule_call(wpar->loop, wpar->fini_cb, wpar->arg2);

    free(wpar->work);
    free(wpar);
}

int yos_schedule_work(yos_call_t action, void *arg1, yos_call_t fini_cb, void *arg2)
{
    static workqueue_t *wq;

    if (!wq) {
        wq = malloc(sizeof(*wq));
#define WQ_STACK_SIZE 8192
        void *stack = malloc(WQ_STACK_SIZE);
        yunos_workqueue_create(wq, "loop", 9, stack, WQ_STACK_SIZE / 4);
    }

    work_t *work = malloc(sizeof(*work));
    work_par_t *wpar = malloc(sizeof(*wpar));

    wpar->work = work;
    wpar->loop = yos_current_loop();
    wpar->action = action;
    wpar->arg1 = arg1;
    wpar->fini_cb = fini_cb;
    wpar->arg2 = arg2;

    yunos_work_init(work, run_my_work, wpar, 0);
    yunos_work_run(wq, work);
}


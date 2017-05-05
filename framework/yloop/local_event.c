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

#include <yos/framework.h>
#include <yos/list.h>

#include "debug_mem.h"
#include "vfs.h"
#include "yloop.h"

typedef struct {
    dlist_t       node;
    yos_event_cb  cb;
    yos_free_cb   free_cb;
    void         *private_data;
} local_event_list_node_t;

static struct {
    int   idx;
    int   fd;
} eventfd = {
    .fd = -1,
};

static dlist_t g_local_event_list = YOS_DLIST_INIT(g_local_event_list);

static void handle_events(input_event_t *event);
static int  input_add_event(input_event_t *event);
static void event_read_cb(int fd, void *yloop_data, void *param);

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

int input_add_event(input_event_t *event)
{
    int fd = yloop_get_eventfd();
    if (fd < 0)
        fd = eventfd.fd;
    return yunos_write(fd, event, sizeof(*event));
}

void event_read_cb(int fd, void *yloop_data, void *param)
{
    input_event_t event;
    int ret = yunos_read(fd, &event, sizeof(event));
    if (ret == sizeof(event)) {
        handle_events(&event);
    }
}

int local_event_service_init(void)
{
    char name[32];
    int fd;

    snprintf(name, sizeof(name)-1, "/dev/event%d", eventfd.idx ++);
    fd = yunos_open(name, 0);

    if (eventfd.fd < 0)
        eventfd.fd = fd;
    yloop_register_read_sock(fd, event_read_cb, NULL, NULL);
    yloop_set_eventfd(fd);

    return fd;
}

int yos_local_event_post(uint16_t type, uint16_t code, unsigned long value)
{
    input_event_t event = {
        .type  = type,
        .code  = code,
        .value = value,
    };

    return input_add_event(&event);
}

void yos_local_event_listener_register(yos_event_cb cb,
                                       yos_free_cb free_cb,
                                       void *private_data)
{
    local_event_list_node_t* event_node = DEBUG_MALLOC(sizeof(local_event_list_node_t));
    if(NULL == event_node){
        return;
    }

    event_node->cb           = cb;
    event_node->free_cb      = free_cb;
    event_node->private_data = private_data;

    dlist_add_tail(&event_node->node, &g_local_event_list);
}

void yos_local_event_listener_unregister(yos_event_cb cb)
{
    local_event_list_node_t* event_node = NULL;
    dlist_for_each_entry(&g_local_event_list, event_node, local_event_list_node_t, node) {
        if(event_node->cb == cb) {
            dlist_del(&event_node->node);
            if (event_node->free_cb != NULL) {
                event_node->free_cb(event_node->private_data);
            }
            DEBUG_FREE(event_node);
            return;
        }
    }
}

/*
 * schedule a callback in yoc loop main thread
 */
int yos_schedule_call(yos_call_t fun, void *arg)
{
    input_event_t event = {
        .type = EV_RPC,
        .value = (unsigned long)fun,
        .extra = (unsigned long)arg,
    };

    return input_add_event(&event);
}


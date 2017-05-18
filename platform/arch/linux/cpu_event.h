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

#ifndef CPU_EVENT_H
#define CPU_EVENT_H

typedef void (*cpu_event_handler)(const void *);

typedef struct {
    cpu_event_handler handler;
    const void       *arg;
} cpu_event_t;

#ifdef HAVE_RHINO_KERNEL
int cpu_notify_event(cpu_event_t *event);
void *cpu_event_malloc(int size);
void cpu_event_free(void *p);
#else
static inline int cpu_notify_event(cpu_event_t *event)
{
    event->handler(event->arg);
    return 0;
}
#define cpu_event_malloc malloc
#define cpu_event_free free
#endif

static inline int cpu_call_handler(cpu_event_handler handler, const void *arg)
{
    cpu_event_t event = {
        .handler = handler,
        .arg = arg,
    };
    return cpu_notify_event(&event);
}

#endif /* CPU_EVENT_H */


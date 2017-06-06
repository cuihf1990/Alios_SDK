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

#ifndef RINGBUF_TEST_H
#define RINGBUF_TEST_H

#define TASK_RINGBUF_PRI   16
#define TASK_TEST_STACK_SIZE 1024

#define MYASSERT(value) do {if ((value) == 0) { printf("%s:%d\n", __FUNCTION__, __LINE__); }} while (0)

extern ktask_t *task_ringbuf;
extern ktask_t *task_ringbuf_co;

typedef uint8_t (*test_func_t)(void);

void task_ringbuf_entry_register(const char *name, test_func_t *runner,
                                 uint8_t casenum);
void task_ringbuf_entry(void *arg);
void ringbuf_test(void);
void ringbuf_break_test(void);

#endif /* RINGBUF_TEST_H */


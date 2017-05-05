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

#ifndef TIME_TEST_H
#define TIME_TEST_H

#define TASK_TIME_PRI          16
#define TASK_TEST_STACK_SIZE 1024

#define MYASSERT(value) do {if ((int)(value) == 0) { return 1; }} while (0)

extern ktask_t *task_time;

typedef uint8_t (*test_func_t)(void);

void task_time_entry_register(const char *name, test_func_t *runner, uint8_t casenum);
void task_time_entry(void *arg);
void time_test(void);
void time_opr_test(void);

#endif /* TIME_TEST_H */


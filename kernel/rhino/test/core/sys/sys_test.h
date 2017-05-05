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

#ifndef SYS_TEST_H
#define SYS_TEST_H

#define TASK_SYS_PRI         16
#define TASK_TEST_STACK_SIZE 1024

#define MYASSERT(value) do { printf("ERROR: %s:%d, ERROR-NO: %d\n", __FUNCTION__, __LINE__, value); } while (0)

extern ktask_t *task_sys;

typedef uint8_t (*test_func_t)(void);

void task_sys_entry_register(const char *name, test_func_t *runner, uint8_t casenum);
void task_sys_entry(void *arg);
void sys_test(void);
void sys_opr_test(void);

#endif /* SYS_TEST_H */


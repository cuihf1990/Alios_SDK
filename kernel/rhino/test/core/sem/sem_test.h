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

#ifndef SEM_TEST_H
#define SEM_TEST_H

#define TASK_SEM_PRI         16
#define TASK_TEST_STACK_SIZE 1024

#define MYASSERT(value) do {if ((int)(value) == 0) { return 1; }} while (0)

extern ktask_t *task_sem;
extern ktask_t *task_sem_co1;
extern ktask_t *task_sem_co2;
extern ksem_t  *test_sem;
extern ksem_t  test_sem_ext;
extern ksem_t  *test_sem_co1;
extern ksem_t  *test_sem_co2;

typedef uint8_t (*test_func_t)(void);

void task_sem_entry_register(const char *name, test_func_t *runner, uint8_t casnum);
void task_sem_entry(void *arg);
void sem_test(void);
void sem_param_test(void);
void sem_break_test(void);
void sem_reinit_test(void);
void sem_count_test(void);
void sem_opr_test(void);
void sem_coopr1_test(void);
void sem_coopr2_test(void);

#endif /* SEM_TEST_H */


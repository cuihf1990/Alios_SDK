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

#ifndef MM_TEST_H
#define MM_TEST_H

#define TASK_MM_PRI          16
#define TASK_TEST_STACK_SIZE 1024
#define MM_POOL_SIZE         1024 * 10

#define MYASSERT(value) do {if ((int)(value) == 0) { return 1; }} while (0)

#if (YUNOS_CONFIG_MM_TLF > 0)
extern ktask_t *task_mm;
extern ktask_t *task_mm_co;
extern k_mm_head *pmmhead;
extern char mm_pool[MM_POOL_SIZE * 4];

typedef uint8_t (*test_func_t)(void);

void task_mm_entry_register(const char *name, test_func_t *runner, uint8_t casenum);
void task_mm_entry(void *arg);
void mm_test(void);
void mm_param_test(void);
//void mm_reinit_test(void);
void mm_break_test(void);
void mm_opr_test(void);
void mm_coopr_test(void);
#endif
#endif /* MM_TEST_H */


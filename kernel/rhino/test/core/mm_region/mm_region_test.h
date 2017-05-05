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

#ifndef MM_TEST_REGION_H
#define MM_TEST_REGION_H

#define TASK_MM_REGION_PRI   16
#define TASK_TEST_STACK_SIZE 1024

#define MM_REGION_TOTAL_SIZE 2000
#define MM_REGION_0_SIZE 8
#define MM_REGION_1_SIZE 300
#define MM_REGION_2_SIZE 500
#define MM_REGION_MAX_SIZE 500
#define MM_REGION_VALID_REGION 2
#define MM_REGION_INVALID_REGION 1


#define MYASSERT(value) do {if ((int)(value) == 0) { printf("%s:%d\n", __FUNCTION__, __LINE__); }} while (0)

extern ktask_t *task_mm_region;
extern ktask_t *task_mm_region_co;
extern k_mm_region_t regions[];
extern k_mm_region_t regions2[];

typedef uint8_t (*test_func_t)(void);

void task_mm_region_entry_register(const char *name, test_func_t *runner, uint8_t casenum);
void task_mm_region_entry(void *arg);
void mm_region_test(void);
void mm_region_break_test(void);

#endif /* MM_TEST_REGION_H */


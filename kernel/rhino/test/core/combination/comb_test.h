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

#ifndef COMB_TEST_H
#define COMB_TEST_H

#define TASK_COMB_PRI         16
#define TASK_TEST_STACK_SIZE 256

void comb_test(void);
void sem_queue_coopr_test(void);
void sem_event_coopr_test(void);
void sem_buf_queue_coopr_test(void);
void sem_mutex_coopr_test(void);
void comb_all_coopr_test(void);
void mutex_queue_coopr_test(void);
void mutex_buf_queue_coopr_test(void);

#endif /* SEM_TEST_H */


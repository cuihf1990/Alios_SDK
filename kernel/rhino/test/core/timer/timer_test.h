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

#ifndef TIMER_TEST_H
#define TIMER_TEST_H

#define TASK_TEST_STACK_SIZE 256

#define TIMER_VAL_CHK(value) do {if ((int)(value) == 0) \
        {printf("timer test is [FAIL %d], func %s, line %d\n", \
                (int)++test_case_fail,__FUNCTION__, __LINE__);}}while(0)

kstat_t task_timer_create_del_test(void);
kstat_t task_timer_dyn_create_del_test(void);
kstat_t task_timer_start_stop_test(void);
kstat_t task_timer_change_test(void);

#endif /* TIMER_TEST_H */

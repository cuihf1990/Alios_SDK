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

#ifndef YTS_H
#define YTS_H

void yts_run(int argc, char **argv);

void osupdate_online_test_run(char *bin, int id2);

#define check_cond_wait(cond, seconds) do { \
    unsigned int i; \
    for (i=0;i<(unsigned int)seconds && !(cond);i++) { \
        yos_msleep(1000); \
    } \
    YUNIT_ASSERT(cond); \
} while(0);

#define run_times(func, times) do { \
    int i; \
    for (i=0;i<times;i++, func); \
} while(0);

#endif /* YTS_H */


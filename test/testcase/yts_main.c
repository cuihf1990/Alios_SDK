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

#include <stdlib.h>
#include <sys/wait.h>

#include <yos/kernel.h>

#include "yunit.h"
#include "yts.h"

extern void test_basic(void);
extern void test_rhino(void);
extern void test_rhino_port(void);
extern void test_csp(void);
extern void test_vfs(void);
extern void test_crypto(void);
extern void test_time(void);
extern void test_flash(void);
extern void test_wifi(void);
extern void test_audio(void);
extern void test_mesh(void);
extern void test_netconf(void);
extern void test_uradar(void);
extern void test_yloop(void);
extern void test_cjson(void);
extern void test_hal(void);

static void add_test(void);

void yts_run(int argc, char **argv)
{
    yos_msleep(2 * 1000);

    yunit_test_init();

    add_test();

    int ret = 0;

    if (argc > 1) {
        int i;
        for (i = 1; i < argc; i++) {
            yunit_test_suite_t *test_suite = yunit_get_test_suite(argv[i]);
            if (test_suite != NULL) {
                ret = yunit_run_test_suite(test_suite);
                printf("suite %s completed with %d\n", argv[i], ret);

                continue;
            }

            const char *suite_case = argv[i];
            char *test = strrchr(suite_case, ':');
            if (test != NULL) {
                *test++ = '\0';

                test_suite = yunit_get_test_suite(suite_case);
                yunit_test_case_t *test_case = yunit_get_test_case(test_suite, test);
                if (test_case != NULL) {
                    ret = yunit_run_test_case(test_suite, test_case);
                    printf("suite %s completed with %d\n", argv[i], ret);
                } else {
                    printf("test suite %s not found\n", argv[i]);
                }
            }
        }
    } else {
        ret = yunit_test_run();
        printf("\nTests completed with return value %d\n", ret);
    }

    yunit_test_print_result();

    yunit_test_deinit();
}

void add_test(void)
{
    test_basic();

    test_yloop();

    test_hal();

    test_cjson();
}

#include <k_api.h>
#include <test_fw.h>
#include "sys_test.h"

#define MODULE_NAME "sys_opr"

static uint8_t sys_opr_case1(void)
{
    kstat_t       ret;
    const name_t *ptr;

    ptr = yunos_version_get();
    if (strcmp(ptr, YUNOS_VERSION) != 0) {
        return 1;
    }

    ret = yunos_start();
    if (ret != YUNOS_RUNNING) {
        MYASSERT(ret);
        return 1;
    }

    ret = yunos_sched_disable();
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        return 1;
    }

    /* sched disable (out intrpt) */
    ret = yunos_sched_disable();
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        return 1;
    }

    ret = yunos_sched_enable();
    if (ret != YUNOS_SCHED_DISABLE) {
        MYASSERT(ret);
        return 1;
    }

    ret = yunos_sched_enable();
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        return 1;
    }

    ret = yunos_sched_enable();
    if (ret != YUNOS_SCHED_ALREADY_ENABLED) {
        MYASSERT(ret);
        return 1;
    }

    return 0;
}

static const test_func_t sys_func_runner[] = {
    sys_opr_case1,
    NULL
};

void sys_opr_test(void)
{
    kstat_t ret;

    task_sys_entry_register(MODULE_NAME, (test_func_t *)sys_func_runner,
                            sizeof(sys_func_runner) / sizeof(test_func_t));

    ret = yunos_task_dyn_create(&task_sys, MODULE_NAME, 0, TASK_SYS_PRI,
                                0, TASK_TEST_STACK_SIZE, task_sys_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}


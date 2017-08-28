#include <k_api.h>
#include <test_fw.h>
#include "mutex_test.h"

#define MODULE_NAME "mutex_reinit"

static uint8_t mutex_reinit_case(void)
{
    kstat_t ret;

    ret = yunos_mutex_create(&test_mutex, MODULE_NAME);
    MYASSERT(ret == YUNOS_SUCCESS);

    test_mutex.blk_obj.obj_type = YUNOS_SEM_OBJ_TYPE;
    ret = yunos_mutex_lock(&test_mutex, YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_KOBJ_TYPE_ERR);

    test_mutex.blk_obj.obj_type = YUNOS_MUTEX_OBJ_TYPE;
    ret = yunos_mutex_lock(&test_mutex, YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_SUCCESS);

    test_mutex.blk_obj.obj_type = YUNOS_SEM_OBJ_TYPE;
    ret = yunos_mutex_unlock(&test_mutex);
    MYASSERT(ret == YUNOS_KOBJ_TYPE_ERR);

    test_mutex.blk_obj.obj_type = YUNOS_MUTEX_OBJ_TYPE;
    ret = yunos_mutex_unlock(&test_mutex);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_mutex_del(&test_mutex);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static const test_func_t mutex_func_runner[] = {
    mutex_reinit_case,
    NULL
};

void mutex_reinit_test(void)
{
    kstat_t ret;

    task_mutex_entry_register(MODULE_NAME, (test_func_t *)mutex_func_runner,
                              sizeof(mutex_func_runner) / sizeof(test_func_t));

    ret = yunos_task_dyn_create(&task_mutex, MODULE_NAME, 0, TASK_MUTEX_PRI,
                                0, TASK_TEST_STACK_SIZE, task_mutex_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}


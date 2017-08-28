#include <k_api.h>
#include <test_fw.h>
#include "sem_test.h"

#define MODULE_NAME "sem_count"

static uint8_t sem_count_case1(void)
{
    kstat_t     ret;
    sem_count_t cnt;

    ret = yunos_sem_dyn_create(&test_sem, MODULE_NAME, 3);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_sem->count == 3);
    MYASSERT(test_sem->peak_count == 3);

    ret = yunos_sem_count_get(test_sem, &cnt);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(cnt == 3);

    ret = yunos_sem_take(test_sem, YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_sem_count_get(test_sem, &cnt);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(cnt == 2);

    test_sem->blk_obj.obj_type = YUNOS_MUTEX_OBJ_TYPE;
    ret = yunos_sem_count_set(test_sem, 8);
    MYASSERT(ret == YUNOS_KOBJ_TYPE_ERR);

    test_sem->blk_obj.obj_type = YUNOS_SEM_OBJ_TYPE;
    ret = yunos_sem_count_set(test_sem, 8);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_sem_count_get(test_sem, &cnt);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(cnt == 8);

    ret = yunos_sem_dyn_del(test_sem);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static const test_func_t sem_func_runner[] = {
    sem_count_case1,
    NULL
};

void sem_count_test(void)
{
    kstat_t ret;

    task_sem_entry_register(MODULE_NAME, (test_func_t *)sem_func_runner,
                            sizeof(sem_func_runner) / sizeof(test_case_t));

    ret = yunos_task_dyn_create(&task_sem, MODULE_NAME, 0, TASK_SEM_PRI,
                                0, TASK_TEST_STACK_SIZE, task_sem_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}


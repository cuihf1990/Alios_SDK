#include <k_api.h>
#include <test_fw.h>
#include "sem_test.h"

#define MODULE_NAME "sem_param"

static uint8_t sem_param_case1(void)
{
    kstat_t ret;

    ret = yunos_sem_dyn_create(NULL, MODULE_NAME, 0);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_sem_dyn_create(&test_sem, NULL, 0);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_sem_dyn_create(&test_sem, MODULE_NAME, (sem_count_t) - 1);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_sem_dyn_create(&test_sem, MODULE_NAME, 0);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_sem_dyn_del(NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    test_sem->mm_alloc_flag = K_OBJ_STATIC_ALLOC;
    ret = yunos_sem_dyn_del(test_sem);
    MYASSERT(ret == YUNOS_KOBJ_DEL_ERR);

    test_sem->mm_alloc_flag = K_OBJ_DYN_ALLOC;
    ret = yunos_sem_dyn_del(test_sem);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_sem_create(&test_sem_ext, MODULE_NAME, 0);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_sem_del(NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    test_sem_ext.blk_obj.obj_type = YUNOS_MUTEX_OBJ_TYPE;
    ret = yunos_sem_del(&test_sem_ext);
    MYASSERT(ret == YUNOS_KOBJ_TYPE_ERR);

    test_sem_ext.blk_obj.obj_type = YUNOS_SEM_OBJ_TYPE;
    test_sem_ext.mm_alloc_flag = K_OBJ_DYN_ALLOC;
    ret = yunos_sem_del(&test_sem_ext);
    MYASSERT(ret == YUNOS_KOBJ_DEL_ERR);

    test_sem_ext.mm_alloc_flag = K_OBJ_STATIC_ALLOC;
    ret = yunos_sem_del(&test_sem_ext);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static uint8_t sem_param_case2(void)
{
    kstat_t ret;
    CPSR_ALLOC();
    sem_count_t count_old;

    ret = yunos_sem_dyn_create(&test_sem, MODULE_NAME, 0);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_sem_take(NULL, YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_sem_give(NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    yunos_sched_disable();
    ret = yunos_sem_take(test_sem, YUNOS_WAIT_FOREVER);
    yunos_sched_enable();
    MYASSERT(ret == YUNOS_SCHED_DISABLE);

    YUNOS_CRITICAL_ENTER();
    count_old = test_sem->count;
    test_sem->count = -1;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_sem_give(test_sem);
    MYASSERT(ret == YUNOS_SEM_OVF);

    YUNOS_CRITICAL_ENTER();
    test_sem->count = count_old;
    YUNOS_CRITICAL_EXIT();

    YUNOS_CRITICAL_ENTER();
    test_sem->blk_obj.obj_type = YUNOS_MUTEX_OBJ_TYPE;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_sem_take(test_sem, YUNOS_WAIT_FOREVER);
    MYASSERT(ret == YUNOS_KOBJ_TYPE_ERR);

    YUNOS_CRITICAL_ENTER();
    test_sem->blk_obj.obj_type = YUNOS_SEM_OBJ_TYPE;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_sem_give_all(NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_sem_dyn_del(test_sem);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static uint8_t sem_param_case3(void)
{
    kstat_t ret;

    ret = yunos_sem_dyn_create(&test_sem, MODULE_NAME, 0);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_sem_dyn_del(test_sem);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static uint8_t sem_param_case4(void)
{
    kstat_t     ret;
    sem_count_t count;

    ret = yunos_sem_dyn_create(&test_sem, MODULE_NAME, 0);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_sem_count_get(NULL, &count);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_sem_count_get(test_sem, NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_sem_count_set(NULL, 2);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_sem_count_set(test_sem, (sem_count_t) - 1);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_sem_dyn_del(test_sem);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static const test_func_t sem_func_runner[] = {
    sem_param_case1,
    sem_param_case2,
    sem_param_case3,
    sem_param_case4,
    NULL
};

void sem_param_test(void)
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


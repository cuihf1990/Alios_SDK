#include <k_api.h>
#include <test_fw.h>
#include "event_test.h"

#define MODULE_NAME "event_param"

#define TEST_FLAG   0x5a5a5a5a

static uint8_t event_param_case1(void)
{
    kstat_t ret;
    CPSR_ALLOC();

    ret = yunos_event_create(NULL, MODULE_NAME, TEST_FLAG);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_event_create(&test_event, NULL, TEST_FLAG);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_event_create(&test_event, MODULE_NAME, TEST_FLAG);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_event_del(NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    YUNOS_CRITICAL_ENTER();
    test_event.mm_alloc_flag = K_OBJ_DYN_ALLOC;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_event_del(&test_event);
    MYASSERT(ret == YUNOS_KOBJ_DEL_ERR);

    YUNOS_CRITICAL_ENTER();
    test_event.mm_alloc_flag = K_OBJ_STATIC_ALLOC;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_event_del(&test_event);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_event_dyn_create(NULL, MODULE_NAME, TEST_FLAG);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_event_dyn_create(&test_event_ext, NULL, TEST_FLAG);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_event_dyn_create(&test_event_ext, MODULE_NAME, TEST_FLAG);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_event_dyn_del(NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    YUNOS_CRITICAL_ENTER();
    test_event_ext->blk_obj.obj_type = YUNOS_SEM_OBJ_TYPE;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_event_dyn_del(test_event_ext);
    MYASSERT(ret == YUNOS_KOBJ_TYPE_ERR);

    YUNOS_CRITICAL_ENTER();
    test_event_ext->blk_obj.obj_type = YUNOS_EVENT_OBJ_TYPE;
    YUNOS_CRITICAL_EXIT();

    YUNOS_CRITICAL_ENTER();
    test_event_ext->mm_alloc_flag = K_OBJ_STATIC_ALLOC;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_event_dyn_del(test_event_ext);
    MYASSERT(ret == YUNOS_KOBJ_DEL_ERR);

    YUNOS_CRITICAL_ENTER();
    test_event_ext->mm_alloc_flag = K_OBJ_DYN_ALLOC;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_event_dyn_del(test_event_ext);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static uint8_t event_param_case2(void)
{
    kstat_t  ret;
    uint32_t actl_flags;

    ret = yunos_event_create(&test_event, MODULE_NAME, TEST_FLAG);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_event_get(NULL, 0, YUNOS_AND, &actl_flags, YUNOS_WAIT_FOREVER);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_event_get(&test_event, 0, YUNOS_AND, NULL, YUNOS_WAIT_FOREVER);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_event_get(&test_event, 0, 0xff, &actl_flags, YUNOS_WAIT_FOREVER);
    MYASSERT(ret == YUNOS_NO_THIS_EVENT_OPT);

    ret = yunos_event_set(NULL, TEST_FLAG, YUNOS_OR);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_event_set(&test_event, TEST_FLAG, 0xff);
    MYASSERT(ret == YUNOS_NO_THIS_EVENT_OPT);

    ret = yunos_event_del(&test_event);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static const test_func_t event_func_runner[] = {
    event_param_case1,
    event_param_case2,
    NULL
};

void event_param_test(void)
{
    kstat_t ret;

    task_event_entry_register(MODULE_NAME, (test_func_t *)event_func_runner,
                              sizeof(event_func_runner) / sizeof(test_func_t));

    ret = yunos_task_dyn_create(&task_event, MODULE_NAME, 0, TASK_EVENT_PRI,
                                0, TASK_TEST_STACK_SIZE, task_event_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}


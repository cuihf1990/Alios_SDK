#include <k_api.h>
#include <test_fw.h>
#include "mm_blk_test.h"

#define MODULE_NAME "mm_blk_break"

static uint8_t mm_blk_break_case1(void)
{
    void   *ptr;
    kstat_t ret;

    ret = yunos_mblk_pool_init(&mblk_pool_test, MODULE_NAME, (void *)mblk_pool,
                               MBLK_POOL_SIZE >> 2, MBLK_POOL_SIZE);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(mblk_pool_test.obj_type == YUNOS_MM_BLK_OBJ_TYPE);

    /* check mblk pool object type after change it */
    mblk_pool_test.obj_type = YUNOS_MM_OBJ_TYPE;
    ret = yunos_mblk_alloc(&mblk_pool_test, &ptr);
    MYASSERT(ret == YUNOS_KOBJ_TYPE_ERR);

    mblk_pool_test.obj_type = YUNOS_MM_BLK_OBJ_TYPE;
    ret = yunos_mblk_alloc(&mblk_pool_test, &ptr);
    MYASSERT(ret == YUNOS_SUCCESS);

    /* check mblk pool object type after change it */
    mblk_pool_test.obj_type = YUNOS_MM_OBJ_TYPE;
    ret = yunos_mblk_free(&mblk_pool_test, ptr);
    MYASSERT(ret == YUNOS_KOBJ_TYPE_ERR);

    mblk_pool_test.obj_type = YUNOS_MM_BLK_OBJ_TYPE;
    ret = yunos_mblk_free(&mblk_pool_test, ptr);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static const test_func_t mm_blk_func_runner[] = {
    mm_blk_break_case1,
    NULL
};

void mm_blk_break_test(void)
{
    kstat_t ret;

    task_mm_blk_entry_register(MODULE_NAME, (test_func_t *)mm_blk_func_runner,
                               sizeof(mm_blk_func_runner) / sizeof(test_func_t));

    ret = yunos_task_dyn_create(&task_mm_blk, MODULE_NAME, 0, TASK_MM_BLK_PRI,
                                0, TASK_TEST_STACK_SIZE, task_mm_blk_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}


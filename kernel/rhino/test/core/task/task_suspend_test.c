#include <k_api.h>
#include <test_fw.h>

#define TASK_TEST_STACK_SIZE 512

ktask_t *task_1_test;
ktask_t *task_2_test;

void task_suspend_entry(void *arg)
{
    while (1) {
        yunos_task_suspend(yunos_cur_task_get());

        test_case_success++;
        PRINT_RESULT("task_suspend", PASS);

        next_test_case_notify();
        yunos_task_dyn_del(yunos_cur_task_get());
    }
}

void task_resume_entry(void *arg)
{
    while (1) {
        yunos_task_resume(task_1_test);
        yunos_task_dyn_del(yunos_cur_task_get());
    }
}

void task_suspend_test(void)
{
    yunos_task_dyn_create(&task_1_test, "task_suspend_test_1", 0, 10,
                          0, TASK_TEST_STACK_SIZE,
                          task_suspend_entry, 1);

    /* for smp sync */
    yunos_task_sleep(YUNOS_CONFIG_TICKS_PER_SECOND / 10);

    yunos_task_dyn_create(&task_2_test, "task_suspend_test_2", 0, 11,
                          0, TASK_TEST_STACK_SIZE,
                          task_resume_entry, 1);
}


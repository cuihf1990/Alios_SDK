#ifndef COMB_TEST_H
#define COMB_TEST_H

#define TASK_COMB_PRI         16
#define TASK_TEST_STACK_SIZE 256

void comb_test(void);
void sem_queue_coopr_test(void);
void sem_event_coopr_test(void);
void sem_buf_queue_coopr_test(void);
void sem_mutex_coopr_test(void);
void comb_all_coopr_test(void);
void mutex_queue_coopr_test(void);
void mutex_buf_queue_coopr_test(void);

#endif /* SEM_TEST_H */


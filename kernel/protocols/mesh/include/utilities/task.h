#ifndef UMESH_TASK_H
#define UMESH_TASK_H

typedef void (* umesh_task_t)(void *args);

ur_error_t umesh_task_schedule_call(umesh_task_t task, void *arg);

#endif /* UMESH_TASK_H */

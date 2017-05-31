#include <assert.h>
#include <k_api.h>
#include <fifo.h>


#if (YUNOS_CONFIG_TRACE > 0)

#define TRACE_BUFFER_SIZE 1024
#define ROUND_POINT(sz) (((sz) + (4 - 1)) & ~(4 - 1))

#define TRACE_PACKET_LENGTH 80

struct k_fifo trace_fifo;
static uint32_t buffer[TRACE_BUFFER_SIZE];

/* task trace function */
void _trace_init(void)
{
    uint32_t buf[10];

    fifo_init(&trace_fifo, buffer, TRACE_BUFFER_SIZE * 4);

    buf[0] = 0x101;
    buf[1] = 0x0;
    buf[2] = 0x0;

    fifo_in_full_reject_lock(&trace_fifo, buf, 12);
}

void _trace_task_switch(ktask_t *from, ktask_t *to)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x102;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, from->task_name);
    addr_second = (char *)&buf[2] + strlen(from->task_name) + 1;
    str_len += strlen(from->task_name) + 1;

    strcpy(addr_second, to->task_name);
    addr_second += strlen(to->task_name) + 1;
    str_len += strlen(to->task_name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_intrpt_task_switch(ktask_t *from, ktask_t *to)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x104;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, from->task_name);
    addr_second = (char *)&buf[2] + strlen(from->task_name) + 1;
    str_len += strlen(from->task_name) + 1;

    strcpy(addr_second, to->task_name);
    addr_second += strlen(to->task_name) + 1;
    str_len += strlen(to->task_name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_task_create(ktask_t *task)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x103;
    buf[1] = 0x1;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;

    str_len += strlen(task->task_name) + 1;
    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_task_sleep(ktask_t *task, tick_t ticks)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x10a;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;

    str_len += strlen(task->task_name) + 1;
    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = (uint32_t)ticks;
    addr_second += 4;
   *(addr_third + 1) = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_task_pri_change(ktask_t *task, ktask_t *task_pri_chg, uint8_t pri)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x105;
    buf[1] = 0x3;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, task_pri_chg->task_name);
    addr_second += strlen(task_pri_chg->task_name) + 1;
    str_len += strlen(task_pri_chg->task_name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = pri;
    addr_second += 4;

    addr_third = (uint32_t *)addr_second;
    *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_task_suspend(ktask_t *task, ktask_t *task_suspended)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x106;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, task_suspended->task_name);
    addr_second += strlen(task_suspended->task_name) + 1;
    str_len += strlen(task_suspended->task_name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_task_resume(ktask_t *task, ktask_t *task_resumed)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x107;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, task_resumed->task_name);
    addr_second += strlen(task_resumed->task_name) + 1;
    str_len += strlen(task_resumed->task_name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_task_del(ktask_t *task, ktask_t *task_del)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x108;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, task_del->task_name);
    addr_second += strlen(task_del->task_name) + 1;
    str_len += strlen(task_del->task_name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_task_abort(ktask_t *task, ktask_t *task_abort)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x109;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, task_abort->task_name);
    addr_second += strlen(task_abort->task_name) + 1;
    str_len += strlen(task_abort->task_name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

/* semaphore trace function */
void _trace_sem_create(ktask_t *task, ksem_t *sem)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;
    const name_t *task_name;

    if (task == 0) {
        task_name = "system init";
    }
    else {
        task_name = task->task_name;
    }

    buf[0] = 0x201;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task_name);
    addr_second = (char *)&buf[2] + strlen(task_name) + 1;
    str_len += strlen(task_name) + 1;

    strcpy(addr_second, sem->blk_obj.name);
    addr_second += strlen(sem->blk_obj.name) + 1;
    str_len += strlen(sem->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_sem_overflow(ktask_t *task, ksem_t *sem)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x202;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, sem->blk_obj.name);
    addr_second += strlen(sem->blk_obj.name) + 1;
    str_len += strlen(sem->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_sem_del(ktask_t *task, ksem_t *sem)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x207;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, sem->blk_obj.name);
    addr_second += strlen(sem->blk_obj.name) + 1;
    str_len += strlen(sem->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_sem_get_success(ktask_t *task, ksem_t *sem)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x204;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, sem->blk_obj.name);
    addr_second += strlen(sem->blk_obj.name) + 1;
    str_len += strlen(sem->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_sem_get_blk(ktask_t *task, ksem_t *sem, tick_t wait_option)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x205;
    buf[1] = 0x3;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, sem->blk_obj.name);
    addr_second += strlen(sem->blk_obj.name) + 1;
    str_len += strlen(sem->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = (uint32_t)wait_option;
    addr_second += 4;

    addr_third = (uint32_t *)addr_second;
   *addr_third = (uint32_t)0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_sem_task_wake(ktask_t *task, ktask_t *task_waked_up, ksem_t *sem, uint8_t opt_wake_all)
{

    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x206;
    buf[1] = 0x3;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, task_waked_up->task_name);
    addr_second += strlen(task_waked_up->task_name) + 1;
    str_len += strlen(task_waked_up->task_name) + 1;

    strcpy(addr_second, sem->blk_obj.name);
    addr_second += strlen(sem->blk_obj.name) + 1;
    str_len += strlen(sem->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = opt_wake_all;
    addr_second += 4;

    addr_third = (uint32_t *)addr_second;
    *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_sem_cnt_increase(ktask_t *task, ksem_t *sem)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x203;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, sem->blk_obj.name);
    addr_second += strlen(sem->blk_obj.name) + 1;
    str_len += strlen(sem->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

/* mutex trace function */
void _trace_mutex_create(ktask_t *task, kmutex_t *mutex, const name_t *name)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    const name_t *task_name;

    if (task == 0) {
        task_name = "system init";
    }
    else {
        task_name = task->task_name;
    }

    buf[0] = 0x301;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task_name);
    addr_second = (char *)&buf[2] + strlen(task_name) + 1;
    str_len += strlen(task_name) + 1;

    strcpy(addr_second, mutex->blk_obj.name);
    addr_second += strlen(mutex->blk_obj.name) + 1;
    str_len += strlen(mutex->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_mutex_release(ktask_t *task, ktask_t *task_release, uint8_t new_pri)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x302;
    buf[1] = 0x3;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, task_release->task_name);
    addr_second += strlen(task_release->task_name) + 1;
    str_len += strlen(task_release->task_name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = new_pri;
    addr_second += 4;

    addr_third = (uint32_t *)addr_second;
    *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_mutex_get(ktask_t *task, kmutex_t *mutex, tick_t wait_option)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x303;
    buf[1] = 0x3;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, mutex->blk_obj.name);
    addr_second += strlen(mutex->blk_obj.name) + 1;
    str_len += strlen(mutex->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = wait_option;
    addr_second += 4;

    addr_third = (uint32_t *)addr_second;
    *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_task_pri_inv(ktask_t *task, ktask_t *mtxtsk)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x304;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, mtxtsk->task_name);
    addr_second += strlen(mtxtsk->task_name) + 1;
    str_len += strlen(mtxtsk->task_name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_mutex_get_blk(ktask_t *task, kmutex_t *mutex, tick_t wait_option)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x305;
    buf[1] = 0x3;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, mutex->blk_obj.name);
    addr_second += strlen(mutex->blk_obj.name) + 1;
    str_len += strlen(mutex->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = wait_option;
    addr_second += 4;

    addr_third = (uint32_t *)addr_second;
    *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_mutex_release_success(ktask_t *task, kmutex_t *mutex)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x306;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, mutex->blk_obj.name);
    addr_second += strlen(mutex->blk_obj.name) + 1;
    str_len += strlen(mutex->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}


void _trace_mutex_task_wake(ktask_t *task, ktask_t *task_waked_up, kmutex_t *mutex)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x307;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, task_waked_up->task_name);
    addr_second += strlen(task_waked_up->task_name) + 1;
    str_len += strlen(task_waked_up->task_name) + 1;

    strcpy(addr_second, mutex->blk_obj.name);
    addr_second += strlen(mutex->blk_obj.name) + 1;
    str_len += strlen(mutex->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_mutex_del(ktask_t *task, kmutex_t *mutex)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x308;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, mutex->blk_obj.name);
    addr_second += strlen(mutex->blk_obj.name) + 1;
    str_len += strlen(mutex->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

/* event trace function */
void _trace_event_create(ktask_t *task, kevent_t *event, const name_t *name, uint32_t flags_init)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;
    const name_t *task_name;

    if (task == 0) {
        task_name = "system init";
    }
    else {
        task_name = task->task_name;
    }

    buf[0] = 0x401;
    buf[1] = 0x3;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task_name);
    addr_second = (char *)&buf[2] + strlen(task_name) + 1;
    str_len += strlen(task_name) + 1;

    strcpy(addr_second, event->blk_obj.name);
    addr_second += strlen(event->blk_obj.name) + 1;
    str_len += strlen(event->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = flags_init;
    addr_second += 4;

    addr_third = (uint32_t *)addr_second;
   *addr_third =  0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);

}

void _trace_event_get(ktask_t *task, kevent_t *event)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x402;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, event->blk_obj.name);
    addr_second += strlen(event->blk_obj.name) + 1;
    str_len += strlen(event->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_event_get_blk(ktask_t *task, kevent_t *event, tick_t wait_option)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x403;
    buf[1] = 0x3;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, event->blk_obj.name);
    addr_second += strlen(event->blk_obj.name) + 1;
    str_len += strlen(event->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = (uint32_t)wait_option;
    addr_second += 4;

    addr_third = (uint32_t *)addr_second;
   *addr_third = (uint32_t)0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_event_task_wake(ktask_t *task, ktask_t *task_waked_up, kevent_t *event)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x404;
    buf[1] = 0x3;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, task_waked_up->task_name);
    addr_second += strlen(task_waked_up->task_name) + 1;
    str_len += strlen(task_waked_up->task_name) + 1;

    strcpy(addr_second, event->blk_obj.name);
    addr_second += strlen(event->blk_obj.name) + 1;
    str_len += strlen(event->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}


void _trace_event_del(ktask_t *task, kevent_t *event)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x405;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, event->blk_obj.name);
    addr_second += strlen(event->blk_obj.name) + 1;
    str_len += strlen(event->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

/* buf_queue trace function */
void _trace_buf_queue_create(ktask_t *task, kbuf_queue_t *buf_queue)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;
    const name_t *task_name;

    if (task == 0) {
        task_name = "system init";
    }
    else {
        task_name = task->task_name;
    }

    buf[0] = 0x501;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task_name);
    addr_second = (char *)&buf[2] + strlen(task_name) + 1;
    str_len += strlen(task_name) + 1;

    strcpy(addr_second, buf_queue->blk_obj.name);
    addr_second += strlen(buf_queue->blk_obj.name) + 1;
    str_len += strlen(buf_queue->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_buf_max(ktask_t *task, kbuf_queue_t *buf_queue, void *p_void, size_t msg_size)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x502;
    buf[1] = 0x3;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, buf_queue->blk_obj.name);
    addr_second += strlen(buf_queue->blk_obj.name) + 1;
    str_len += strlen(buf_queue->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = msg_size;
    addr_second += 4;

    addr_third = (uint32_t *)addr_second;
    *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_buf_post(ktask_t *task, kbuf_queue_t *buf_queue, void *p_void, size_t msg_size)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x503;
    buf[1] = 0x3;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, buf_queue->blk_obj.name);
    addr_second += strlen(buf_queue->blk_obj.name) + 1;
    str_len += strlen(buf_queue->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = msg_size;
    addr_second += 4;

    addr_third = (uint32_t *)addr_second;
    *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}


void _trace_buf_queue_task_wake(ktask_t *task, ktask_t *task_waked_up, kbuf_queue_t *buf_queue)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x504;
    buf[1] = 0x3;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, task_waked_up->task_name);
    addr_second += strlen(task_waked_up->task_name) + 1;
    str_len += strlen(task_waked_up->task_name) + 1;

    strcpy(addr_second, buf_queue->blk_obj.name);
    addr_second += strlen(buf_queue->blk_obj.name) + 1;
    str_len += strlen(buf_queue->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}


void _trace_buf_queue_get_blk(ktask_t *task, kbuf_queue_t *buf_queue, tick_t wait_option)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x505;
    buf[1] = 0x3;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, buf_queue->blk_obj.name);
    addr_second += strlen(buf_queue->blk_obj.name) + 1;
    str_len += strlen(buf_queue->blk_obj.name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = (uint32_t)wait_option;
    addr_second += 4;

    addr_third = (uint32_t *)addr_second;
   *addr_third = (uint32_t)0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject(&trace_fifo, buf, addr_second - addr_first);
}

/* timer trace function */
void _trace_timer_create(ktask_t *task, ktimer_t *timer)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;
    const name_t *task_name;

    if (task == 0) {
        task_name = "system init";
    }
    else {
        task_name = task->task_name;
    }

    buf[0] = 0x601;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task_name);
    addr_second = (char *)&buf[2] + strlen(task_name) + 1;
    str_len += strlen(task_name) + 1;

    strcpy(addr_second, timer->name);
    addr_second += strlen(timer->name) + 1;
    str_len += strlen(timer->name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_timer_del(ktask_t *task, ktimer_t *timer)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0x602;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, timer->name);
    addr_second += strlen(timer->name) + 1;
    str_len += strlen(timer->name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}

/* mblk trace function */
void _trace_mblk_pool_create(ktask_t *task, mblk_pool_t *pool)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;
    const name_t *task_name;

    if (task == 0) {
        task_name = "system init";
    }
    else {
        task_name = task->task_name;
    }

    buf[0] = 0x701;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task_name);
    addr_second = (char *)&buf[2] + strlen(task_name) + 1;
    str_len += strlen(task_name) + 1;

    strcpy(addr_second, pool->pool_name);
    addr_second += strlen(pool->pool_name) + 1;
    str_len += strlen(pool->pool_name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}

/* mm trace function */
#if 0
void _trace_mm_pool_create(ktask_t *task, mm_pool_t *pool)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;
    const name_t *task_name;

    if (task == 0) {
        task_name = "system init";
    }
    else {
        task_name = task->task_name;
    }

    buf[0] = 0x801;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task_name);
    addr_second = (char *)&buf[2] + strlen(task_name) + 1;
    str_len += strlen(task_name) + 1;

    strcpy(addr_second, pool->pool_name);
    addr_second += strlen(pool->pool_name) + 1;
    str_len += strlen(pool->pool_name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}
#endif
/* mm region function */
void _trace_mm_region_create(ktask_t *task, k_mm_region_t *regions)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;
    const name_t *task_name;

    if (task == 0) {
        task_name = "system init";
    }
    else {
        task_name = task->task_name;
    }

    buf[0] = 0x901;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task_name);
    addr_second = (char *)&buf[2] + strlen(task_name) + 1;
    str_len += strlen(task_name) + 1;

    strcpy(addr_second, "k_mm_region");
    addr_second += strlen("k_mm_region") + 1;
    str_len += strlen("k_mm_region") + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}

/* work queue trace */
void _trace_work_init(ktask_t *task, kwork_t *work)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;
    const name_t *task_name;

    if (task == 0) {
        task_name = "system init";
    }
    else {
        task_name = task->task_name;
    }

    buf[0] = 0xa01;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task_name);
    addr_second = (char *)&buf[2] + strlen(task_name) + 1;
    str_len += strlen(task_name) + 1;

    strcpy(addr_second, "work");
    addr_second += strlen("work") + 1;
    str_len += strlen("work") + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_workqueue_create(ktask_t *task, kworkqueue_t *workqueue)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;
    const name_t *task_name;

    if (task == 0) {
        task_name = "system init";
    }
    else {
        task_name = task->task_name;
    }

    buf[0] = 0xa02;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task_name);
    addr_second = (char *)&buf[2] + strlen(task_name) + 1;
    str_len += strlen(task_name) + 1;

    strcpy(addr_second, workqueue->name);
    addr_second += strlen(workqueue->name) + 1;
    str_len += strlen(workqueue->name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}

void _trace_workqueue_del(ktask_t *task, kworkqueue_t *workqueue)
{
    uint32_t  buf[20];
    char     *addr_first = (char  *)buf;
    char     *addr_second;
    uint32_t *addr_third;
    uint32_t  str_len = 0;

    buf[0] = 0xa03;
    buf[1] = 0x2;

    addr_second = (char *)&buf[2];
    strcpy(addr_second, task->task_name);
    addr_second = (char *)&buf[2] + strlen(task->task_name) + 1;
    str_len += strlen(task->task_name) + 1;

    strcpy(addr_second, workqueue->name);
    addr_second += strlen(workqueue->name) + 1;
    str_len += strlen(workqueue->name) + 1;

    str_len = ROUND_POINT(str_len);

    addr_second = addr_first + 8 + str_len;

    addr_third = (uint32_t *)addr_second;
   *addr_third = 0;
    addr_second += 4;

    assert((addr_second - addr_first) <= TRACE_PACKET_LENGTH);

    fifo_in_full_reject_lock(&trace_fifo, buf, addr_second - addr_first);
}

#endif


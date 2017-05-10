#ifndef __YOS_SYS_H_
#define __YOS_SYS_H_

#include <errno.h>
#include <semaphore.h>
#include <pthread.h>
#include "yos/common.h"

//reverse byte order
static inline uint32_t __reverse_32bit(uint32_t data)
{
	data = (data >> 16) | (data << 16);
	return ((data & 0xff00ff00UL) >> 8) | ((data & 0x00ff00ffUL) << 8);
}

static inline int __is_big_endian(void) 
{
    uint32_t data = 0xFF000000;
    return (0xFF == *(uint8_t *) & data);
}

//host byte order to big endian
static inline uint32_t yos_htobe32(uint32_t data)
{
	if (__is_big_endian()) 
		return data;
	return __reverse_32bit(data);
}

#define yos_WAIT_INFINITE          (~0)

uint32_t yos_get_time_ms(void);
pthread_mutex_t *yos_mutex_init();
void yos_mutex_destroy(pthread_mutex_t *mutex);
void *yos_semaphore_init(void);
int yos_semaphore_wait(sem_t *sem,  uint32_t timeout_ms);
void yos_semaphore_destroy(sem_t *sem);
void yos_semaphore_post(sem_t *sem);

#endif


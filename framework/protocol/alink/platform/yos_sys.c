#include "yos_sys.h"

uint32_t yos_get_time_ms(void)
{
	struct timeval tv = { 0 };
	uint32_t time_ms;

	gettimeofday(&tv, NULL);

	time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	return time_ms;
}

pthread_mutex_t *yos_mutex_init()
{
    pthread_mutex_t *ret = NULL;
    ret = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    if(!ret)
        return NULL;
    if(0 != pthread_mutex_init(ret,NULL)){
        free(ret);
        return NULL;
    }
    return ret;
}

void yos_mutex_destroy(pthread_mutex_t *mutex)
{
    if(mutex){
        pthread_mutex_destroy(mutex); 
        free(mutex);
    }
}

uint32_t yos_get_unaligned_be32(uint8_t * ptr)
{
	uint32_t res;

	memcpy(&res, ptr, sizeof(uint32_t));

	return yos_be32toh(res);
}

void *yos_semaphore_init(void)
{
	sem_t *sem = (sem_t *)malloc(sizeof(sem_t));
	if (NULL == sem)
	{
		return NULL;
	}

	if (0 != sem_init(sem, 0, 0))
	{
        free(sem);
        return NULL;
	}

	return sem;
}

int yos_semaphore_wait(sem_t *sem, uint32_t timeout_ms)
{
	if (yos_WAIT_INFINITE == timeout_ms)
	{
		sem_wait(sem);
		return 0;
	}
	else
	{
		struct timespec ts;
		int s;
		/* Restart if interrupted by handler */
		do
		{
	        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
	        {
	            return -1;
	        }

	        s = 0;
	        ts.tv_nsec += (timeout_ms % 1000) * 1000000;
	        if (ts.tv_nsec >= 10000000)
	        {
	            ts.tv_nsec -= 10000000;
	            s = 1;
	        }

	        ts.tv_sec += timeout_ms / 1000 + s;

		} while (((s = sem_timedwait(sem, &ts)) != 0) && errno == EINTR);

		return (s == 0) ? 0 : -1;
	}
}

void yos_semaphore_post(sem_t *sem)
{
	sem_post(sem);
}


void yos_semaphore_destroy(sem_t *sem)
{
	sem_destroy(sem);
	free(sem);
}
 


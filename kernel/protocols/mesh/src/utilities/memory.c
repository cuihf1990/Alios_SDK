#include <stdint.h>
#include <stddef.h>

#include <yos/kernel.h>
#include "umesh_utils.h"

typedef struct mem_info_s {
    ur_mem_stats_t stats;
    yos_mutex_t mutex;
} mem_info_t;
static mem_info_t g_mem_info;

void *ur_mem_alloc(uint16_t size)
{
    void *mem = NULL;

    if (g_mem_info.stats.num + size > MEM_BUF_SIZE) {
        return mem;
    }

    mem = (void *)yos_malloc((size_t)size);
    if (mem) {
        yos_mutex_lock(&g_mem_info.mutex, YOS_WAIT_FOREVER);
        g_mem_info.stats.num += size;
        yos_mutex_unlock(&g_mem_info.mutex);
    }
    return mem;
}

void ur_mem_free(void *mem, uint16_t size)
{
    if (mem) {
        yos_free(mem);
        yos_mutex_lock(&g_mem_info.mutex, YOS_WAIT_FOREVER);
        g_mem_info.stats.num -= size;
        yos_mutex_unlock(&g_mem_info.mutex);
    }
}

void umesh_mem_init(void)
{
    bzero(&g_mem_info.stats, sizeof(g_mem_info.stats));
    yos_mutex_new(&g_mem_info.mutex);
}

void umesh_mem_deinit(void)
{
    yos_mutex_free(&g_mem_info.mutex);
}

const ur_mem_stats_t *ur_mem_get_stats(void)
{
    return &g_mem_info.stats;
}

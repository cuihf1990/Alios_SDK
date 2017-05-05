#include <k_api.h>
#include <yos/kernel.h>

static uint32_t used_bitmap;

int yos_task_key_create(yos_task_key_t *key)
{
    int i;
    for (i = YUNOS_CONFIG_TASK_INFO_NUM - 1; i >= 0; i --) {
        if (!((1 << i) & used_bitmap)) {
            used_bitmap |= 1 << i;
            *key = i;
            return 0;
        }
    }

    return -1;
}

void yos_task_key_delete(yos_task_key_t key)
{
    if (key >= YUNOS_CONFIG_TASK_INFO_NUM)
        return;

    used_bitmap &= ~(1 << key);
}

int yos_task_setspecific(yos_task_key_t key, void *vp)
{
    yunos_task_info_set(g_active_task, key, vp);
}

void *yos_task_getspecific(yos_task_key_t key)
{
    void *vp = NULL;
    yunos_task_info_get(g_active_task, key, &vp);
    return vp;
}

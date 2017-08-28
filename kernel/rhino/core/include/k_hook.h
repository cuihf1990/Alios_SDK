#ifndef K_HOOK_H
#define K_HOOK_H

#if (YUNOS_CONFIG_USER_HOOK > 0)
/**
 * This function will provide init hook
 */
void yunos_init_hook(void);

/**
 * This function will provide system start hook
 */
void yunos_start_hook(void);

/**
 * This function will provide task create hook
 * @param[in]  task  pointer to the task
 */
void yunos_task_create_hook(ktask_t *task);

/**
 * This function will provide task delete hook
 * @param[in]  task  pointer to the task
 */
void yunos_task_del_hook(ktask_t *task);

/**
 * This function will provide task abort hook
 * @param[in]  task  pointer to the task
 */
void yunos_task_abort_hook(ktask_t *task);

/**
 * This function will provide task switch hook
 */
void yunos_task_switch_hook(ktask_t *orgin, ktask_t *dest);

/**
 * This function will provide system tick hook
 */
void yunos_tick_hook(void);

/**
 * This function will provide idle hook
 */
void yunos_idle_hook(void);

/**
 * This function will provide  yunos_mm_alloc hook
 */
void yunos_mm_alloc_hook(void *mem, size_t size);
#endif

#endif /* K_HOOK_H */


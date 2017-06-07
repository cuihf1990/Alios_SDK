/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
#endif

#endif /* K_HOOK_H */


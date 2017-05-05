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

#ifndef PORT_H
#define PORT_H

size_t cpu_intrpt_save(void);
void   cpu_intrpt_restore(size_t cpsr);
void   cpu_intrpt_switch(void);
void   cpu_task_switch(void);
void   cpu_first_task_start(void);
void  *cpu_task_stack_init(cpu_stack_t *base, size_t size, void *arg, task_entry_t entry);

#if (YUNOS_CONFIG_BITMAP_HW != 0)
int32_t cpu_bitmap_clz(uint32_t val);
#endif

#if (YUNOS_CONFIG_STACK_OVF_CHECK_HW != 0)
void cpu_intrpt_stack_protect(void);
void cpu_task_stack_protect(cpu_stack_t *base, size_t size);
#endif

#define CPSR_ALLOC() size_t cpsr

#define YUNOS_CPU_INTRPT_DISABLE() { cpsr = cpu_intrpt_save(); }
#define YUNOS_CPU_INTRPT_ENABLE()  { cpu_intrpt_restore(cpsr); }

#endif /* PORT_H */


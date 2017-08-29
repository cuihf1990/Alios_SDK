#ifndef PORT_H
#define PORT_H

size_t cpu_intrpt_save(void);
void   cpu_intrpt_restore(size_t cpsr);
void   cpu_intrpt_switch(void);
void   cpu_task_switch(void);
void   cpu_first_task_start(void);
void  *cpu_task_stack_init(cpu_stack_t *base, size_t size, void *arg, task_entry_t entry);

#define CPSR_ALLOC() size_t cpsr

#define YUNOS_CPU_INTRPT_DISABLE() { cpsr = cpu_intrpt_save(); }
#define YUNOS_CPU_INTRPT_ENABLE()  { cpu_intrpt_restore(cpsr); }

#endif /* PORT_H */


#include <k_api.h>

void *cpu_task_stack_init(cpu_stack_t *base, size_t size, void *arg, task_entry_t entry)
{
	cpu_stack_t *stk;
	uint32_t temp = (uint32_t)(base + size);

	/*stack need 8 bytes align*/
	temp &= 0xfffffff8;
	stk = (cpu_stack_t *)temp;

	*(--stk)  = (uint32_t)entry;         /* Entry Point */
	*(--stk)  = (uint32_t)0;             /* LR          */
	*(--stk)  = (uint32_t)0;             /* R12		   */
	*(--stk)  = (uint32_t)0;             /* R11         */
	*(--stk)  = (uint32_t)0;             /* R10         */
	*(--stk)  = (uint32_t)0;             /* R9          */
	*(--stk)  = (uint32_t)0;             /* R8          */
	*(--stk)  = (uint32_t)0;             /* R7 :        */
	*(--stk)  = (uint32_t)0;             /* R6          */
	*(--stk)  = (uint32_t)0;             /* R5          */
	*(--stk)  = (uint32_t)0;             /* R4          */
	*(--stk)  = (uint32_t)0;             /* R3          */
	*(--stk)  = (uint32_t)0;             /* R2          */
	*(--stk)  = (uint32_t)0;             /* R1          */
	*(--stk)  = (uint32_t)arg;           /* R0 argument */
	*(--stk)  = (uint32_t)0x13;          /* CPSR SVC mode */

	 return stk;	 
}


@******************************************************************************
@                            EXTERN PARAMETERS
@******************************************************************************

.extern g_active_task
.extern g_preferred_ready_task

@******************************************************************************
@                            EXPORT FUNCTIONS
@******************************************************************************

.global cpu_intrpt_save
.global cpu_intrpt_restore
.global cpu_task_switch
.global cpu_intrpt_switch
.global cpu_first_task_start

.global cpu_dis_interpt
.global cpu_en_interpt

.text

cpu_intrpt_save:
    MRS     R0, CPSR                @Set IRQ and FIQ bits in CPSR to disable all interrupts
    ORR     R1, R0, #0xC0
    MSR     CPSR_c, R1
    MRS     R1, CPSR                @Confirm that CPSR contains the proper interrupt disable flags
    AND     R1, R1, #0xC0
    CMP     R1, #0xC0
    BNE     cpu_intrpt_save         @Not properly disabled (try again)
    BX      LR                      @Disabled, return the original CPSR contents in R0

cpu_intrpt_restore:
    MSR     CPSR_c, R0
    BX      LR


cpu_dis_interpt:
    MRS R0,CPSR
    ORR R1,R0,#0xC0
    MSR CPSR_c,R1
    BX  LR

cpu_en_interpt:
    MRS R0,CPSR
    AND R1,R0,#0x3F
    MSR CPSR_c,R1
    BX  LR


cpu_first_task_start:
    LDR R0, =g_active_task
    LDR R0, [R0]         
    LDR SP, [R0]
    
    LDMFD   SP!, {R0}  
    MSR     SPSR_cxsf, R0
    LDMFD   SP!, {R0-R12, LR, PC}^

cpu_task_switch:
    STMFD SP!,{LR}             
    STMFD SP!,{R0-R12,LR}       
    MRS   R0,CPSR
    STMFD SP!,{R0}                 @push current cpsr         

    /* g_active_task->task_stack = SP */
    LDR R0, =g_active_task
    LDR R0, [R0]
    STR SP, [R0]

cpu_intrpt_switch:
    LDR     R0, =g_preferred_ready_task
    LDR     R1, =g_active_task
    LDR     R0, [R0]
    STR     R0, [R1]

    LDR     R0, =g_active_task
    LDR     R0, [R0]
    LDR     SP, [R0]

    @----------------------------------------------------------------------------------	
	@ Restore New Task context
	@----------------------------------------------------------------------------------
	
    LDMFD   SP!, {R0}              
    MSR     SPSR_cxsf, R0
    LDMFD   SP!, {R0-R12, LR, PC}^  

    .end


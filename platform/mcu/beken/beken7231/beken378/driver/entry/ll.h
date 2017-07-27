/**
 ****************************************************************************************
 *
 * @file ll.h
 *
 * @brief Declaration of low level functions.
 *
 * Copyright (C) RivieraWaves 2011-2016
 *
 ****************************************************************************************
 */

#ifndef LL_H_
#define LL_H_

#include "co_int.h"
#include "compiler.h"

/*
 * Enable Interrupts
 */	
#define portENABLE_IRQ()					\
	({							              \
		unsigned long temp;				\
		__asm volatile(					\
		"mrs	%0, cpsr		@ local_irq_enable\n"	\
	       "bic	%0, %0, #0x80\n"					\
	       "msr	cpsr_c, %0"					       \
		: "=r" (temp)						       \
		:							              \
		: "memory");						       \
	})
#define portENABLE_FIQ()					\
	({							              \
		unsigned long temp;				\
		__asm volatile(					\
		"mrs	%0, cpsr		@ local_irq_enable\n"	\
	       "bic	%0, %0, #0x40\n"					\
	       "msr	cpsr_c, %0"					       \
		: "=r" (temp)						       \
		:							              \
		: "memory");						       \
	})

extern uint32_t platform_is_in_interrupt_context( void );
extern uint32_t platform_is_in_fiq_context( void );
	
#define portENABLE_INTERRUPTS()			do{		\
			if(!platform_is_in_interrupt_context())\
										    	portENABLE_IRQ();\
			if(!platform_is_in_fiq_context())\
										    	portENABLE_FIQ();\
										    }while(0)
										    
/*
 * Disable Interrupts
 */
static inline  int portDISABLE_FIQ(void)
{						                     
	unsigned long temp;				       
	unsigned long mask;		
	
	__asm volatile(					
	"mrs	%1, cpsr		@ local_irq_disable\n"	
	"orr	%0, %1, #0x40\n"					
	"msr	cpsr_c, %0"					       
	: "=r" (temp),"=r" (mask)						       
	:							              
	: "memory");		

	return (!!(mask & 0x40));
}

static inline  int portDISABLE_IRQ(void)
{						                     
	unsigned long temp;				       
	unsigned long mask;		
	
	__asm volatile(					
	"mrs	%1, cpsr		@ local_irq_disable\n"	
	"orr	%0, %1, #0x80\n"					
	"msr	cpsr_c, %0"					       
	: "=r" (temp),"=r" (mask)						       
	:							              
	: "memory");		

	return (!!(mask & 0x80));
}
	
#define portDISABLE_INTERRUPTS()		do{		\
										    	portDISABLE_FIQ();\
										    	portDISABLE_IRQ();\
										    }while(0)


#define GLOBAL_INT_START               portENABLE_INTERRUPTS
#define GLOBAL_INT_STOP                portDISABLE_INTERRUPTS

#define GLOBAL_INT_DECLARATION()   uint32_t fiq_tmp, irq_tmp
#define GLOBAL_INT_DISABLE()       do{\
										fiq_tmp = portDISABLE_FIQ();\
										irq_tmp = portDISABLE_IRQ();\
									}while(0)


#define GLOBAL_INT_RESTORE()       do{                         \
                                        if(!fiq_tmp)           \
                                        {                      \
                                            portENABLE_FIQ();    \
                                        }                      \
                                        if(!irq_tmp)           \
                                        {                      \
                                            portENABLE_IRQ();    \
                                        }                      \
                                   }while(0)
#endif // LL_H_


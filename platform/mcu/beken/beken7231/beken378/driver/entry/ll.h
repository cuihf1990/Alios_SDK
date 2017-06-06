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
#include "portmacro.h"


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


#include "hal/soc/soc.h"
#include "mico_rtos.h"

typedef unsigned char  		  UINT8;          /* Unsigned  8 bit quantity        */
typedef signed   char  		  INT8;           /* Signed    8 bit quantity        */
typedef unsigned short 		  UINT16;         /* Unsigned 16 bit quantity        */
typedef signed   short 		  INT16;          /* Signed   16 bit quantity        */
typedef uint32_t   		      UINT32;         /* Unsigned 32 bit quantity        */
typedef int32_t   		      INT32;          /* Signed   32 bit quantity        */

#include "flash_pub.h"

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

#define SECTOR_SIZE 0x1000 /* 4 K/sector */

extern const hal_logic_partition_t hal_partitions[];

hal_logic_partition_t *hal_flash_get_info(hal_partition_t in_partition)
{
    hal_logic_partition_t *logic_partition;

    logic_partition = (hal_logic_partition_t *)&hal_partitions[ in_partition ];

    return logic_partition;
}

int32_t hal_flash_erase(hal_partition_t in_partition, uint32_t off_set, uint32_t size)
{
    uint32_t addr;
    uint32_t start_addr, end_addr;
    hal_logic_partition_t *partition_info;

    GLOBAL_INT_DECLARATION();

    partition_info = hal_flash_get_info( in_partition );

    start_addr = (partition_info->partition_start_addr + off_set) & (~0xFFF);
    end_addr = (partition_info->partition_start_addr + off_set + size - 1) & (~0xFFF);

    for(addr = start_addr; addr <= end_addr; addr += SECTOR_SIZE)
    {
        GLOBAL_INT_DISABLE();
        flash_ctrl(CMD_FLASH_ERASE_SECTOR, &addr);
        GLOBAL_INT_RESTORE();
    }
    
    return 0;
}
                        
int32_t hal_flash_write(hal_partition_t in_partition, uint32_t *off_set, const void *in_buf , uint32_t in_buf_len)
{
    uint32_t start_addr;
    hal_logic_partition_t *partition_info;

    GLOBAL_INT_DECLARATION();

    partition_info = hal_flash_get_info( in_partition );

    start_addr = partition_info->partition_start_addr + *off_set;

    GLOBAL_INT_DISABLE();
    flash_write(in_buf, in_buf_len, start_addr);
    GLOBAL_INT_RESTORE();

    *off_set += in_buf_len;

    return 0;
}

int32_t hal_flash_read(hal_partition_t in_partition, uint32_t *off_set, void *out_buf, uint32_t out_buf_len)
{
    uint32_t start_addr;
    hal_logic_partition_t *partition_info;

    GLOBAL_INT_DECLARATION();

    partition_info = hal_flash_get_info( in_partition );

    start_addr = partition_info->partition_start_addr + *off_set;

    GLOBAL_INT_DISABLE();
    flash_read(out_buf, out_buf_len, start_addr);
    GLOBAL_INT_RESTORE();

    *off_set += out_buf_len;

    return 0;
}

int32_t hal_flash_enable_secure(hal_partition_t partition, uint32_t off_set, uint32_t size)
{
    return 0;
}

int32_t hal_flash_dis_secure(hal_partition_t partition, uint32_t off_set, uint32_t size)
{
    return 0;
}

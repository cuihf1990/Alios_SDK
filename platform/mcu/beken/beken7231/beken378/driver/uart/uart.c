#include "include.h"
#include "arm_arch.h"

#include "uart_pub.h"
#include "uart.h"

#include "drv_model_pub.h"
#include "sys_ctrl_pub.h"
#include "mem_pub.h"
#include "icu_pub.h"
#include "gpio_pub.h"

#include <stdio.h>

#include "ll.h"
#include "mem_pub.h"
#include "intc_pub.h"


////just for uart init
/**
 * UART data width
 */
typedef enum
{
    DATA_WIDTH_5BIT,
    DATA_WIDTH_6BIT,
    DATA_WIDTH_7BIT,
    DATA_WIDTH_8BIT
} uart_data_width_t;

/**
 * UART stop bits
 */
typedef enum
{
    STOP_BITS_1,
    STOP_BITS_2,
} uart_stop_bits_t;

/**
 * UART flow control
 */
typedef enum
{
    FLOW_CONTROL_DISABLED,
    FLOW_CONTROL_CTS,
    FLOW_CONTROL_RTS,
    FLOW_CONTROL_CTS_RTS
} uart_flow_control_t;

/**
 * UART parity
 */
typedef enum
{
    NO_PARITY,
    ODD_PARITY,
    EVEN_PARITY,
} uart_parity_t;

typedef struct
{
    uint32_t					  baud_rate;
    uart_data_width_t    data_width;
    uart_parity_t 	      parity;
    uart_stop_bits_t	  stop_bits;
    uart_flow_control_t  flow_control;
    uint8_t					      flags;	 /**< if set, UART can wake up MCU from stop mode, reference: @ref UART_WAKEUP_DISABLE and @ref UART_WAKEUP_ENABLE*/
} uart_config_t;



#ifndef KEIL_SIMULATOR
#if CFG_UART_DEBUG_COMMAND_LINE
STATIC UART_S uart =
{
    0,
};

static DD_OPERATIONS uart_op =
{
    uart_open,
    uart_close,
    uart_read,
    uart_write,
    uart_ctrl
};
#endif

#if __CC_ARM
struct __FILE
{
    int handle;                 // Add whatever  need here
};

FILE __stdout;
FILE __stdin;

extern void uart_rx_cb(void);
extern void uart_tx_cb(void);

void uart_send_byte(UINT8 data)
{
    while(!UART_TX_WRITE_READY);

    UART_WRITE_BYTE(data);
}

/*----------------------------------------------------------------------------
  fputc
 *----------------------------------------------------------------------------*/
int fputc(int ch, FILE *f)
{
    uart_send_byte(ch);

    return ch;
}

/*----------------------------------------------------------------------------
  fgetc
 *----------------------------------------------------------------------------*/
int fgetc(FILE *f)
{
    return (-1);
}
#else // __GNUC__
void bk_send_byte(UINT8 data)
{
    while(!UART_TX_WRITE_READY);

    UART_WRITE_BYTE(data);
}

void bk_send_string(char *string)
{
    while(*string)
    {
        bk_send_byte(*string++);
    }
}

static char output_buf[128];

void bk_printf(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(output_buf, 128, fmt, ap);
    output_buf[127] = 0;
    bk_send_string(output_buf);
    va_end(ap);
}

#endif

#if CFG_BACKGROUND_PRINT
INT32 uart_printf(const char *fmt, ...)
{
    INT32 rc;
    char buf[TX_RB_LENGTH];

    va_list args;
    va_start(args, fmt);
    rc = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    buf[sizeof(buf) - 1] = '\0';
    if((rc < sizeof(buf) - 2)
            && (buf[rc - 1] == '\n')
            && (buf[rc - 2] != '\r'))
    {
        buf[rc - 1] = '\r';
        buf[rc] = '\n';
        buf[rc + 1] = 0;

        rc += 1;
    }

    return kfifo_put(uart.tx, (UINT8 *)&buf[0], rc);
}
#endif // CFG_BACKGROUND_PRINT

void fatal_print(const char *fmt, ...)
{
    os_printf(fmt);

    DEAD_WHILE();
}


void uart_hw_init(void)
{
    UINT32 reg;
    UINT32 baud_div;

    baud_div = UART_CLOCK / UART_BAUD_RATE;
    baud_div = baud_div - 1;

    reg = UART_TX_ENABLE
          | UART_RX_ENABLE
          & (~UART_IRDA)
          | ((DEF_DATA_LEN & UART_DATA_LEN_MASK) << UART_DATA_LEN_POSI)
          & (~UART_PAR_EN)
          & (~UART_PAR_ODD_MODE)
          & (~UART_STOP_LEN_2)
          | ((baud_div & UART_CLK_DIVID_MASK) << UART_CLK_DIVID_POSI);
    REG_WRITE(REG_UART_CONFIG, reg);

    reg = ((TX_FIFO_THRD & TX_FIFO_THRESHOLD_MASK) << TX_FIFO_THRESHOLD_POSI)
          | ((RX_FIFO_THRD & RX_FIFO_THRESHOLD_MASK) << RX_FIFO_THRESHOLD_POSI)
          | ((RX_STOP_DETECT_TIME32 & RX_STOP_DETECT_TIME_MASK) << RX_STOP_DETECT_TIME_POSI);
    REG_WRITE(REG_UART_FIFO_CONFIG, reg);

    REG_WRITE(REG_UART_FLOW_CONFIG, 0);
    REG_WRITE(REG_UART_WAKE_CONFIG, 0);

    reg = RX_FIFO_NEED_READ_EN | UART_RX_STOP_END_EN;
    REG_WRITE(REG_UART_INTR_ENABLE, reg);

    return;
}

void uart_hw_set_change(uart_config_t *uart_config)
{
    UINT32 reg;
    UINT32 baud_div;
    UINT32 	width;
    uart_parity_t 	     parity_en;
    uart_stop_bits_t	  stop_bits;
    uart_flow_control_t  flow_control;
    UINT8 parity_mode = 0;

    REG_WRITE(REG_UART_INTR_ENABLE, 0);//disable int

    baud_div = UART_CLOCK / uart_config->baud_rate;
    baud_div = baud_div - 1;
    width = uart_config->data_width;
    parity_en = uart_config->parity;
    stop_bits = uart_config->stop_bits;
    flow_control = uart_config->flow_control;

    if(parity_en)
    {
        if(parity_en == ODD_PARITY)
            parity_mode = 1;
        else
            parity_mode = 0;
        parity_en = 1;
    }

    reg = UART_TX_ENABLE
          | UART_RX_ENABLE
          & (~UART_IRDA)
          | ((width & UART_DATA_LEN_MASK) << UART_DATA_LEN_POSI)
          | (parity_en << 5)
          | (parity_mode << 6)
          | (stop_bits << 7)
          | ((baud_div & UART_CLK_DIVID_MASK) << UART_CLK_DIVID_POSI);

    width = ((width & UART_DATA_LEN_MASK) << UART_DATA_LEN_POSI);
    REG_WRITE(REG_UART_CONFIG, reg);

    reg = ((TX_FIFO_THRD & TX_FIFO_THRESHOLD_MASK) << TX_FIFO_THRESHOLD_POSI)
          | ((RX_FIFO_THRD & RX_FIFO_THRESHOLD_MASK) << RX_FIFO_THRESHOLD_POSI)
          | ((RX_STOP_DETECT_TIME32 & RX_STOP_DETECT_TIME_MASK) << RX_STOP_DETECT_TIME_POSI);
    REG_WRITE(REG_UART_FIFO_CONFIG, reg);

    REG_WRITE(REG_UART_FLOW_CONFIG, 0);
    REG_WRITE(REG_UART_WAKE_CONFIG, 0);

    reg = RX_FIFO_NEED_READ_EN | UART_RX_STOP_END_EN;
    REG_WRITE(REG_UART_INTR_ENABLE, reg);
}

void uart_isr(void)
{
#if CFG_UART_DEBUG_COMMAND_LINE
    UINT32 status;
    UINT32 intr_en;
    UINT32 intr_status;

    intr_en = REG_READ(REG_UART_INTR_ENABLE);
    intr_status = REG_READ(REG_UART_INTR_STATUS);
    REG_WRITE(REG_UART_INTR_STATUS, intr_status);
    status = intr_status & intr_en;

    if(status & (RX_FIFO_NEED_READ_STA | UART_RX_STOP_END_STA))
    {
        uart_rx_cb();
    }
    else if(status & TX_FIFO_NEED_WRITE_STA)
    {
    }
    else if(status & RX_FIFO_OVER_FLOW_STA)
    {
    }
    else if(status & UART_RX_PARITY_ERR_STA)
    {
        uart_fifo_flush();
    }
    else if(status & UART_RX_STOP_ERR_STA)
    {
    }
    else if(status & UART_TX_STOP_END_STA)
    {
        uart_tx_cb();
    }
    else if(status & UART_RXD_WAKEUP_STA)
    {
    }
    else
    {
    }

#endif
}

#if (!CFG_UART_DEBUG_COMMAND_LINE)
void uart_init(void)
{
    UINT32 param;

    intc_service_register(IRQ_UART, PRI_IRQ_UART, uart_isr);

    param = PWD_UART_CLK_BIT;
    sddev_control(ICU_DEV_NAME, CMD_CLK_PWR_UP, &param);

    param = GFUNC_MODE_UART;
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_ENABLE_SECOND, &param);

    uart_hw_init();
}

void uart_exit(void)
{
}
#else
UINT32 uart_sw_init(void)
{
    uart.rx = kfifo_alloc(RX_RB_LENGTH);
    uart.tx = kfifo_alloc(TX_RB_LENGTH);

    if((!uart.tx) || (!uart.rx))
    {
        goto sw_fail;
    }

    return UART_SUCCESS;

sw_fail:
    if(uart.tx)
    {
        kfifo_free(uart.tx);
    }

    if(uart.rx)
    {
        kfifo_free(uart.rx);
    }

    return UART_FAILURE;
}

UINT32 uart_sw_uninit(void)
{
    if(uart.tx)
    {
        kfifo_free(uart.tx);
    }

    if(uart.rx)
    {
        kfifo_free(uart.rx);
    }

    os_memset(&uart, 0, sizeof(uart));

    return UART_SUCCESS;
}

void uart_fifo_flush(void)
{
    UINT32 val;
    UINT32 reg;

    val = REG_READ(REG_UART_CONFIG);
    reg = val & (~(UART_TX_ENABLE | UART_RX_ENABLE));

    REG_WRITE(REG_UART_CONFIG, reg);
    REG_WRITE(REG_UART_CONFIG, val);
}

void uart_hw_uninit(void)
{
    UINT32 i;
    UINT32 reg;
    UINT32 rx_count;

    /*disable rtx intr*/
    reg = REG_READ(REG_UART_INTR_ENABLE);
    reg &= (~(RX_FIFO_NEED_READ_EN | UART_RX_STOP_END_EN));
    REG_WRITE(REG_UART_INTR_ENABLE, reg);

    /* flush fifo*/
    uart_fifo_flush();

    /* disable rtx*/
    reg = REG_READ(REG_UART_CONFIG);
    reg = reg & (~(UART_TX_ENABLE | UART_RX_ENABLE));
    REG_WRITE(REG_UART_CONFIG, reg);

    /* double discard fifo data*/
    reg = REG_READ(REG_UART_FIFO_STATUS);
    rx_count = (reg >> RX_FIFO_COUNT_POSI) & RX_FIFO_COUNT_MASK;
    for(i = 0; i < rx_count; i ++)
    {
        UART_READ_BYTE_DISCARD();
    }
}

void uart_reset(void)
{
    uart_exit();
    uart_init();
}

void uart_send_backgroud(void)
{
    /* send the buf at backgroud context*/
    uart_write_fifo_frame(uart.tx, DEBUG_PRT_MAX_CNT);
}

UINT32 uart_write_fifo_frame(KFIFO_PTR tx_ptr, UINT32 count)
{
    UINT32 len;
    UINT32 ret;
    UINT32 val;

    len = 0;

    while(1)
    {
        ret = kfifo_get(tx_ptr, (UINT8 *)&val, 1);
        if(0 == ret)
        {
            break;
        }


#if __CC_ARM
        uart_send_byte((UINT8)val);
#else
        bk_send_byte((UINT8)val);
#endif

        len += ret;
        if(len >= count)
        {
            break;
        }
    }

    return len;
}

UINT32 uart_read_fifo_frame(KFIFO_PTR rx_ptr)
{
    UINT32 val;
    UINT32 rx_count;

    rx_count = 0;
    while(REG_READ(REG_UART_FIFO_STATUS) & FIFO_RD_READY)
    {
        UART_READ_BYTE(val);
        rx_count += kfifo_put(rx_ptr, (UINT8 *)&val, 1);
    }

    return rx_count;
}

/*******************************************************************/
void uart_init(void)
{
    UINT32 ret;
    UINT32 param;
    UINT32 intr_status;

    ret = uart_sw_init();
    ASSERT(UART_SUCCESS == ret);

    ddev_register_dev(UART_DEV_NAME, &uart_op);

    intc_service_register(IRQ_UART, PRI_IRQ_UART, uart_isr);

    param = PWD_UART_CLK_BIT;
    sddev_control(ICU_DEV_NAME, CMD_CLK_PWR_UP, &param);

    param = GFUNC_MODE_UART;
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_ENABLE_SECOND, &param);
    uart_hw_init();

    /*irq enable, Be careful: it is best that irq enable at open routine*/
    intr_status = REG_READ(REG_UART_INTR_STATUS);
    REG_WRITE(REG_UART_INTR_STATUS, intr_status);

    param = IRQ_UART_BIT;
    sddev_control(ICU_DEV_NAME, CMD_ICU_INT_ENABLE, &param);

    UART_PRT("uart_init_over\r\n");
    UART_PRT("     uart.rx->in:%d, \r\n", uart.rx->in);
    UART_PRT("     uart.rx->out:%d, \r\n", uart.rx->out);
    UART_PRT("     uart.rx->size:%d, \r\n", uart.rx->size);
    UART_PRT("     uart.rx->buffer:0x%p, \r\n", uart.rx->buffer);

    UART_PRT("\r\n");

    UART_PRT("     uart.tx->in:%d, \r\n", uart.tx->in);
    UART_PRT("     uart.tx->out:%d, \r\n", uart.tx->out);
    UART_PRT("     uart.tx->size:%d, \r\n", uart.tx->size);
    UART_PRT("     uart.tx->buffer:0x%p, \r\n", uart.tx->buffer);
}

void uart_exit(void)
{
    UINT32 param;

    /*irq enable, Be careful: it is best that irq enable at close routine*/
    param = IRQ_UART_BIT;
    sddev_control(ICU_DEV_NAME, CMD_ICU_INT_DISABLE, &param);

    uart_hw_uninit();

    ddev_unregister_dev(UART_DEV_NAME);

    uart_sw_uninit();
}

UINT32 uart_open(UINT32 op_flag)
{
    return UART_SUCCESS;
}

UINT32 uart_close(void)
{
    return UART_SUCCESS;
}

UINT32 uart_read(char *user_buf, UINT32 count, UINT32 op_flag)
{
    return kfifo_get(uart.rx, (UINT8 *)user_buf, count);
}

UINT32 uart_write(char *user_buf, UINT32 count, UINT32 op_flag)
{
    return kfifo_put(uart.tx, (UINT8 *)user_buf, count);
}

UINT32 uart_ctrl(UINT32 cmd, void *parm)
{
    UINT32 ret;

    ret = UART_SUCCESS;
    switch(cmd)
    {
    case CMD_SEND_BACKGROUND:
        uart_send_backgroud();
        break;

    case CMD_UART_RESET:
        uart_reset();
        break;

    case CMD_RX_COUNT:
        ret = kfifo_data_size(uart.rx);
        break;

    case CMD_RX_PEEK:
    {
        UART_PEEK_RX_PTR peek;

        peek = (UART_PEEK_RX_PTR)parm;

        if(!((URX_PEEK_SIG != peek->sig)
                || (NULLPTR == peek->ptr)
                || (0 == peek->len)))
        {
            ret = kfifo_out_peek(uart.rx, peek->ptr, peek->len);
        }

        break;
    }
    case CMD_UART_INIT:
        uart_hw_set_change(parm);
        break;
    default:
        break;
    }

    return ret;
}
#endif // (!CFG_UART_DEBUG_COMMAND_LINE)

#endif // KEIL_SIMULATOR

INT32 os_null_printf(const char *fmt, ...)
{
    return 0;
}

INT32 uart_read_byte( UINT8 *byte )
{
    UINT32 val;

    if(REG_READ(REG_UART_FIFO_STATUS) & FIFO_RD_READY)
    {
        UART_READ_BYTE(val);
        *byte = (UINT8)val;
        return 0;
    }

    return -1;
}

VOID uart_write_byte( UINT8 byte )
{
    UART_WRITE_BYTE(byte);
}

UINT8 uart_get_tx_fifo_cnt(VOID)
{
    return REG_READ(REG_UART_FIFO_STATUS) >> TX_FIFO_COUNT_POSI & TX_FIFO_COUNT_MASK;
}

UINT8 uart_is_tx_fifo_empty(VOID)
{
    return (REG_READ(REG_UART_FIFO_STATUS) & TX_FIFO_EMPTY) != 0 ? 1 : 0;
}

UINT8 uart_is_tx_fifo_full(VOID)
{
    return (REG_READ(REG_UART_FIFO_STATUS) & TX_FIFO_FULL) != 0 ? 1 : 0;
}

VOID uart_set_tx_stop_end_int(UINT8 set)
{
    UINT32 reg;

    reg = REG_READ(REG_UART_INTR_ENABLE);
    
    if(set == 1)
    {
        reg |= UART_TX_STOP_END_EN;
    }
    else
    {
        reg &= ~UART_TX_STOP_END_EN;
    }
    
    REG_WRITE(REG_UART_INTR_ENABLE, reg);
}

// EOF


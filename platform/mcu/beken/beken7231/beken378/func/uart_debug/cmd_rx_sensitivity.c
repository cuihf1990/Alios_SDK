#include "include.h"
#include "cmd_rx_sensitivity.h"
#include "mem_pub.h"
#include "str_pub.h"
#include "rx_sensitivity_pub.h"
#include "fake_clock_pub.h"
#include "uart_debug_pub.h"
#include "schedule_pub.h"
#include "rtos_pub.h"
#include "error.h"

UINT32 mode = RXSENS_DEFUALT_MODE;
UINT32 duration = RXSENS_DEFUALT_DURATION;
UINT32 channel = RXSENS_DEFUALT_CHANNEL;


#if CFG_RX_SENSITIVITY_TEST
mico_timer_t rx_sens_tmr = {0};
UINT32 g_rxsens_start = 0;
#endif

void rxsens_ct_hdl(void *param)
{
#if CFG_RX_SENSITIVITY_TEST
    rx_get_rx_result_end();
    rx_get_rx_result_begin();
#endif // CFG_RX_SENSITIVITY_TEST
}

int do_rx_sensitivity(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    OSStatus err;
    char cmd0 = 0;
    char cmd1 = 0;
    UINT8 fail = 0;

#if CFG_RX_SENSITIVITY_TEST
    UINT8 ret;
#endif

    UINT32 arg_id = 1;
    UINT32 arg_cnt = argc;

#if CFG_RX_SENSITIVITY_TEST
    uint32_t t_ms = 0;
#endif

    /*step0, parameter conversion*/
    while(arg_cnt > 1)
    {
        if(arg_cnt > 1)
        {
            cmd0 = argv[arg_id][0];
            cmd1 = argv[arg_id][1];
        }

        switch(cmd0)
        {
        case '-':
        {
            arg_cnt -= 1;

            if(arg_cnt < 1)
            {
                fail = 1;
                break;
            }

            arg_cnt -= 1;
            switch(cmd1)
            {
            case 'm':
                mode = os_strtoul(argv[arg_id + 1], NULL, 10);
                break;

            case 'd':
                duration = os_strtoul(argv[arg_id + 1], NULL, 10);
                break;

            case 'c':
                channel = os_strtoul(argv[arg_id + 1], NULL, 10);
                break;

            case 'l':
#if CFG_RX_SENSITIVITY_TEST
                err = mico_rtos_stop_timer(&rx_sens_tmr);
	            ASSERT(kNoErr == err);
                return 0;
#endif

            default:
                fail = 1;
                break;
            }
        }
        break;

        default:
            fail = 1;
            break;
        }

        if(fail)
        {
            return 1;
        }

        arg_id += 2;
    }

    /*step1, parameter check*/
    if(!((0 < channel)
            || (14 > channel)))
    {
        return 1;
    }

    if((mode != 1) && (mode != 0) )
    {
        return 1;
    }

    /*step2, handle*/
#if CFG_RX_SENSITIVITY_TEST
    ret = rs_set_mode(mode);
    if(ret)
    {
        return 1;
    }

    ret = rs_set_channel(channel);
    if(ret)
    {
        return 1;
    }

    g_rxsens_start = 1;
    rs_test();
    rx_get_rx_result_begin();

    t_ms = fclk_from_sec_to_tick(duration);

	err = mico_rtos_init_timer(&rx_sens_tmr, 
							t_ms, 
							rxsens_ct_hdl, 
							(void *)0);
    ASSERT(kNoErr == err);
	err = mico_rtos_start_timer(&rx_sens_tmr);
	ASSERT(kNoErr == err);
    
#endif // CFG_RX_SENSITIVITY_TEST

    return 0;
}

// eof


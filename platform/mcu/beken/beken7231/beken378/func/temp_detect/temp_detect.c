#include "include.h"
#include "arm_arch.h"

#if CFG_USE_TEMPERATURE_DETECT
#include "target_util_pub.h"
#include "mem_pub.h"
#include "drv_model_pub.h"
#include "sys_ctrl_pub.h"
#include "saradc_pub.h"
#include "uart_pub.h"

#include "temp_detect_pub.h"
#include "temp_detect.h"

DD_HANDLE tmp_detect_hdl;
saradc_desc_t tmp_detect_desc;
UINT16 tmp_detect_buff[ADC_TEMP_DETECT_BUFFER_SIZE];
TEMP_DETECT_CONFIG_ST g_temp_detect_config;

#if CFG_SUPPORT_CALIBRATION
extern int calibration_main(void);
extern INT32 rwnx_cal_load_trx_rcbekn_reg_val(void);
extern INT32 rwnx_cal_save_trx_rcbekn_reg_val(void);
extern void do_calibration_in_temp_dect(void);
#endif

void temp_detect_handler(void);

/*---------------------------------------------------------------------------*/
PROCESS(temp_detect_process, "temp_detect_process");
/*---------------------------------------------------------------------------*/

static void temp_detect_desc_init(void)
{
    os_memset(&tmp_detect_buff[0], 0, sizeof(UINT16)*ADC_TEMP_DETECT_BUFFER_SIZE);

    tmp_detect_desc.channel = ADC_TEMP_SENSER_CHANNEL;
    tmp_detect_desc.pData = &tmp_detect_buff[0];
    tmp_detect_desc.data_buff_size = ADC_TEMP_DETECT_BUFFER_SIZE;
    tmp_detect_desc.mode = (ADC_CONFIG_MODE_CONTINUE << 0)
                           | (ADC_CONFIG_MODE_36DIV << 2);

    tmp_detect_desc.has_data                = 0;
    tmp_detect_desc.current_read_data_cnt   = 0;
    tmp_detect_desc.current_sample_data_cnt = 0;
    tmp_detect_desc.p_Int_Handler = temp_detect_handler;
}

static void temp_detect_enable_config_sysctrl(void)
{
    UINT32 param;

    param = BLK_BIT_TEMPRATURE_SENSOR;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BLK_ENABLE, &param);
}

static void temp_detect_disable_config_sysctrl(void)
{
    UINT32 param;
    param = BLK_BIT_TEMPRATURE_SENSOR;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BLK_DISABLE, &param);
}

UINT32 temp_detect_init(void)
{    
    process_start(&temp_detect_process, NULL);
    return 0;
}

UINT32 temp_detect_uninit(void)
{
    return 0;
}

static UINT32 temp_detect_open(void)
{
    UINT32 status;

    tmp_detect_hdl = ddev_open(SARADC_DEV_NAME, &status, (UINT32)&tmp_detect_desc);
    if(DD_HANDLE_UNVALID == tmp_detect_hdl)
    {
        return SARADC_FAILURE;
    }

    return SARADC_SUCCESS;
}

static UINT32 temp_detect_close(void)
{
    UINT32 status;

    status = ddev_close(tmp_detect_hdl);
    if(DRV_FAILURE == status )
    {
        return SARADC_FAILURE;
    }

    return SARADC_SUCCESS;
}

static UINT32 temp_detect_enable(void)
{
    UINT32 err = SARADC_SUCCESS;

    temp_detect_desc_init();
    temp_detect_enable_config_sysctrl();

    err = temp_detect_open();
    if(err == SARADC_FAILURE)
    {
        TMP_DETECT_FATAL("Can't open saradc, have you register this device?\r\n");
        return err;
    }
    TMP_DETECT_PRT("saradc_open is ok \r\n");

    return SARADC_SUCCESS;
}

static void temp_detect_disable(void)
{
    temp_detect_close();
    temp_detect_disable_config_sysctrl();
    TMP_DETECT_PRT("saradc_open is close \r\n");
}

static UINT8 temp_detect_get_wifi_traffic_idle(void)
{
    return 1;
}

void temp_detect_do_calibration(void)
{
#if CFG_SUPPORT_CALIBRATION
    TMP_DETECT_WARN("do calibration...\r\n");
    g_temp_detect_config.do_cali_flag = 1;
    rwnx_cal_save_trx_rcbekn_reg_val();
    calibration_main();
    rwnx_cal_load_trx_rcbekn_reg_val();
    g_temp_detect_config.do_cali_flag = 0;
    TMP_DETECT_WARN("calibration done!!!\r\n");
#endif
}

void temp_detect_do_calibration_temp(void)
{
#if CFG_SUPPORT_CALIBRATION
    TMP_DETECT_WARN("do calibration...\r\n");
    g_temp_detect_config.do_cali_flag = 1;
    rwnx_cal_save_trx_rcbekn_reg_val();
    do_calibration_in_temp_dect();
    rwnx_cal_load_trx_rcbekn_reg_val();
    g_temp_detect_config.do_cali_flag = 0;
    TMP_DETECT_WARN("calibration done!!!\r\n");
#endif
}

static void temp_detect_timer_handler(void *data)
{
    struct etimer *timer = (struct etimer *)data;

    if(timer == &g_temp_detect_config.detect_timer)
    {
        if(etimer_expired(&g_temp_detect_config.detect_timer))
            temp_detect_enable();
    }
    else if(timer == &g_temp_detect_config.calibration_timer)
    {
        if(etimer_expired(&g_temp_detect_config.calibration_timer))
        {
            if(temp_detect_get_wifi_traffic_idle())
            {
                //temp_detect_do_calibration();
                temp_detect_do_calibration_temp();

                PROCESS_CONTEXT_BEGIN(&temp_detect_process);
                etimer_restart(&g_temp_detect_config.detect_timer);
                PROCESS_CONTEXT_END(&temp_detect_process);
            }
            else
            {
                PROCESS_CONTEXT_BEGIN(&temp_detect_process);
                etimer_restart(&g_temp_detect_config.calibration_timer);
                PROCESS_CONTEXT_END(&temp_detect_process);
            }
        }
    }
}

static void temp_detect_polling_handler(void)
{
    {
        UINT32 dist, cur_val, last_val;

        cur_val = tmp_detect_desc.pData[ADC_TEMP_DETECT_BUFFER_SIZE-1];
        last_val = g_temp_detect_config.last_detect_val;
        dist = (cur_val > last_val) ? (cur_val - last_val) : (last_val - cur_val);

        TMP_DETECT_WARN("%d seconds: last:%d, cur:%d, thr:%d\r\n",
                        g_temp_detect_config.detect_intval,
                        last_val,
                        cur_val,
                        g_temp_detect_config.detect_thre);

    if(g_temp_detect_config.last_detect_val == 0)
    {
        g_temp_detect_config.detect_intval = ADC_TMEP_DETECT_INTVAL;

        PROCESS_CONTEXT_BEGIN(&temp_detect_process);
        etimer_set(&g_temp_detect_config.detect_timer, ADC_TMEP_DETECT_INTVAL * CLOCK_CONF_SECOND);
        PROCESS_CONTEXT_END(&temp_detect_process);
    }
	
        if(dist > g_temp_detect_config.detect_thre)
        {
            g_temp_detect_config.last_detect_val = cur_val;

            PROCESS_CONTEXT_BEGIN(&temp_detect_process);
            etimer_set(&g_temp_detect_config.calibration_timer, CLOCK_CONF_SECOND);
            PROCESS_CONTEXT_END(&temp_detect_process);

            return;
        }
    }

    PROCESS_CONTEXT_BEGIN(&temp_detect_process);
    etimer_restart(&g_temp_detect_config.detect_timer);
    PROCESS_CONTEXT_END(&temp_detect_process);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(temp_detect_process, ev, data)
{
    PROCESS_BEGIN();

    {
        clock_time_t interval;
        g_temp_detect_config.last_detect_val = 0;
        g_temp_detect_config.detect_thre = ADC_TMEP_DO_CALIBRATION_TXOUTPOWR_THRE;
        g_temp_detect_config.detect_intval = 2;//ADC_TMEP_DETECT_INTVAL;
        g_temp_detect_config.do_cali_flag = 0;

        interval = g_temp_detect_config.detect_intval * CLOCK_CONF_SECOND;
        etimer_set(&g_temp_detect_config.detect_timer, interval);
    }

    while(1)
    {
        PROCESS_YIELD();
        if(ev == PROCESS_EVENT_POLL)
        {
            temp_detect_polling_handler();
        }
        else if(ev == PROCESS_EVENT_TIMER)
        {
            temp_detect_timer_handler(data);
        }
        else if(ev == PROCESS_EVENT_EXIT)
        {
            TMP_DETECT_PRT("temp_detect_process exit process\r\n");
            temp_detect_disable();
        }
        else if(ev == PROCESS_EVENT_EXITED)
        {
            struct process *exit_p = (struct process *)data;
            TMP_DETECT_PRT("%s exit in temp_detect_process\r\n",
                           PROCESS_NAME_STRING(exit_p));
        }
    }

    PROCESS_END();
}

/*---------------------------------------------------------------------------*/
void temp_detect_handler(void)
{
    if(tmp_detect_desc.current_sample_data_cnt >= tmp_detect_desc.data_buff_size)
    {
        TMP_DETECT_PRT("buff:%p,%d,%d,%d,%d,%d\r\n", tmp_detect_desc.pData,
                       tmp_detect_desc.pData[0], tmp_detect_desc.pData[1],
                       tmp_detect_desc.pData[2], tmp_detect_desc.pData[3], 
                       tmp_detect_desc.pData[4]);

        temp_detect_disable();
        if(process_is_running(&temp_detect_process))
        {
            process_poll(&temp_detect_process);
        }
    }
}

void temp_detect_change_configuration(UINT32 intval, UINT32 thre)
{
    clock_time_t interval;

    if(intval == 0)
        intval = ADC_TMEP_DETECT_INTVAL;
    if(thre == 0)
        thre = ADC_TMEP_DO_CALIBRATION_TXOUTPOWR_THRE;

    TMP_DETECT_PRT("config: intval:0x%x, thre:0x%x\r\n", intval, thre);

    g_temp_detect_config.detect_thre = thre;
    g_temp_detect_config.detect_intval = intval;
    interval = g_temp_detect_config.detect_intval * CLOCK_CONF_SECOND;

    PROCESS_CONTEXT_BEGIN(&temp_detect_process);
    etimer_set(&g_temp_detect_config.detect_timer, interval);
    PROCESS_CONTEXT_END(&temp_detect_process);
}

UINT8 temp_detct_get_cali_flag(void)
{
    return (UINT8)(g_temp_detect_config.do_cali_flag);
}

#endif  // CFG_USE_TEMPERATURE_DETECT

//EOF


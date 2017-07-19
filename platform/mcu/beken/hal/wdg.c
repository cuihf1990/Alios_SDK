#include "hal/soc/soc.h"
#include "mico_rtos.h"
#include "wdt_pub.h"
#include "icu_pub.h"

int32_t hal_wdg_init(uint32_t timeout)
{
    uint32_t para;

    para = PWD_ARM_WATCHDOG_CLK_BIT;
    icu_ctrl(CMD_CLK_PWR_UP, &para);
    wdt_ctrl(WCMD_SET_PERIOD, &timeout);
    return 0;
}

void hal_wdg_reload(void)
{
    wdt_ctrl(WCMD_CLEAR_COUNTER, 0);
}

int32_t hal_wdg_finalize(void)
{
    uint32_t para;

    para = PWD_ARM_WATCHDOG_CLK_BIT;
    icu_ctrl(CMD_CLK_PWR_DOWN, &para);
    return 0;
}
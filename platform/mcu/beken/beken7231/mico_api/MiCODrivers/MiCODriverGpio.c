/**
 ******************************************************************************
 * @file    MicoDriverGpio.h
 * @author  William Xu
 * @version V1.0.0
 * @date    16-Sep-2014
 * @brief   This file provides all the headers of GPIO operation functions.
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2014 MXCHIP Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 */
#include "include.h"
#include "rtos_pub.h"
#include "BkDriverGpio.h"

void gpio_test_func(uint8_t cmd, uint8_t id, uint32_t mode, void(*p_handle)(char))
{
    if(cmd == 1)
        BkGpioEnableIRQ(id, mode, p_handle, NULL);
    else
        BkGpioDisableIRQ(id);
}
OSStatus MicoGpioEnableIRQ( uint32_t gpio, platform_gpio_irq_trigger_t trigger, platform_gpio_irq_callback_t handler, void *arg )
{
    BkGpioEnableIRQ(gpio, trigger, handler, arg);
}
OSStatus MicoGpioDisableIRQ( uint32_t gpio )
{
    BkGpioDisableIRQ(gpio);
}

OSStatus MicoGpioOp(char cmd, uint32_t id, char mode)
{
    uint32_t command, mode_set;

    if(cmd == '0')
        command = CMD_GPIO_CFG;
    else if(cmd == '1')
        command = CMD_GPIO_INPUT;
    else if(cmd == '2')
        command = CMD_GPIO_OUTPUT;
    else
        command = CMD_GPIO_OUTPUT_REVERSE;

    if(mode == '0')
        mode_set = GMODE_INPUT_PULLDOWN;
    else if(mode == '1')
        mode_set = GMODE_OUTPUT;
    else if(mode == '2')
        mode_set = GMODE_INPUT_PULLUP;
    else
        mode_set = GMODE_INPUT;
    mode_set = (mode_set << 8) | id;

    return sddev_control(GPIO_DEV_NAME, command, &mode_set);
}


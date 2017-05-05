/****************************************************************************
 *
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 ****************************************************************************/

 /**
 * @file
 * @brief      dynamic loader test
 * @details
 * @author     xzf
 * @date       2016-12-14
 * @version    0.1
 */

#include <stdint.h>
#include "dload.h"
#include "dload_port.h"

#if (YUNOS_CONFIG_DLOAD_SUPPORT > 0)

#define TEST_IMAGE_FD    0x1003D000         /* stub fd as image base, see nsfs_get_image_base () */

#define CONFIG_DLOAD_MODULE_SET_RESULT      0
#define CONFIG_DLOAD_MODULE_TEST_PERFORMACE 0

#if (CONFIG_DLOAD_MODULE_SET_RESULT > 0)    /* if dload test module set memory flag */
#define TEST_RESULT_BASE  0x2003fff0u       /*256K ram */
#define RESULT_LOAD       0x12345678u
#define RESULT_UNLOAD     0x9ABCDEF0u
#define RESULT_SUCCESS    0xffffffffu
#endif

#if (CONFIG_DLOAD_MODULE_SET_RESULT > 0)
extern int yunos_sched_disable(void);
extern int yunos_sched_enable(void);
#endif

void dload_test(void)
{
    int32_t ret;

#if (CONFIG_DLOAD_MODULE_SET_RESULT > 0)
    uint32_t *result = (uint32_t *)TEST_RESULT_BASE;
    *result = 0x01020304;
#endif

    LOG_D("===================dload_test:%s===================",__TIME__);

#if (CONFIG_DLOAD_MODULE_SET_RESULT > 0)
    yunos_sched_disable();
#endif

    ymod_init();

    ymod_load(TEST_IMAGE_FD);

#if (CONFIG_DLOAD_MODULE_SET_RESULT > 0)
    if(*result == RESULT_SUCCESS){
        LOG_D("load test success");
    }else{
        LOG_E("load test error:%x",*result);
    }
#endif

    ymod_show_status();

    ret = ymod_unload(TEST_IMAGE_FD);
    if(ret != 0){
        LOG_E("unload test error:%x",ret);
    }else {
#if (CONFIG_DLOAD_MODULE_SET_RESULT > 0)
        if(*result == 0x9ABCDEF0u){
            LOG_D("unload test success");
        }else{
            LOG_E("unload test error:%x",*result);
        }
#endif
    }

    ymod_show_status();

#if (CONFIG_DLOAD_MODULE_SET_RESULT > 0)
    yunos_sched_enable();
#endif
}

#endif /*YUNOS_CONFIG_DLOAD_SUPPORT*/

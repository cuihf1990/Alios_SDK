/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <k_api.h>
#include <k_mm_region.h>
#include <test_fw.h>
#include "mm_region_test.h"

#define MODULE_NAME "mm_region_break"
#define YOS_MM_ALLOC_DEPTH  0

static k_mm_region_head_t  my_mm_region_list_head,my_mm_region_list_head2,my_mm_region_list_head_2;
static void * mp[11]       = {0};
static void * mp2[11]      = {0};

static uint8_t mm_region_break_case1(void)
{
    kstat_t  ret;
    uint8_t *ptmp       = NULL;
    uint8_t *ptmp1      = NULL;
    uint8_t *ptmp2      = NULL;
    uint32_t r_num      = MM_REGION_VALID_REGION + MM_REGION_INVALID_REGION;
    size_t   free       = 0;
    size_t   check_free = 0;
    size_t   i          = 0;
    size_t   allocator  = (size_t)__builtin_return_address(YOS_MM_ALLOC_DEPTH);

    memset(&my_mm_region_list_head,0,sizeof(k_mm_region_head_t));
    ret  =  yunos_mm_region_init(NULL,NULL, r_num);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret  =  yunos_mm_region_init(&my_mm_region_list_head,regions, 0);
    MYASSERT(ret == YUNOS_INV_PARAM);

    ret  =  yunos_mm_region_init(NULL,regions, r_num);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret  =  yunos_mm_region_init(&my_mm_region_list_head,regions, r_num);
    MYASSERT(ret == YUNOS_SUCCESS);

    check_free = yunos_mm_region_get_free_size(NULL);
    MYASSERT(check_free == 0);

    check_free = yunos_mm_region_get_free_size(&my_mm_region_list_head);

    r_num -= MM_REGION_INVALID_REGION;
    MYASSERT(check_free == MM_REGION_1_SIZE + MM_REGION_2_SIZE - r_num * sizeof(k_mm_region_list_t));
    MYASSERT(my_mm_region_list_head.frag_num == r_num);

    ret = yunos_mm_bf_alloc(&my_mm_region_list_head, (void **)&ptmp,0,allocator);
    MYASSERT(ret == YUNOS_MM_ALLOC_SIZE_ERR);

    ret = yunos_mm_bf_alloc(NULL, (void **)&ptmp,48,allocator);
    MYASSERT(ret == YUNOS_NULL_PTR);


    ret = yunos_mm_bf_alloc(&my_mm_region_list_head, (void **)NULL, 48,allocator);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mm_bf_alloc(&my_mm_region_list_head, (void **)&ptmp, MM_REGION_TOTAL_SIZE,allocator);
    MYASSERT(ret == YUNOS_NO_MEM);

    ret = yunos_mm_bf_alloc(&my_mm_region_list_head, (void **)&ptmp, MM_REGION_MAX_SIZE,allocator);
    MYASSERT(ret == YUNOS_NO_MEM);

    ret = yunos_mm_bf_alloc(&my_mm_region_list_head, (void **)&ptmp, 48,allocator);

    MYASSERT(ret == YUNOS_SUCCESS);
    check_free -= sizeof(k_mm_region_list_t) + 48;
    free = yunos_mm_region_get_free_size(&my_mm_region_list_head);
    MYASSERT(free == check_free);
    MYASSERT(my_mm_region_list_head.frag_num == ++r_num);
    MYASSERT(NULL != ptmp);

    ret = yunos_mm_bf_alloc(&my_mm_region_list_head, (void **)&ptmp1, 60,allocator);

    MYASSERT(ret == YUNOS_SUCCESS);
    check_free -= sizeof(k_mm_region_list_t) + 60;
    free = yunos_mm_region_get_free_size(&my_mm_region_list_head);
    MYASSERT(free == check_free);
    MYASSERT(my_mm_region_list_head.frag_num == ++r_num);
    MYASSERT(NULL != ptmp1);

    /* the first region is exhausted*/
    ret = yunos_mm_bf_alloc(&my_mm_region_list_head, (void **)&ptmp2, free - MM_REGION_2_SIZE,allocator);

    MYASSERT(ret == YUNOS_SUCCESS);
    check_free = MM_REGION_2_SIZE -sizeof(k_mm_region_list_t);
    free = yunos_mm_region_get_free_size(&my_mm_region_list_head);
    MYASSERT(free == check_free);
    MYASSERT(my_mm_region_list_head.frag_num == r_num);
    MYASSERT(NULL != ptmp2);

    ret = yunos_mm_xf_free(NULL, ptmp);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mm_xf_free(&my_mm_region_list_head, ptmp);
    MYASSERT(ret == YUNOS_SUCCESS);
    check_free += 48;
    free = yunos_mm_region_get_free_size(&my_mm_region_list_head);
    MYASSERT(free == check_free);
    MYASSERT(my_mm_region_list_head.frag_num == r_num);

    ret = yunos_mm_xf_free(&my_mm_region_list_head, ptmp1);
    MYASSERT(ret == YUNOS_SUCCESS);
    check_free += 60 + sizeof(k_mm_region_list_t);
    free = yunos_mm_region_get_free_size(&my_mm_region_list_head);
    MYASSERT(free == check_free);
    MYASSERT(my_mm_region_list_head.frag_num == --r_num);

    ret = yunos_mm_xf_free(&my_mm_region_list_head, ptmp2);
    MYASSERT(ret == YUNOS_SUCCESS);
    check_free = MM_REGION_1_SIZE + MM_REGION_2_SIZE -MM_REGION_VALID_REGION*sizeof(k_mm_region_list_t);
    free = yunos_mm_region_get_free_size(&my_mm_region_list_head);
    MYASSERT(free == check_free);
    MYASSERT(my_mm_region_list_head.frag_num == --r_num);

    memset(regions1[0].start,0,regions1[0].len);

    ret  =  yunos_mm_region_init(&my_mm_region_list_head_2,regions1, 1);
    MYASSERT(ret == YUNOS_SUCCESS);
    //add for best fit test
    for(i=1;i<10;i++){
        ret = yunos_mm_bf_alloc(&my_mm_region_list_head_2, &mp[i-1],4*i+2,allocator);
        MYASSERT(ret == YUNOS_SUCCESS);
    }
    for(i=1;i<10;i++){
        if(i%2 == 0){
            ret = yunos_mm_xf_free(&my_mm_region_list_head_2, mp[i-1]);
            MYASSERT(ret == YUNOS_SUCCESS);
        }
    }
    for(i=1;i<10;i++){
        if(i%2 == 0){
            ret = yunos_mm_bf_alloc(&my_mm_region_list_head_2, &mp2[i-1],4*i+1,allocator);
            MYASSERT(ret == YUNOS_SUCCESS);
            MYASSERT(mp[i-1] == mp2[i-1]);
            ret = yunos_mm_xf_free(&my_mm_region_list_head_2, mp2[i-1]);
            MYASSERT(ret == YUNOS_SUCCESS);
        }
    }

    for(i=1;i<10;i++){
        if(i%2 != 0){
            ret = yunos_mm_xf_free(&my_mm_region_list_head_2, mp[i-1]);
            MYASSERT(ret == YUNOS_SUCCESS);
        }
    }
    return 0;
}
static uint8_t mm_region_break_case2(void)
{
    kstat_t  ret;
    uint8_t *ptmp       = NULL;
    uint8_t *ptmp1      = NULL;
    uint8_t *ptmp2      = NULL;
    uint32_t r_num      = MM_REGION_VALID_REGION + MM_REGION_INVALID_REGION;
    size_t   free       = 0;
    size_t   check_free = 0;
    size_t   allocator  = (size_t)__builtin_return_address(YOS_MM_ALLOC_DEPTH);

    ret  =  yunos_mm_region_init(NULL,NULL, r_num);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret  =  yunos_mm_region_init(&my_mm_region_list_head2,regions2, 0);
    MYASSERT(ret == YUNOS_INV_PARAM);

    ret  =  yunos_mm_region_init(NULL,regions2, r_num);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret  =  yunos_mm_region_init(&my_mm_region_list_head2,regions2, r_num);
    MYASSERT(ret == YUNOS_SUCCESS);

    check_free = yunos_mm_region_get_free_size(NULL);
    MYASSERT(check_free == 0);

    check_free = yunos_mm_region_get_free_size(&my_mm_region_list_head2);

    r_num -= MM_REGION_INVALID_REGION;
    MYASSERT(check_free == MM_REGION_1_SIZE + MM_REGION_2_SIZE - r_num * sizeof(k_mm_region_list_t));
    MYASSERT(my_mm_region_list_head2.frag_num == r_num);

    ret = yunos_mm_ff_alloc(&my_mm_region_list_head2, (void **)&ptmp,0,allocator);
    MYASSERT(ret == YUNOS_MM_ALLOC_SIZE_ERR);

    ret = yunos_mm_ff_alloc(NULL, (void **)&ptmp,48,allocator);
    MYASSERT(ret == YUNOS_NULL_PTR);


    ret = yunos_mm_ff_alloc(&my_mm_region_list_head2, (void **)NULL, 48,allocator);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mm_ff_alloc(&my_mm_region_list_head2, (void **)&ptmp, MM_REGION_TOTAL_SIZE,allocator);
    MYASSERT(ret == YUNOS_NO_MEM);

    ret = yunos_mm_ff_alloc(&my_mm_region_list_head2, (void **)&ptmp, MM_REGION_MAX_SIZE,allocator);
    MYASSERT(ret == YUNOS_NO_MEM);

    ret = yunos_mm_ff_alloc(&my_mm_region_list_head2, (void **)&ptmp, 48,allocator);

    MYASSERT(ret == YUNOS_SUCCESS);
    check_free -= sizeof(k_mm_region_list_t) + 48;
    free = yunos_mm_region_get_free_size(&my_mm_region_list_head2);
    MYASSERT(free == check_free);
    MYASSERT(my_mm_region_list_head2.frag_num == ++r_num);
    MYASSERT(NULL != ptmp);

    ret = yunos_mm_ff_alloc(&my_mm_region_list_head2, (void **)&ptmp1, 60, allocator);
    MYASSERT(ret == YUNOS_SUCCESS);

    check_free -= sizeof(k_mm_region_list_t) + 60;
    free = yunos_mm_region_get_free_size(&my_mm_region_list_head2);
    MYASSERT(free == check_free);
    MYASSERT(my_mm_region_list_head2.frag_num == ++r_num);
    MYASSERT(NULL != ptmp1);

    /* the first region is exhausted*/
    ret = yunos_mm_ff_alloc(&my_mm_region_list_head2, (void **)&ptmp2, free - MM_REGION_2_SIZE,allocator);
    MYASSERT(ret == YUNOS_SUCCESS);
    check_free = MM_REGION_2_SIZE -sizeof(k_mm_region_list_t);
    free = yunos_mm_region_get_free_size(&my_mm_region_list_head2);
    MYASSERT(free == check_free);
    MYASSERT(my_mm_region_list_head2.frag_num == r_num);
    MYASSERT(NULL != ptmp2);

    ret = yunos_mm_xf_free(NULL, ptmp);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mm_xf_free(&my_mm_region_list_head2, ptmp);
    MYASSERT(ret == YUNOS_SUCCESS);
    check_free += 48;
    free = yunos_mm_region_get_free_size(&my_mm_region_list_head2);
    MYASSERT(free == check_free);
    MYASSERT(my_mm_region_list_head2.frag_num == r_num);

#if(YUNOS_CONFIG_MM_DEBUG>0)
    ret = check_mm_info_func();
    MYASSERT(ret == YUNOS_SUCCESS);
#endif

    ret = yunos_mm_xf_free(&my_mm_region_list_head2, ptmp1);
    MYASSERT(ret == YUNOS_SUCCESS);
    check_free += 60 + sizeof(k_mm_region_list_t);
    free = yunos_mm_region_get_free_size(&my_mm_region_list_head2);
    MYASSERT(free == check_free);
    MYASSERT(my_mm_region_list_head2.frag_num == --r_num);

    ret = yunos_mm_xf_free(&my_mm_region_list_head2, ptmp2);
    MYASSERT(ret == YUNOS_SUCCESS);
    check_free = MM_REGION_1_SIZE + MM_REGION_2_SIZE -MM_REGION_VALID_REGION*sizeof(k_mm_region_list_t);
    free = yunos_mm_region_get_free_size(&my_mm_region_list_head2);
    MYASSERT(free == check_free);
    MYASSERT(my_mm_region_list_head2.frag_num == --r_num);

    return 0;
}


static const test_func_t mm_region_func_runner[] = {
    mm_region_break_case1,
    mm_region_break_case2,
    NULL
};

void mm_region_break_test(void)
{
    kstat_t ret;

    task_mm_region_entry_register(MODULE_NAME, (test_func_t *)mm_region_func_runner, sizeof(mm_region_func_runner) / sizeof(test_func_t));

    ret = yunos_task_dyn_create(&task_mm_region, MODULE_NAME, 0, TASK_MM_REGION_PRI,
                                0, TASK_TEST_STACK_SIZE, task_mm_region_entry, 1);

    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}


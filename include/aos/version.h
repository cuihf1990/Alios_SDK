/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef YOC_SYS_VERSION
#define YOC_SYS_VERSION

const char *get_aos_product_model(void);
const char *get_aos_os_version (void);
const char *get_aos_kernel_version (void);
const char *get_aos_app_version (void);
const char *get_aos_device_name(void);
void dump_sys_info(void);

#endif /* YOC_SYS_VERSION */


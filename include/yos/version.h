/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef YOC_SYS_VERSION
#define YOC_SYS_VERSION

const char *get_yos_product_model(void);
const char *get_yos_os_version (void);
const char *get_yos_kernel_version (void);
const char *get_yos_app_version (void);
const char *get_yos_device_name(void);
void dump_sys_info(void);

#endif /* YOC_SYS_VERSION */


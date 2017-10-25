/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <aos/aos.h>
#include <aos/kernel.h>

extern int vfs_init(void);
extern int vfs_device_init(void);
extern int aos_kv_init(void);
extern void ota_service_init(void);
extern int aos_framework_init(void);
extern int application_start(int argc, char **argv);
extern void trace_start(void);
int aos_cloud_init(void);

#ifdef AOS_BINS
#include <k_api.h>
struct app_info_t {
     void (*app_entry)(void *ksyscall_tbl, void *fsyscall_tbl, int argc, char *argv[]);
     unsigned int data_ram_start;
     unsigned int data_ram_end;
     unsigned int data_flash_begin;
     unsigned int bss_start;
     unsigned int bss_end;
     unsigned int heap_start;
     unsigned int heap_end;
};

struct framework_info_t {
     void (*framework_entry)(void *syscall_tbl, int argc, char *argv[]);
     unsigned int data_ram_start;
     unsigned int data_ram_end;
     unsigned int data_flash_begin;
     unsigned int bss_start;
     unsigned int bss_end;
     unsigned int heap_start;
     unsigned int heap_end;
};

extern void *syscall_ktbl[];
extern char  app_info_addr;
extern char  framework_info_addr;

extern k_mm_head  *g_kmm_head;

struct framework_info_t *framework_info = (struct framework_info_t *)&framework_info_addr;
struct app_info_t *app_info = (struct app_info_t *)&app_info_addr;

static void app_pre_init(void)
{
    memcpy((void *)(app_info->data_ram_start), (void *)(app_info->data_flash_begin),
           app_info->data_ram_end - app_info->data_ram_start);
    memset((void *)(app_info->bss_start), 0, app_info->bss_end - app_info->bss_start);

    krhino_add_mm_region(g_kmm_head, (void *)(app_info->heap_start),
                        app_info->heap_end - app_info->heap_start);

    krhino_mm_leak_region_init((void *)(app_info->data_ram_start), (void *)(app_info->data_ram_end));
    krhino_mm_leak_region_init((void *)(app_info->bss_start), (void *)(app_info->bss_end));
}

static void framework_pre_init(void)
{
    memcpy((void *)(framework_info->data_ram_start), (void *)(framework_info->data_flash_begin),
           framework_info->data_ram_end - framework_info->data_ram_start);
    memset((void *)(framework_info->bss_start), 0, framework_info->bss_end - framework_info->bss_start);

    krhino_add_mm_region(g_kmm_head, (void *)(framework_info->heap_start),
                        framework_info->heap_end - framework_info->heap_start);

    krhino_mm_leak_region_init((void *)(framework_info->data_ram_start), (void *)(framework_info->data_ram_end));
    krhino_mm_leak_region_init((void *)(framework_info->bss_start), (void *)(framework_info->bss_end));
}
#endif

int aos_kernel_init(void)
{
#ifdef AOS_VFS
    vfs_init();
    vfs_device_init();
#endif
    
#ifdef CONFIG_AOS_CLI
    aos_cli_init();
#endif
    
#ifdef AOS_KV
    aos_kv_init();
#endif
    
#ifdef AOS_LOOP
    aos_loop_init();
#endif

#ifdef VCALL_RHINO
    trace_start();
#endif

#ifdef AOS_FOTA 
    ota_service_init();
#endif

#ifdef AOS_BINS
    app_pre_init();
    framework_pre_init();

    if (framework_info->framework_entry) {
        framework_info->framework_entry((void *)syscall_ktbl, 0, NULL);
    }
#else
    aos_framework_init();
    application_start(0, NULL);
#endif

    return 0;
}



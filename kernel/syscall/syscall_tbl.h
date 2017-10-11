/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */


#if (1)
#define SYS_AOS_ALLOC_TRACE 0
SYSCALL(SYS_AOS_ALLOC_TRACE, aos_alloc_trace)
#endif

#if (1)
#define SYS_AOS_CANCEL_DELAYED_ACTION 1
SYSCALL(SYS_AOS_CANCEL_DELAYED_ACTION, aos_cancel_delayed_action)
#endif

#if (1)
#define SYS_AOS_CANCEL_POLL_READ_FD 2
SYSCALL(SYS_AOS_CANCEL_POLL_READ_FD, aos_cancel_poll_read_fd)
#endif

#if (1)
#define SYS_AOS_CANCEL_WORK 3
SYSCALL(SYS_AOS_CANCEL_WORK, aos_cancel_work)
#endif

#if (1)
#define SYS_AOS_CLI_INIT 4
SYSCALL(SYS_AOS_CLI_INIT, aos_cli_init)
#endif

#if (1)
#define SYS_AOS_CLI_REGISTER_COMMAND 5
SYSCALL(SYS_AOS_CLI_REGISTER_COMMAND, aos_cli_register_command)
#endif

#if (1)
#define SYS_AOS_CLI_REGISTER_COMMANDS 6
SYSCALL(SYS_AOS_CLI_REGISTER_COMMANDS, aos_cli_register_commands)
#endif

#if (1)
#define SYS_AOS_CLI_STOP 7
SYSCALL(SYS_AOS_CLI_STOP, aos_cli_stop)
#endif

#if (1)
#define SYS_AOS_CLI_UNREGISTER_COMMAND 8
SYSCALL(SYS_AOS_CLI_UNREGISTER_COMMAND, aos_cli_unregister_command)
#endif

#if (1)
#define SYS_AOS_CLI_UNREGISTER_COMMANDS 9
SYSCALL(SYS_AOS_CLI_UNREGISTER_COMMANDS, aos_cli_unregister_commands)
#endif

#if (1)
#define SYS_AOS_CLOSE 10
SYSCALL(SYS_AOS_CLOSE, aos_close)
#endif

#if (1)
#define SYS_AOS_CLOSEDIR 11
SYSCALL(SYS_AOS_CLOSEDIR, aos_closedir)
#endif

#if (1)
#define SYS_AOS_CURRENT_LOOP 12
SYSCALL(SYS_AOS_CURRENT_LOOP, aos_current_loop)
#endif

#if (1)
#define SYS_AOS_FCNTL 13
SYSCALL(SYS_AOS_FCNTL, aos_fcntl)
#endif

#if (1)
#define SYS_AOS_FREE 14
SYSCALL(SYS_AOS_FREE, aos_free)
#endif

#if (1)
#define SYS_AOS_GET_HZ 15
SYSCALL(SYS_AOS_GET_HZ, aos_get_hz)
#endif

#if (1)
#define SYS_AOS_IOCTL 16
SYSCALL(SYS_AOS_IOCTL, aos_ioctl)
#endif

#if (1)
#define SYS_AOS_KV_DEL 17
SYSCALL(SYS_AOS_KV_DEL, aos_kv_del)
#endif

#if (1)
#define SYS_AOS_KV_GET 18
SYSCALL(SYS_AOS_KV_GET, aos_kv_get)
#endif

#if (1)
#define SYS_AOS_KV_SET 19
SYSCALL(SYS_AOS_KV_SET, aos_kv_set)
#endif

#if (1)
#define SYS_AOS_LOOP_DESTROY 20
SYSCALL(SYS_AOS_LOOP_DESTROY, aos_loop_destroy)
#endif

#if (1)
#define SYS_AOS_LOOP_EXIT 21
SYSCALL(SYS_AOS_LOOP_EXIT, aos_loop_exit)
#endif

#if (1)
#define SYS_AOS_LOOP_INIT 22
SYSCALL(SYS_AOS_LOOP_INIT, aos_loop_init)
#endif

#if (1)
#define SYS_AOS_LOOP_RUN 23
SYSCALL(SYS_AOS_LOOP_RUN, aos_loop_run)
#endif

#if (1)
#define SYS_AOS_LOOP_SCHEDULE_CALL 24
SYSCALL(SYS_AOS_LOOP_SCHEDULE_CALL, aos_loop_schedule_call)
#endif

#if (1)
#define SYS_AOS_LOOP_SCHEDULE_WORK 25
SYSCALL(SYS_AOS_LOOP_SCHEDULE_WORK, aos_loop_schedule_work)
#endif

#if (1)
#define SYS_AOS_LSEEK 26
SYSCALL(SYS_AOS_LSEEK, aos_lseek)
#endif

#if (1)
#define SYS_AOS_MKDIR 27
SYSCALL(SYS_AOS_MKDIR, aos_mkdir)
#endif

#if (1)
#define SYS_AOS_MSLEEP 28
SYSCALL(SYS_AOS_MSLEEP, aos_msleep)
#endif

#if (1)
#define SYS_AOS_MUTEX_FREE 29
SYSCALL(SYS_AOS_MUTEX_FREE, aos_mutex_free)
#endif

#if (1)
#define SYS_AOS_MUTEX_LOCK 30
SYSCALL(SYS_AOS_MUTEX_LOCK, aos_mutex_lock)
#endif

#if (1)
#define SYS_AOS_MUTEX_NEW 31
SYSCALL(SYS_AOS_MUTEX_NEW, aos_mutex_new)
#endif

#if (1)
#define SYS_AOS_MUTEX_UNLOCK 32
SYSCALL(SYS_AOS_MUTEX_UNLOCK, aos_mutex_unlock)
#endif

#if (1)
#define SYS_AOS_NOW 33
SYSCALL(SYS_AOS_NOW, aos_now)
#endif

#if (1)
#define SYS_AOS_NOW_MS 34
SYSCALL(SYS_AOS_NOW_MS, aos_now_ms)
#endif

#if (1)
#define SYS_AOS_OPEN 35
SYSCALL(SYS_AOS_OPEN, aos_open)
#endif

#if (1)
#define SYS_AOS_OPENDIR 36
SYSCALL(SYS_AOS_OPENDIR, aos_opendir)
#endif

#if (1)
#define SYS_AOS_POLL 37
SYSCALL(SYS_AOS_POLL, aos_poll)
#endif

#if (1)
#define SYS_AOS_POLL_READ_FD 38
SYSCALL(SYS_AOS_POLL_READ_FD, aos_poll_read_fd)
#endif

#if (1)
#define SYS_AOS_POST_DELAYED_ACTION 39
SYSCALL(SYS_AOS_POST_DELAYED_ACTION, aos_post_delayed_action)
#endif

#if (1)
#define SYS_AOS_POST_EVENT 40
SYSCALL(SYS_AOS_POST_EVENT, aos_post_event)
#endif

#if (1)
#define SYS_AOS_QUEUE_FREE 41
SYSCALL(SYS_AOS_QUEUE_FREE, aos_queue_free)
#endif

#if (1)
#define SYS_AOS_QUEUE_NEW 42
SYSCALL(SYS_AOS_QUEUE_NEW, aos_queue_new)
#endif

#if (1)
#define SYS_AOS_QUEUE_RECV 43
SYSCALL(SYS_AOS_QUEUE_RECV, aos_queue_recv)
#endif

#if (1)
#define SYS_AOS_QUEUE_SEND 44
SYSCALL(SYS_AOS_QUEUE_SEND, aos_queue_send)
#endif

#if (1)
#define SYS_AOS_READ 45
SYSCALL(SYS_AOS_READ, aos_read)
#endif

#if (1)
#define SYS_AOS_READDIR 46
SYSCALL(SYS_AOS_READDIR, aos_readdir)
#endif

#if (1)
#define SYS_AOS_REBOOT 47
SYSCALL(SYS_AOS_REBOOT, aos_reboot)
#endif

#if (1)
#define SYS_AOS_REGISTER_EVENT_FILTER 48
SYSCALL(SYS_AOS_REGISTER_EVENT_FILTER, aos_register_event_filter)
#endif

#if (1)
#define SYS_AOS_RENAME 49
SYSCALL(SYS_AOS_RENAME, aos_rename)
#endif

#if (1)
#define SYS_AOS_SCHEDULE_CALL 50
SYSCALL(SYS_AOS_SCHEDULE_CALL, aos_schedule_call)
#endif

#if (1)
#define SYS_AOS_SEM_FREE 51
SYSCALL(SYS_AOS_SEM_FREE, aos_sem_free)
#endif

#if (1)
#define SYS_AOS_SEM_NEW 52
SYSCALL(SYS_AOS_SEM_NEW, aos_sem_new)
#endif

#if (1)
#define SYS_AOS_SEM_SIGNAL 53
SYSCALL(SYS_AOS_SEM_SIGNAL, aos_sem_signal)
#endif

#if (1)
#define SYS_AOS_SEM_WAIT 54
SYSCALL(SYS_AOS_SEM_WAIT, aos_sem_wait)
#endif

#if (1)
#define SYS_AOS_STAT 55
SYSCALL(SYS_AOS_STAT, aos_stat)
#endif

#if (1)
#define SYS_AOS_SYNC 56
SYSCALL(SYS_AOS_SYNC, aos_sync)
#endif

#if (1)
#define SYS_AOS_TASK_EXIT 57
SYSCALL(SYS_AOS_TASK_EXIT, aos_task_exit)
#endif

#if (1)
#define SYS_AOS_TASK_GETSPECIFIC 58
SYSCALL(SYS_AOS_TASK_GETSPECIFIC, aos_task_getspecific)
#endif

#if (1)
#define SYS_AOS_TASK_KEY_CREATE 59
SYSCALL(SYS_AOS_TASK_KEY_CREATE, aos_task_key_create)
#endif

#if (1)
#define SYS_AOS_TASK_KEY_DELETE 60
SYSCALL(SYS_AOS_TASK_KEY_DELETE, aos_task_key_delete)
#endif

#if (1)
#define SYS_AOS_TASK_NAME 61
SYSCALL(SYS_AOS_TASK_NAME, aos_task_name)
#endif

#if (1)
#define SYS_AOS_TASK_NEW 62
SYSCALL(SYS_AOS_TASK_NEW, aos_task_new)
#endif

#if (1)
#define SYS_AOS_TASK_NEW_EXT 63
SYSCALL(SYS_AOS_TASK_NEW_EXT, aos_task_new_ext)
#endif

#if (1)
#define SYS_AOS_TASK_SETSPECIFIC 64
SYSCALL(SYS_AOS_TASK_SETSPECIFIC, aos_task_setspecific)
#endif

#if (1)
#define SYS_AOS_TIMER_CHANGE 65
SYSCALL(SYS_AOS_TIMER_CHANGE, aos_timer_change)
#endif

#if (1)
#define SYS_AOS_TIMER_FREE 66
SYSCALL(SYS_AOS_TIMER_FREE, aos_timer_free)
#endif

#if (1)
#define SYS_AOS_TIMER_NEW 67
SYSCALL(SYS_AOS_TIMER_NEW, aos_timer_new)
#endif

#if (1)
#define SYS_AOS_TIMER_START 68
SYSCALL(SYS_AOS_TIMER_START, aos_timer_start)
#endif

#if (1)
#define SYS_AOS_TIMER_STOP 69
SYSCALL(SYS_AOS_TIMER_STOP, aos_timer_stop)
#endif

#if (1)
#define SYS_AOS_UART_SEND 70
SYSCALL(SYS_AOS_UART_SEND, aos_uart_send)
#endif

#if (1)
#define SYS_AOS_UNLINK 71
SYSCALL(SYS_AOS_UNLINK, aos_unlink)
#endif

#if (1)
#define SYS_AOS_UNREGISTER_EVENT_FILTER 72
SYSCALL(SYS_AOS_UNREGISTER_EVENT_FILTER, aos_unregister_event_filter)
#endif

#if (1)
#define SYS_AOS_VERSION_GET 73
SYSCALL(SYS_AOS_VERSION_GET, aos_version_get)
#endif

#if (1)
#define SYS_AOS_WRITE 74
SYSCALL(SYS_AOS_WRITE, aos_write)
#endif

#if (1)
#define SYS_GET_ERRNO 75
SYSCALL(SYS_GET_ERRNO, get_errno)
#endif

#if (1)
#define SYS_HAL_OTA_GET_DEFAULT_MODULE 76
SYSCALL(SYS_HAL_OTA_GET_DEFAULT_MODULE, hal_ota_get_default_module)
#endif

#if (1)
#define SYS_HAL_OTA_INIT 77
SYSCALL(SYS_HAL_OTA_INIT, hal_ota_init)
#endif

#if (1)
#define SYS_HAL_OTA_READ 78
SYSCALL(SYS_HAL_OTA_READ, hal_ota_read)
#endif

#if (1)
#define SYS_HAL_OTA_REGISTER_MODULE 79
SYSCALL(SYS_HAL_OTA_REGISTER_MODULE, hal_ota_register_module)
#endif

#if (1)
#define SYS_HAL_OTA_SET_BOOT 80
SYSCALL(SYS_HAL_OTA_SET_BOOT, hal_ota_set_boot)
#endif

#if (1)
#define SYS_HAL_OTA_WRITE 81
SYSCALL(SYS_HAL_OTA_WRITE, hal_ota_write)
#endif

#if (1)
#define SYS_HAL_WIFI_GET_DEFAULT_MODULE 82
SYSCALL(SYS_HAL_WIFI_GET_DEFAULT_MODULE, hal_wifi_get_default_module)
#endif

#if (1)
#define SYS_HAL_WIFI_GET_IP_STAT 83
SYSCALL(SYS_HAL_WIFI_GET_IP_STAT, hal_wifi_get_ip_stat)
#endif

#if (1)
#define SYS_HAL_WIFI_GET_LINK_STAT 84
SYSCALL(SYS_HAL_WIFI_GET_LINK_STAT, hal_wifi_get_link_stat)
#endif

#if (1)
#define SYS_HAL_WIFI_GET_MAC_ADDR 85
SYSCALL(SYS_HAL_WIFI_GET_MAC_ADDR, hal_wifi_get_mac_addr)
#endif

#if (1)
#define SYS_HAL_WIFI_INIT 86
SYSCALL(SYS_HAL_WIFI_INIT, hal_wifi_init)
#endif

#if (1)
#define SYS_HAL_WIFI_INSTALL_EVENT 87
SYSCALL(SYS_HAL_WIFI_INSTALL_EVENT, hal_wifi_install_event)
#endif

#if (1)
#define SYS_HAL_WIFI_POWER_OFF 88
SYSCALL(SYS_HAL_WIFI_POWER_OFF, hal_wifi_power_off)
#endif

#if (1)
#define SYS_HAL_WIFI_POWER_ON 89
SYSCALL(SYS_HAL_WIFI_POWER_ON, hal_wifi_power_on)
#endif

#if (1)
#define SYS_HAL_WIFI_REGISTER_MODULE 90
SYSCALL(SYS_HAL_WIFI_REGISTER_MODULE, hal_wifi_register_module)
#endif

#if (1)
#define SYS_HAL_WIFI_REGISTER_MONITOR_CB 91
SYSCALL(SYS_HAL_WIFI_REGISTER_MONITOR_CB, hal_wifi_register_monitor_cb)
#endif

#if (1)
#define SYS_HAL_WIFI_SET_CHANNEL 92
SYSCALL(SYS_HAL_WIFI_SET_CHANNEL, hal_wifi_set_channel)
#endif

#if (1)
#define SYS_HAL_WIFI_START 93
SYSCALL(SYS_HAL_WIFI_START, hal_wifi_start)
#endif

#if (1)
#define SYS_HAL_WIFI_START_ADV 94
SYSCALL(SYS_HAL_WIFI_START_ADV, hal_wifi_start_adv)
#endif

#if (1)
#define SYS_HAL_WIFI_START_SCAN 95
SYSCALL(SYS_HAL_WIFI_START_SCAN, hal_wifi_start_scan)
#endif

#if (1)
#define SYS_HAL_WIFI_START_SCAN_ADV 96
SYSCALL(SYS_HAL_WIFI_START_SCAN_ADV, hal_wifi_start_scan_adv)
#endif

#if (1)
#define SYS_HAL_WIFI_START_WIFI_MONITOR 97
SYSCALL(SYS_HAL_WIFI_START_WIFI_MONITOR, hal_wifi_start_wifi_monitor)
#endif

#if (1)
#define SYS_HAL_WIFI_STOP_WIFI_MONITOR 98
SYSCALL(SYS_HAL_WIFI_STOP_WIFI_MONITOR, hal_wifi_stop_wifi_monitor)
#endif

#if (1)
#define SYS_HAL_WIFI_SUSPEND 99
SYSCALL(SYS_HAL_WIFI_SUSPEND, hal_wifi_suspend)
#endif

#if (1)
#define SYS_HAL_WIFI_SUSPEND_SOFT_AP 100
SYSCALL(SYS_HAL_WIFI_SUSPEND_SOFT_AP, hal_wifi_suspend_soft_ap)
#endif

#if (1)
#define SYS_HAL_WIFI_SUSPEND_STATION 101
SYSCALL(SYS_HAL_WIFI_SUSPEND_STATION, hal_wifi_suspend_station)
#endif

#if (1)
#define SYS_HAL_WLAN_REGISTER_MGNT_MONITOR_CB 102
SYSCALL(SYS_HAL_WLAN_REGISTER_MGNT_MONITOR_CB, hal_wlan_register_mgnt_monitor_cb)
#endif

#if (1)
#define SYS_HAL_WLAN_SEND_80211_RAW_FRAME 103
SYSCALL(SYS_HAL_WLAN_SEND_80211_RAW_FRAME, hal_wlan_send_80211_raw_frame)
#endif

#if (1)
#define SYS_SET_ERRNO 104
SYSCALL(SYS_SET_ERRNO, set_errno)
#endif

#if (CONFIG_AOS_MESH > 0u)
#define SYS_UMESH_GET_DEVICE_STATE 105
SYSCALL(SYS_UMESH_GET_DEVICE_STATE, umesh_get_device_state)
#endif

#if (CONFIG_AOS_MESH > 0u)
#define SYS_UMESH_GET_MAC_ADDRESS 106
SYSCALL(SYS_UMESH_GET_MAC_ADDRESS, umesh_get_mac_address)
#endif

#if (CONFIG_AOS_MESH > 0u)
#define SYS_UMESH_GET_MCAST_ADDR 107
SYSCALL(SYS_UMESH_GET_MCAST_ADDR, umesh_get_mcast_addr)
#endif

#if (CONFIG_AOS_MESH > 0u)
#define SYS_UMESH_GET_MODE 108
SYSCALL(SYS_UMESH_GET_MODE, umesh_get_mode)
#endif

#if (CONFIG_AOS_MESH > 0u)
#define SYS_UMESH_GET_UCAST_ADDR 109
SYSCALL(SYS_UMESH_GET_UCAST_ADDR, umesh_get_ucast_addr)
#endif

#if (CONFIG_AOS_MESH > 0u)
#define SYS_UMESH_INIT 110
SYSCALL(SYS_UMESH_INIT, umesh_init)
#endif

#if (CONFIG_AOS_MESH > 0u)
#define SYS_UMESH_SET_MODE 111
SYSCALL(SYS_UMESH_SET_MODE, umesh_set_mode)
#endif

#if (CONFIG_AOS_MESH > 0u)
#define SYS_UMESH_START 112
SYSCALL(SYS_UMESH_START, umesh_start)
#endif

#if (CONFIG_AOS_MESH > 0u)
#define SYS_UMESH_STOP 113
SYSCALL(SYS_UMESH_STOP, umesh_stop)
#endif

#if (MBEDTLS_IN_KERNEL > 0u)
#define SYS_MBEDTLS_SSL_CLOSE 114
SYSCALL(SYS_MBEDTLS_SSL_CLOSE, mbedtls_ssl_close)
#endif

#if (MBEDTLS_IN_KERNEL > 0u)
#define SYS_MBEDTLS_SSL_CONNECT 115
SYSCALL(SYS_MBEDTLS_SSL_CONNECT, mbedtls_ssl_connect)
#endif

#if (MBEDTLS_IN_KERNEL > 0u)
#define SYS_MBEDTLS_SSL_RECV 116
SYSCALL(SYS_MBEDTLS_SSL_RECV, mbedtls_ssl_recv)
#endif

#if (MBEDTLS_IN_KERNEL > 0u)
#define SYS_MBEDTLS_SSL_SEND 117
SYSCALL(SYS_MBEDTLS_SSL_SEND, mbedtls_ssl_send)
#endif

#if (RHINO_CONFIG_MM_DEBUG <= 0u || RHINO_CONFIG_GCC_RETADDR <= 0u)
#define SYS_AOS_MALLOC 118
SYSCALL(SYS_AOS_MALLOC, aos_malloc)
#endif

#if (RHINO_CONFIG_MM_DEBUG <= 0u || RHINO_CONFIG_GCC_RETADDR <= 0u)
#define SYS_AOS_REALLOC 119
SYSCALL(SYS_AOS_REALLOC, aos_realloc)
#endif

#if (RHINO_CONFIG_MM_DEBUG <= 0u || RHINO_CONFIG_GCC_RETADDR <= 0u)
#define SYS_AOS_ZALLOC 120
SYSCALL(SYS_AOS_ZALLOC, aos_zalloc)
#endif

#if (RHINO_CONFIG_MM_DEBUG > 0u && RHINO_CONFIG_GCC_RETADDR > 0u)
#define SYS_ALI_AES_FINISH 121
SYSCALL(SYS_ALI_AES_FINISH, ali_aes_finish)
#endif

#if (RHINO_CONFIG_MM_DEBUG > 0u && RHINO_CONFIG_GCC_RETADDR > 0u)
#define SYS_ALI_AES_GET_CTX_SIZE 122
SYSCALL(SYS_ALI_AES_GET_CTX_SIZE, ali_aes_get_ctx_size)
#endif

#if (RHINO_CONFIG_MM_DEBUG > 0u && RHINO_CONFIG_GCC_RETADDR > 0u)
#define SYS_ALI_AES_INIT 123
SYSCALL(SYS_ALI_AES_INIT, ali_aes_init)
#endif

#if (RHINO_CONFIG_MM_DEBUG > 0u && RHINO_CONFIG_GCC_RETADDR > 0u)
#define SYS_SYS_AOS_MALLOC 124
SYSCALL(SYS_SYS_AOS_MALLOC, sys_aos_malloc)
#endif

#if (RHINO_CONFIG_MM_DEBUG > 0u && RHINO_CONFIG_GCC_RETADDR > 0u)
#define SYS_SYS_AOS_REALLOC 125
SYSCALL(SYS_SYS_AOS_REALLOC, sys_aos_realloc)
#endif

#if (RHINO_CONFIG_MM_DEBUG > 0u && RHINO_CONFIG_GCC_RETADDR > 0u)
#define SYS_SYS_AOS_ZALLOC 126
SYSCALL(SYS_SYS_AOS_ZALLOC, sys_aos_zalloc)
#endif

#if (RHINO_CONFIG_WORKQUEUE  > 0)
#define SYS_AOS_WORK_CANCEL 127
SYSCALL(SYS_AOS_WORK_CANCEL, aos_work_cancel)
#endif

#if (RHINO_CONFIG_WORKQUEUE  > 0)
#define SYS_AOS_WORK_DESTROY 128
SYSCALL(SYS_AOS_WORK_DESTROY, aos_work_destroy)
#endif

#if (RHINO_CONFIG_WORKQUEUE  > 0)
#define SYS_AOS_WORK_INIT 129
SYSCALL(SYS_AOS_WORK_INIT, aos_work_init)
#endif

#if (RHINO_CONFIG_WORKQUEUE  > 0)
#define SYS_AOS_WORK_RUN 130
SYSCALL(SYS_AOS_WORK_RUN, aos_work_run)
#endif

#if (RHINO_CONFIG_WORKQUEUE  > 0)
#define SYS_AOS_WORK_SCHED 131
SYSCALL(SYS_AOS_WORK_SCHED, aos_work_sched)
#endif

#if (RHINO_CONFIG_WORKQUEUE  > 0)
#define SYS_AOS_WORKQUEUE_CREATE 132
SYSCALL(SYS_AOS_WORKQUEUE_CREATE, aos_workqueue_create)
#endif

#if (RHINO_CONFIG_WORKQUEUE  > 0)
#define SYS_AOS_WORKQUEUE_DEL 133
SYSCALL(SYS_AOS_WORKQUEUE_DEL, aos_workqueue_del)
#endif

#if (WITH_LWIP > 0u)
#define SYS_IP4ADDR_ATON 134
SYSCALL(SYS_IP4ADDR_ATON, ip4addr_aton)
#endif

#if (WITH_LWIP > 0u)
#define SYS_IP4ADDR_NTOA 135
SYSCALL(SYS_IP4ADDR_NTOA, ip4addr_ntoa)
#endif

#if (WITH_LWIP > 0u)
#define SYS_IPADDR_ADDR 136
SYSCALL(SYS_IPADDR_ADDR, ipaddr_addr)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_ACCEPT 137
SYSCALL(SYS_LWIP_ACCEPT, lwip_accept)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_BIND 138
SYSCALL(SYS_LWIP_BIND, lwip_bind)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_CLOSE 139
SYSCALL(SYS_LWIP_CLOSE, lwip_close)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_CONNECT 140
SYSCALL(SYS_LWIP_CONNECT, lwip_connect)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_EVENTFD 141
SYSCALL(SYS_LWIP_EVENTFD, lwip_eventfd)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_FCNTL 142
SYSCALL(SYS_LWIP_FCNTL, lwip_fcntl)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_FREEADDRINFO 143
SYSCALL(SYS_LWIP_FREEADDRINFO, lwip_freeaddrinfo)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_GETADDRINFO 144
SYSCALL(SYS_LWIP_GETADDRINFO, lwip_getaddrinfo)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_GETHOSTBYNAME 145
SYSCALL(SYS_LWIP_GETHOSTBYNAME, lwip_gethostbyname)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_GETPEERNAME 146
SYSCALL(SYS_LWIP_GETPEERNAME, lwip_getpeername)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_GETSOCKNAME 147
SYSCALL(SYS_LWIP_GETSOCKNAME, lwip_getsockname)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_GETSOCKOPT 148
SYSCALL(SYS_LWIP_GETSOCKOPT, lwip_getsockopt)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_HTONL 149
SYSCALL(SYS_LWIP_HTONL, lwip_htonl)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_HTONS 150
SYSCALL(SYS_LWIP_HTONS, lwip_htons)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_IOCTL 151
SYSCALL(SYS_LWIP_IOCTL, lwip_ioctl)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_LISTEN 152
SYSCALL(SYS_LWIP_LISTEN, lwip_listen)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_READ 153
SYSCALL(SYS_LWIP_READ, lwip_read)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_RECV 154
SYSCALL(SYS_LWIP_RECV, lwip_recv)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_RECVFROM 155
SYSCALL(SYS_LWIP_RECVFROM, lwip_recvfrom)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_SELECT 156
SYSCALL(SYS_LWIP_SELECT, lwip_select)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_SEND 157
SYSCALL(SYS_LWIP_SEND, lwip_send)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_SENDMSG 158
SYSCALL(SYS_LWIP_SENDMSG, lwip_sendmsg)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_SENDTO 159
SYSCALL(SYS_LWIP_SENDTO, lwip_sendto)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_SETSOCKOPT 160
SYSCALL(SYS_LWIP_SETSOCKOPT, lwip_setsockopt)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_SHUTDOWN 161
SYSCALL(SYS_LWIP_SHUTDOWN, lwip_shutdown)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_SOCKET 162
SYSCALL(SYS_LWIP_SOCKET, lwip_socket)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_WRITE 163
SYSCALL(SYS_LWIP_WRITE, lwip_write)
#endif

#if (WITH_LWIP > 0u)
#define SYS_LWIP_WRITEV 164
SYSCALL(SYS_LWIP_WRITEV, lwip_writev)
#endif


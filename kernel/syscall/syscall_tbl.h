/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
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

/* --------------------Rhino-------------------- */

#define SYS_REBOOT 0
SYSCALL(SYS_REBOOT, yos_reboot)

#define SYS_GET_HZ 1
SYSCALL(SYS_GET_HZ, yos_get_hz)

#define SYS_VERSION_GET 2
SYSCALL(SYS_VERSION_GET, yos_version_get)

#define SYS_STRERROR 3
SYSCALL(SYS_STRERROR, yos_strerror)

#define SYS_TASK_NEW 4
SYSCALL(SYS_TASK_NEW, yos_task_new)

#define SYS_TASK_NEW_EXT 5
SYSCALL(SYS_TASK_NEW_EXT, yos_task_new_ext)

#define SYS_TASK_EXIT 6
SYSCALL(SYS_TASK_EXIT, yos_task_exit)

#define SYS_TASK_NAME 7
SYSCALL(SYS_TASK_NAME, yos_task_name)

#define SYS_TASK_KEY_CREATE 8
SYSCALL(SYS_TASK_KEY_CREATE, yos_task_key_create)

#define SYS_TASK_KEY_DELETE 9
SYSCALL(SYS_TASK_KEY_DELETE, yos_task_key_delete)

#define SYS_TASK_SETSPECIFIC 10
SYSCALL(SYS_TASK_SETSPECIFIC, yos_task_setspecific)

#define SYS_TASK_GETSPECIFIC 11
SYSCALL(SYS_TASK_GETSPECIFIC, yos_task_getspecific)

#define SYS_MUTEX_NEW 12
SYSCALL(SYS_MUTEX_NEW, yos_mutex_new)

#define SYS_MUTEX_FREE 13
SYSCALL(SYS_MUTEX_FREE, yos_mutex_free)

#define SYS_MUTEX_LOCK 14
SYSCALL(SYS_MUTEX_LOCK, yos_mutex_lock)

#define SYS_MUTEX_UNLOCK 15
SYSCALL(SYS_MUTEX_UNLOCK, yos_mutex_unlock)

#define SYS_SEM_NEW 16
SYSCALL(SYS_SEM_NEW, yos_sem_new)

#define SYS_SEM_FREE 17
SYSCALL(SYS_SEM_FREE, yos_sem_free)

#define SYS_SEM_WAIT 18
SYSCALL(SYS_SEM_WAIT, yos_sem_wait)

#define SYS_SEM_SIGNAL 19
SYSCALL(SYS_SEM_SIGNAL, yos_sem_signal)

#define SYS_QUEUE_NEW 20
SYSCALL(SYS_QUEUE_NEW, yos_queue_new)

#define SYS_QUEUE_FREE 21
SYSCALL(SYS_QUEUE_FREE, yos_queue_free)

#define SYS_QUEUE_SEND 22
SYSCALL(SYS_QUEUE_SEND, yos_queue_send)

#define SYS_QUEUE_RECV 23
SYSCALL(SYS_QUEUE_RECV, yos_queue_recv)

#define SYS_TIMER_NEW 24
SYSCALL(SYS_TIMER_NEW, yos_timer_new)

#define SYS_TIMER_FREE 25
SYSCALL(SYS_TIMER_FREE, yos_timer_free)

#define SYS_TIMER_START 26
SYSCALL(SYS_TIMER_START, yos_timer_start)

#define SYS_TIMER_STOP 27
SYSCALL(SYS_TIMER_STOP, yos_timer_stop)

#define SYS_TIMER_CHANGE 28
SYSCALL(SYS_TIMER_CHANGE, yos_timer_change)

#define SYS_WORKQUEUE_CREATE 29
SYSCALL(SYS_WORKQUEUE_CREATE, yos_workqueue_create)

#define SYS_WORKQUEUE_DEL 30
SYSCALL(SYS_WORKQUEUE_DEL, yos_workqueue_del)

#define SYS_WORK_INIT 31
SYSCALL(SYS_WORK_INIT, yos_work_init)

#define SYS_WORK_DESTROY 32
SYSCALL(SYS_WORK_DESTROY, yos_work_destroy)

#define SYS_WORK_RUN 33
SYSCALL(SYS_WORK_RUN, yos_work_run)

#define SYS_WORK_SCHED 34
SYSCALL(SYS_WORK_SCHED, yos_work_sched)

#define SYS_WORK_CANCEL 35
SYSCALL(SYS_WORK_CANCEL, yos_work_cancel)

#define SYS_MALLOC 36
SYSCALL(SYS_MALLOC, yos_malloc)

#define SYS_REALLOC 37
SYSCALL(SYS_REALLOC, yos_realloc)

#define SYS_FREE 38
SYSCALL(SYS_FREE, yos_free)

#define SYS_NOW 39
SYSCALL(SYS_NOW, yos_now)

#define SYS_NOW_MS 40
SYSCALL(SYS_NOW_MS, yos_now_ms)

#define SYS_MSLEEP 41
SYSCALL(SYS_MSLEEP, yos_msleep)

#define SYS_ZALLOC 42
SYSCALL(SYS_ZALLOC, yos_zalloc)


/* keep 42~50 for Rhino */

/* --------------------Framework-------------------- */

#define SYS_REGISTER_EVENT_FILTER 51
SYSCALL(SYS_REGISTER_EVENT_FILTER, yos_register_event_filter)

#define SYS_UNREGISTER_EVENT_FILTER 52
SYSCALL(SYS_UNREGISTER_EVENT_FILTER, yos_unregister_event_filter)

#define SYS_POST_EVENT 53
SYSCALL(SYS_POST_EVENT, yos_post_event)

#define SYS_POLL_READ_FD 54
SYSCALL(SYS_POLL_READ_FD, yos_poll_read_fd)

#define SYS_CANCEL_POLL_READ_FD 55
SYSCALL(SYS_CANCEL_POLL_READ_FD, yos_cancel_poll_read_fd)

#define SYS_POST_DELAYED_ACTION 56
SYSCALL(SYS_POST_DELAYED_ACTION, yos_post_delayed_action)

#define SYS_CANCEL_DELAYED_ACTION 57
SYSCALL(SYS_CANCEL_DELAYED_ACTION, yos_cancel_delayed_action)

#define SYS_SCHEDULE_CALL 58
SYSCALL(SYS_SCHEDULE_CALL, yos_schedule_call)

#define SYS_LOOP_INIT 59
SYSCALL(SYS_LOOP_INIT, yos_loop_init)

#define SYS_CURRENT_LOOP 60
SYSCALL(SYS_CURRENT_LOOP, yos_current_loop)

#define SYS_LOOP_RUN 61
SYSCALL(SYS_LOOP_RUN, yos_loop_run)

#define SYS_LOOP_EXIT 62
SYSCALL(SYS_LOOP_EXIT, yos_loop_exit)

#define SYS_LOOP_DESTROY 63
SYSCALL(SYS_LOOP_DESTROY, yos_loop_destroy)

#define SYS_LOOP_SCHEDULE_CALL 64
SYSCALL(SYS_LOOP_SCHEDULE_CALL, yos_loop_schedule_call)

#define SYS_LOOP_SCHEDULE_WORK 65
SYSCALL(SYS_LOOP_SCHEDULE_WORK, yos_loop_schedule_work)

#define SYS_CANCEL_WORK 66
SYSCALL(SYS_CANCEL_WORK, yos_cancel_work)

#define SYS_KV_SET 67
SYSCALL(SYS_KV_SET, yos_kv_set)

#define SYS_KV_GET 68
SYSCALL(SYS_KV_GET, yos_kv_get)

#define SYS_KV_DEL 69
SYSCALL(SYS_KV_DEL, yos_kv_del)

#define SYS_OPEN 70
SYSCALL(SYS_OPEN, yos_open)

#define SYS_CLOSE 71
SYSCALL(SYS_CLOSE, yos_close)

#define SYS_READ 72
SYSCALL(SYS_READ, yos_read)

#define SYS_WRITE 73
SYSCALL(SYS_WRITE, yos_write)

#define SYS_IOCTL 74
SYSCALL(SYS_IOCTL, yos_ioctl)

#define SYS_POLL 75
SYSCALL(SYS_POLL, yos_poll)

#define SYS_FCNTL 76
SYSCALL(SYS_FCNTL, yos_fcntl)

/* keep 77~100 for Framework */

/* --------------------LWIP-------------------- */

#ifdef WITH_LWIP

#define SYS_LWIP_ACCEPT 101
SYSCALL(SYS_LWIP_ACCEPT, lwip_accept)

#define SYS_LWIP_BIND 102
SYSCALL(SYS_LWIP_BIND, lwip_bind)

#define SYS_LWIP_SHUTDOWN 103
SYSCALL(SYS_LWIP_SHUTDOWN, lwip_shutdown)

#define SYS_LWIP_GETPEERNAME 104
SYSCALL(SYS_LWIP_GETPEERNAME, lwip_getpeername)

#define SYS_LWIP_GETSOCKNAME 105
SYSCALL(SYS_LWIP_GETSOCKNAME, lwip_getsockname)

#define SYS_LWIP_GETSOCKOPT 106
SYSCALL(SYS_LWIP_GETSOCKOPT, lwip_getsockopt)

#define SYS_LWIP_SETSOCKOPT 107
SYSCALL(SYS_LWIP_SETSOCKOPT, lwip_setsockopt)

#define SYS_LWIP_CLOSE 108
SYSCALL(SYS_LWIP_CLOSE, lwip_close)

#define SYS_LWIP_CONNECT 109
SYSCALL(SYS_LWIP_CONNECT, lwip_connect)

#define SYS_LWIP_LISTEN 110
SYSCALL(SYS_LWIP_LISTEN, lwip_listen)

#define SYS_LWIP_RECV 111
SYSCALL(SYS_LWIP_RECV, lwip_recv)

#define SYS_LWIP_READ 112
SYSCALL(SYS_LWIP_READ, lwip_read)

#define SYS_LWIP_RECVFROM 113
SYSCALL(SYS_LWIP_RECVFROM, lwip_recvfrom)

#define SYS_LWIP_SEND 114
SYSCALL(SYS_LWIP_SEND, lwip_send)

#define SYS_LWIP_SENDMSG 115
SYSCALL(SYS_LWIP_SENDMSG, lwip_sendmsg)

#define SYS_LWIP_SENDTO 116
SYSCALL(SYS_LWIP_SENDTO, lwip_sendto)

#define SYS_LWIP_SOCKET 117
SYSCALL(SYS_LWIP_SOCKET, lwip_socket)

#define SYS_LWIP_WRITE 118
SYSCALL(SYS_LWIP_WRITE, lwip_write)

#define SYS_LWIP_WRITEV 119
SYSCALL(SYS_LWIP_WRITEV, lwip_writev)

#define SYS_LWIP_SELECT 120
SYSCALL(SYS_LWIP_SELECT, lwip_select)

#define SYS_LWIP_IOCTL 121
SYSCALL(SYS_LWIP_IOCTL, lwip_ioctl)

#define SYS_LWIP_FCNTL 122
SYSCALL(SYS_LWIP_FCNTL, lwip_fcntl)

#define SYS_LWIP_EVENTFD 123
SYSCALL(SYS_LWIP_EVENTFD, lwip_eventfd)

#define SYS_LWIP_HTONL 124
SYSCALL(SYS_LWIP_HTONL, lwip_htonl)

#define SYS_LWIP_HTONS 125
SYSCALL(SYS_LWIP_HTONS, lwip_htons)

#define SYS_LWIP_GETHOSTBYNAME 126
SYSCALL(SYS_LWIP_GETHOSTBYNAME, lwip_gethostbyname)

#define SYS_LWIP_IP4ADDR_ATON 127
SYSCALL(SYS_LWIP_IP4ADDR_ATON, ip4addr_aton)

#define SYS_LWIP_IP4ADDR_NTOA 128
SYSCALL(SYS_LWIP_IP4ADDR_NTOA, ip4addr_ntoa)

#define SYS_LWIP_IPADDR_ADDR 129
SYSCALL(SYS_LWIP_IPADDR_ADDR, ipaddr_addr)

/* keep 130 for LWIP */

#endif /* WITH_LWIP */

/* --------------------WIFI-------------------- */

#define SYS_WIFI_GET_DEFAULT_MODULE 131
SYSCALL(SYS_WIFI_GET_DEFAULT_MODULE, hal_wifi_get_default_module)

#define SYS_WIFI_REGISTER_MODULE 132
SYSCALL(SYS_WIFI_REGISTER_MODULE, hal_wifi_register_module)

#define SYS_WIFI_INIT 133
SYSCALL(SYS_WIFI_INIT, hal_wifi_init)

#define SYS_WIFI_GET_MAC_ADDR 134
SYSCALL(SYS_WIFI_GET_MAC_ADDR, hal_wifi_get_mac_addr)

#define SYS_WIFI_START 135
SYSCALL(SYS_WIFI_START, hal_wifi_start)

#define SYS_WIFI_START_ADV 136
SYSCALL(SYS_WIFI_START_ADV, hal_wifi_start_adv)

#define SYS_WIFI_GET_IP_STAT 137
SYSCALL(SYS_WIFI_GET_IP_STAT, hal_wifi_get_ip_stat)

#define SYS_WIFI_GET_LINK_STAT 138
SYSCALL(SYS_WIFI_GET_LINK_STAT, hal_wifi_get_link_stat)

#define SYS_WIFI_START_SCAN 139
SYSCALL(SYS_WIFI_START_SCAN, hal_wifi_start_scan)

#define SYS_WIFI_START_SCAN_ADV 140
SYSCALL(SYS_WIFI_START_SCAN_ADV, hal_wifi_start_scan_adv)

#define SYS_WIFI_POWER_OFF 141
SYSCALL(SYS_WIFI_POWER_OFF, hal_wifi_power_off)

#define SYS_WIFI_POWER_ON 142
SYSCALL(SYS_WIFI_POWER_ON, hal_wifi_power_on)

#define SYS_WIFI_SUSPEND 143
SYSCALL(SYS_WIFI_SUSPEND, hal_wifi_suspend)

#define SYS_WIFI_SUSPEND_STATION 144
SYSCALL(SYS_WIFI_SUSPEND_STATION, hal_wifi_suspend_station)

#define SYS_WIFI_SUSPEND_SOFT_AP 145
SYSCALL(SYS_WIFI_SUSPEND_SOFT_AP, hal_wifi_suspend_soft_ap)

#define SYS_WIFI_SET_CHANNEL 146
SYSCALL(SYS_WIFI_SET_CHANNEL, hal_wifi_set_channel)

#define SYS_WIFI_START_WIFI_MONITOR 147
SYSCALL(SYS_WIFI_START_WIFI_MONITOR, hal_wifi_start_wifi_monitor)

#define SYS_WIFI_STOP_WIFI_MONITOR 148
SYSCALL(SYS_WIFI_STOP_WIFI_MONITOR, hal_wifi_stop_wifi_monitor)

#define SYS_WIFI_REGISTER_MONITOR_CB 149
SYSCALL(SYS_WIFI_REGISTER_MONITOR_CB, hal_wifi_register_monitor_cb)

#define SYS_WIFI_INSTALL_EVENT 150
SYSCALL(SYS_WIFI_INSTALL_EVENT, hal_wifi_install_event)

#define SYS_UR_MESH_INIT 151
SYSCALL(SYS_UR_MESH_INIT, umesh_init)

#define SYS_UR_MESH_START 152
SYSCALL(SYS_UR_MESH_START, umesh_start)

#define SYS_UR_MESH_STOP 153
SYSCALL(SYS_UR_MESH_STOP, umesh_stop)

#define SYS_UR_MESH_SET_MODE 154
SYSCALL(SYS_UR_MESH_SET_MODE, umesh_set_mode)

#define SYS_UR_MESH_GET_MODE 155
SYSCALL(SYS_UR_MESH_GET_MODE, umesh_get_mode)

#define SYS_HAL_WLAN_REG_MGNT_MONITOR_CB 156
SYSCALL(SYS_HAL_WLAN_REG_MGNT_MONITOR_CB, hal_wlan_register_mgnt_monitor_cb)

#define SYS_HAL_WLAN_SEND_80211_RAW_FRAME 157
SYSCALL(SYS_HAL_WLAN_SEND_80211_RAW_FRAME, hal_wlan_send_80211_raw_frame)

#define SYS_UR_MESH_GET_MCAST_ADDR 158
SYSCALL(SYS_UR_MESH_GET_MCAST_ADDR, umesh_get_mcast_addr)

#define SYS_UR_MESH_GET_UCAST_ADDR 159
SYSCALL(SYS_UR_MESH_GET_UCAST_ADDR, umesh_get_ucast_addr)

#define SYS_UR_MESH_NET_GET_MAC_ADDRESS 160
SYSCALL(SYS_UR_MESH_NET_GET_MAC_ADDRESS, umesh_net_get_mac_address)

#define SYS_UR_MESH_GET_DEVICE_STATE 161
SYSCALL(SYS_UR_MESH_GET_DEVICE_STATE, umesh_get_device_state)


/* --------------------OTA-------------------- */

#define SYS_OTA_REGISTER_MODULE 171
SYSCALL(SYS_OTA_REGISTER_MODULE, hal_ota_register_module)

#define SYS_OTA_INIT 172
SYSCALL(SYS_OTA_INIT, hal_ota_init)

#define SYS_OTA_WRITE 173
SYSCALL(SYS_OTA_WRITE, hal_ota_write)

#define SYS_OTA_READ 174
SYSCALL(SYS_OTA_READ, hal_ota_read)

#define SYS_OTA_SET_BOOT 175
SYSCALL(SYS_OTA_SET_BOOT, hal_ota_set_boot)

#define SYS_OTA_GET_DEFAULT_MODULE 176
SYSCALL(SYS_OTA_GET_DEFAULT_MODULE, hal_ota_get_default_module)

/* keep 167~170 for OTA */

/* --------------------OTHERS-------------------- */
#define SYS_UART_SEND 181
SYSCALL(SYS_UART_SEND, yos_uart_send)

#define SYS_CLI_REGISTER_COMMAND 182
SYSCALL(SYS_CLI_REGISTER_COMMAND, cli_register_command)

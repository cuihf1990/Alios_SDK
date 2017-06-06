/**
******************************************************************************
* @file    mico_rtos.h
* @author  William Xu
* @version V1.0.0
* @date    16-Sep-2014
* @brief   This file provides all the headers of RTOS operation provided by MICO.
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

#if CFG_MXCHIP
void mico_rtos_enter_critical( void )
{
    rtos_enter_critical();
}

void mico_rtos_exit_critical( void )
{
    rtos_exit_critical();
}

OSStatus mico_rtos_create_thread( mico_thread_t* thread, uint8_t priority, const char* name, mico_thread_function_t function, uint32_t stack_size, mico_thread_arg_t arg )
{
    return rtos_create_thread(thread,priority,name,function,stack_size,arg);
}

OSStatus mico_rtos_delete_thread( mico_thread_t* thread )
{
    return rtos_delete_thread(thread);
}

OSStatus mico_rtos_create_worker_thread( mico_worker_thread_t* worker_thread, uint8_t priority, uint32_t stack_size, uint32_t event_queue_size )
{
    return mico_rtos_create_worker_thread(worker_thread, priority, stack_size, event_queue_size );
}

OSStatus mico_rtos_delete_worker_thread( mico_worker_thread_t* worker_thread )
{
    return rtos_delete_worker_thread(worker_thread);
}

void mico_rtos_suspend_thread(mico_thread_t* thread)
{
    rtos_suspend_thread(thread);
}

void mico_rtos_suspend_all_thread(void)
{
    rtos_suspend_all_thread();
}

long mico_rtos_resume_all_thread(void)
{
    return rtos_resume_all_thread();
}

OSStatus mico_rtos_thread_join( mico_thread_t* thread )
{
    return rtos_thread_join(thread);
}

OSStatus mico_rtos_thread_force_awake( mico_thread_t* thread )
{
    return rtos_thread_force_awake(thread);
}

bool mico_rtos_is_current_thread( mico_thread_t* thread )
{
    return rtos_is_current_thread(thread);
}

mico_thread_t* mico_rtos_get_current_thread( void )
{
    return rtos_get_current_thread();
}

void mico_rtos_thread_sleep(uint32_t seconds)
{
    rtos_thread_sleep(seconds);
}

void mico_rtos_thread_msleep(uint32_t milliseconds)
{
    return rtos_thread_msleep(milliseconds);
}

OSStatus mico_rtos_delay_milliseconds( uint32_t num_ms )
{
    return rtos_delay_milliseconds(num_ms);
}

OSStatus mico_rtos_print_thread_status( char* buffer, int length )
{
    return rtos_print_thread_status(buffer,length);
}

OSStatus mico_rtos_init_semaphore( mico_semaphore_t* semaphore, int maxCount )
{
    return rtos_init_semaphore(semaphore,maxCount);
}

OSStatus mico_rtos_set_semaphore( mico_semaphore_t* semaphore )
{
    return rtos_set_semaphore(semaphore);
}

OSStatus mico_rtos_get_semaphore( mico_semaphore_t* semaphore, uint32_t timeout_ms )
{
    return rtos_get_semaphore(semaphore,timeout_ms);
}

OSStatus mico_rtos_deinit_semaphore( mico_semaphore_t* semaphore )
{
    return rtos_deinit_semaphore(semaphore);
}

OSStatus mico_rtos_init_mutex( mico_mutex_t* mutex )
{
    return rtos_init_mutex(mutex);
}

OSStatus mico_rtos_lock_mutex( mico_mutex_t* mutex )
{
    return rtos_lock_mutex(mutex);
}

OSStatus mico_rtos_unlock_mutex( mico_mutex_t* mutex )
{
    return rtos_unlock_mutex(mutex);
}

OSStatus mico_rtos_deinit_mutex( mico_mutex_t* mutex )
{
    return rtos_deinit_mutex(mutex);
}

OSStatus mico_rtos_init_queue( mico_queue_t* queue, const char* name, uint32_t message_size, uint32_t number_of_messages )
{
    return rtos_init_queue(queue,name,message_size,number_of_messages);
}

OSStatus mico_rtos_push_to_queue( mico_queue_t* queue, void* message, uint32_t timeout_ms )
{
    return rtos_push_to_queue(queue,message,timeout_ms);
}

OSStatus mico_rtos_pop_from_queue( mico_queue_t* queue, void* message, uint32_t timeout_ms )
{
    return rtos_pop_from_queue(queue,message,timeout_ms);
}

OSStatus mico_rtos_deinit_queue( mico_queue_t* queue )
{
    return rtos_deinit_queue(queue);
}

bool mico_rtos_is_queue_empty( mico_queue_t* queue )
{
    return rtos_is_queue_empty(queue);
}

bool mico_rtos_is_queue_full( mico_queue_t* queue )
{
    return rtos_is_queue_full(queue);
}

OSStatus mico_rtos_send_asynchronous_event( mico_worker_thread_t* worker_thread, event_handler_t function, void* arg )
{
    return rtos_send_asynchronous_event(worker_thread,function,arg);
}

OSStatus mico_rtos_register_timed_event( mico_timed_event_t* event_object, mico_worker_thread_t* worker_thread, event_handler_t function, uint32_t time_ms, void* arg )
{
    return rtos_register_timed_event(event_object,worker_thread,function,time_ms,arg);
}

OSStatus mico_rtos_deregister_timed_event( mico_timed_event_t* event_object )
{
    return rtos_deregister_timed_event(event_object);
}

uint32_t mico_rtos_get_time(void)
{
    return rtos_get_time();
}

OSStatus mico_rtos_init_timer( mico_timer_t* timer, uint32_t time_ms, timer_handler_t function, void* arg )
{
    return rtos_init_timer(timer,time_ms,function,arg);
}

OSStatus mico_rtos_start_timer( mico_timer_t* timer )
{
    return rtos_start_timer(timer);
}

OSStatus mico_rtos_stop_timer( mico_timer_t* timer )
{
    return rtos_stop_timer(timer);
}

OSStatus mico_rtos_reload_timer( mico_timer_t* timer )
{
    return rtos_reload_timer(timer);
}

OSStatus mico_rtos_deinit_timer( mico_timer_t* timer )
{
    return rtos_deinit_timer(timer);
}

bool mico_rtos_is_timer_running( mico_timer_t* timer )
{
    return rtos_is_timer_running(timer);
}

int mico_rtos_init_event_fd(mico_event_t event_handle)
{
    return rtos_init_event_fd(event_handle);
}

int mico_rtos_deinit_event_fd(int fd)
{
    return rtos_deinit_event_fd(fd);
}
#endif
// eof


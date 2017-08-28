#ifndef TRACE_HAL_H
#define TRACE_HAl_H

void *trace_hal_init(void);
ssize_t trace_hal_send(void *handle, void *buf, size_t len);
ssize_t trace_hal_recv(void *handle, void *buf);
void trace_hal_deinit(void *handle);

#endif


#ifndef ACCS_H
#define ACCS_H

#include "service.h"

void start_accs_work();
void stop_accs_work();
int accs_prepare();
int accs_start();
int accs_stop();
int accs_put(void*, int);
int accs_get(void*, int, void*, int);
int accs_add_listener(service_cb);
int accs_del_listener(service_cb);
int cloud_is_connected(void);
#endif

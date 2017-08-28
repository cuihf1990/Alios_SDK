#ifndef YLOOP_H
#define YLOOP_H

/* set loop's event fd */
void yos_loop_set_eventfd(int fd);

/* get loop's event fd */
int yos_loop_get_eventfd(void *loop);

/* init per-loop event service */
int yos_event_service_init(void);

/* deinit per-loop event service */
void yos_event_service_deinit(int fd);

#endif /* YLOOP_H */


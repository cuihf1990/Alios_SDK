#include "wifi.h"
#include <stdio.h>

#define yos_offsetof(type, member) ({ \
    type tmp;                         \
    (long)(&tmp.member) - (long)&tmp; \
})

#define YOS_DLIST_HEAD_INIT(name) { &(name), &(name) }
#define YOS_DLIST_HEAD(name) \
        dlist_t name = YOS_DLIST_HEAD_INIT(name)



static inline void __dlist_add(dlist_t *node, dlist_t *prev, dlist_t *next)
{
    node->next = next;
    node->prev = prev;

    prev->next = node;
    next->prev = node;
}

#define dlist_entry(addr, type, member) ({         \
    (type *)((long)addr - yos_offsetof(type, member)); \
})

#define dlist_first_entry(ptr, type, member) \
    dlist_entry((ptr)->next, type, member)

#define dlist_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

static inline void dlist_add(dlist_t *node, dlist_t *queue)
{
    __dlist_add(node, queue, queue->next);
}

static inline void dlist_add_tail(dlist_t *node, dlist_t *queue)
{
    __dlist_add(node, queue->prev, queue);
}

static inline void dlist_del(dlist_t *node)
{
    dlist_t *prev = node->prev;
    dlist_t *next = node->next;

    prev->next = next;
    next->prev = prev;
}

static inline void dlist_init(dlist_t *node)
{
    node->next = node->prev = node;
}

static inline int dlist_empty(const dlist_t *head)
{
      return head->next == head;
}


static YOS_DLIST_HEAD(g_wifi_module);


hal_wifi_module_t *hal_wifi_get_default_module(void)
{
    hal_wifi_module_t *m = 0;

    if (dlist_empty(&g_wifi_module)) {
        return 0;
    }

    m = dlist_first_entry(&g_wifi_module, hal_wifi_module_t, base.list);

    return m;
}


void hal_wifi_register_module(hal_wifi_module_t *module)
{
    dlist_add_tail(&module->base.list, &g_wifi_module);

}


int hal_wifi_init(void)
{
    int          err = 0;
    dlist_t *t;

    /* do low level init */
    dlist_for_each(t, &g_wifi_module) {
        hal_wifi_module_t *m = (hal_wifi_module_t*)t;
        m->init(m);
    }

    return err;
}


void hal_wifi_get_mac_addr(hal_wifi_module_t *m, uint8_t *mac)
{
    if (m == NULL) {
        m = hal_wifi_get_default_module();
    }

    m->get_mac_addr(mac);
}


int hal_wifi_start(hal_wifi_module_t *m, hal_wifi_init_type_t *init_para)
{
    if (m == NULL) {
        m = hal_wifi_get_default_module();
    }

    m->start(init_para);
}


int  hal_wifi_start_adv(hal_wifi_module_t *m, hal_wifi_init_type_adv_t *init_para_adv)
{
    if (m == NULL) {
        m = hal_wifi_get_default_module();
    }

    m->start_adv(init_para_adv);
}

int  hal_wifi_get_ip_stat(hal_wifi_module_t *m, hal_wifi_ip_stat_t *out_net_para, hal_wifi_type_t wifi_type)
{
    if (m == NULL) {
        m = hal_wifi_get_default_module();
    }

    m->get_ip_stat(out_net_para, wifi_type);
}

int  hal_wifi_get_link_stat(hal_wifi_module_t *m, hal_wifi_link_stat_t *out_stat)
{
    if (m == NULL) {
        m = hal_wifi_get_default_module();
    }

    m->get_link_stat(out_stat);
}

void hal_wifi_start_scan(hal_wifi_module_t *m)
{
    if (m == NULL) {
        m = hal_wifi_get_default_module();
    }

    m->start_scan();
}

void hal_wifi_start_scan_adv(hal_wifi_module_t *m)
{
    if (m == NULL) {
        m = hal_wifi_get_default_module();
    }

    m->start_scan_adv();
}

int hal_wifi_power_off(hal_wifi_module_t *m)
{
    if (m == NULL) {
        m = hal_wifi_get_default_module();
    }

    m->power_off();
}

int hal_wifi_power_on(hal_wifi_module_t *m)
{
    if (m == NULL) {
        m = hal_wifi_get_default_module();
    }

    m->power_on();
}

int hal_wifi_suspend(hal_wifi_module_t *m)
{
    if (m == NULL) {
        m = hal_wifi_get_default_module();
    }

    m->suspend();

}

int  hal_wifi_suspend_station(hal_wifi_module_t *m)
{
    if (m == NULL) {
        m = hal_wifi_get_default_module();
    }

    m->suspend_station();
}

int  hal_wifi_suspend_soft_ap(hal_wifi_module_t *m)
{
    if (m == NULL) {
        m = hal_wifi_get_default_module();
    }

    m->suspend_soft_ap();
}


void hal_wifi_install_event(hal_wifi_module_t *m, hal_wifi_event_cb_t *cb)
{
    m->ev_cb = cb;
}


static int hce_wifi_init(hal_wifi_module_t *m)
{
    printf("wifi init\n");
    return 0;
};


hal_wifi_module_t esp32_yos_wifi_module = {
    .base.name          = "yos_wifi_module_esp32",
    .init               = hce_wifi_init,
};


int main(void)
{
    hal_wifi_register_module(&esp32_yos_wifi_module);
    hal_wifi_init();
    return 0;

}

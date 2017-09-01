/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef K_EVENT_H
#define K_EVENT_H

typedef struct {
    blk_obj_t blk_obj;
    uint32_t  flags;

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    klist_t   event_item;
#endif

    uint8_t   mm_alloc_flag;
} kevent_t;

#define YUNOS_FLAGS_AND_MASK   0x2u
#define YUNOS_FLAGS_CLEAR_MASK 0x1u

#define YUNOS_AND              0x02u
#define YUNOS_AND_CLEAR        0x03u
#define YUNOS_OR               0x00u
#define YUNOS_OR_CLEAR         0x01u

/**
 * This function will create a event
 * @param[in]  event  pointer to the event
 * @param[in]  name   name of the event
 * @param[in]  flags  flags to be init
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_event_create(kevent_t *event, const name_t *name, uint32_t flags);

/**
 * This function will delete a event
 * @param[in]  event  pointer to a event
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_event_del(kevent_t *event);

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
/**
 * This function will create a dyn-event
 * @param[out]  event  pointer to the event
 * @param[in]   name   name of the semaphore
 * @param[in]   flags  flags to be init
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_event_dyn_create(kevent_t **event, const name_t *name,
                               uint32_t flags);

/**
 * This function will delete a dyn created event
 * @param[in]  event  pointer to a event
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_event_dyn_del(kevent_t *event);
#endif

/**
 * This function will get event
 * @param[in]   event       pointer to the event
 * @param[in]   flags       which is provided by users
 * @param[in]   opt         could be YUNOS_AND, YUNOS_AND_CLEAR, YUNOS_OR, YUNOS_OR_CLEAR
 * @param[out]  actl_flags  the actually flag where flags is satisfied
 * @param[in]   ticks       ticks to wait
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_event_get(kevent_t *event, uint32_t flags, uint8_t opt,
                        uint32_t *actl_flags, tick_t ticks);

/**
 * This function will set a event
 * @param[in]  event  pointer to a event
 * @param[in]  flags  which users want to be set
 * @param[in]  opt    could be YUNOS_AND, YUNOS_OR
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_event_set(kevent_t *event, uint32_t flags, uint8_t opt);

#endif /* K_EVENT_H */


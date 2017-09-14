/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef AOS_LIST_H
#define AOS_LIST_H

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * get offset of a member variable
 *
 * @param[in]   type     the type of the struct this is embedded in.
 * @param[in]   member   the name of the list_struct within the struct.
 */
#define aos_offsetof(type, member) ({ \
    type tmp;                         \
    (long)(&tmp.member) - (long)&tmp; \
})

/**
 * get the struct for this entry
 * @param[in]   ptr     the &struct list_head pointer.
 * @param[in]   type    the type of the struct this is embedded in.
 * @param[in]   member   the name of the list_struct within the struct.
 */
#define aos_container_of(ptr, type, member) \
    ((type *) ((char *) (ptr) - aos_offsetof(type, member)))

/* for double link list */
typedef struct dlist_s {
    struct dlist_s *prev;
    struct dlist_s *next;
} dlist_t;

static inline void __dlist_add(dlist_t *node, dlist_t *prev, dlist_t *next)
{
    node->next = next;
    node->prev = prev;

    prev->next = node;
    next->prev = node;
}

/**
 * get the struct
 *
 * @param[in]   addr    list address
 * @param[in]   type    the type of the struct this is embedded in.
 * @param[in]   member  the name of the list_struct within the struct.
 */
#define dlist_entry(addr, type, member) ({         \
    (type *)((long)addr - aos_offsetof(type, member)); \
})

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

static inline void INIT_AOS_DLIST_HEAD(dlist_t *list)
{
    list->next = list;
    list->prev = list;
}

static inline int dlist_empty(const dlist_t *head)
{
    return head->next == head;
}

/**
 * init zhe list
 *
 * @param[in]   list    list to be inited
 */
#define AOS_DLIST_INIT(list)  {&(list), &(list)}

/**
 * get the fist struct in the list
 *
 * @param[in]   ptr     the &struct list_head pointer.
 * @param[in]   type    the type of the struct this is embedded in.
 * @param[in]   member  the name of the list_struct within the struct.
 */
#define dlist_first_entry(ptr, type, member) \
    dlist_entry((ptr)->next, type, member)

/**
 * iterate over list of given type
 *
 * @param[in]   pos     the type * to use as a loop cursor
 * @param[in]   head    he head for your list
 */
#define dlist_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * iterate over list of given type safety
 *
 * @param[in]   pos     the type * to use as a loop cursor
 * @param[in]   n       the type * to use as a loop temp
 * @param[in]   head    he head for your list
 */
#define dlist_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
    pos = n, n = pos->next)

/**
 * iterate over list of given type
 *
 * @param[in]   queue   he head for your list
 * @param[in]   node    the type * to use as a loop cursor
 * @param[in]   type    the type of the struct this is embedded in
 * @param[in]   member  the name of the list_struct within the struct
 */
#define dlist_for_each_entry(queue, node, type, member) \
    for (node = aos_container_of((queue)->next, type, member); \
         &node->member != (queue); \
         node = aos_container_of(node->member.next, type, member))

/**
 * iterate over list of given type safey
 *
 * @param[in]   queue   he head for your list
 * @param[in]   node    the type * to use as a loop cursor
 * @param[in]   n       the type * to use as a temp.
 * @param[in]   type    the type of the struct this is embedded in
 * @param[in]   member  the name of the list_struct within the struct
 */
#define dlist_for_each_entry_safe(queue, n, node, type, member) \
    for (node = aos_container_of((queue)->next, type, member), \
         n = (queue)->next ? (queue)->next->next : NULL; \
         &node->member != (queue); \
         node = aos_container_of(n, type, member), n = n ? n->next : NULL)

/**
 * get the struct for this entry
 * @param[in]   ptr      the &struct list_head pointer.
 * @param[in]   type     the type of the struct this is embedded in.
 * @param[in]   member   the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
        aos_container_of(ptr, type, member)


/**
 * iterate backwards over list of given type
 *
 * @param[in]   pos     the type * to use as a loop cursor.
 * @param[in]   head    he head for your list
 * @param[in]   member  the name of the list_struct within the struct
 * @param[in]   type    the type of the struct this is embedded in.
 */
#define dlist_for_each_entry_reverse(pos, head, member, type) \
    for (pos = list_entry((head)->prev, type, member);  \
         &pos->member != (head);    \
         pos = list_entry(pos->member.prev, type, member))


/**
 * get zhe list length
 *
 * @param[in]   queue    he head for your list
 */
#define dlist_entry_number(queue) ({ \
    int num; \
    dlist_t *cur = queue; \
    for (num=0;cur->next != queue;cur=cur->next, num++); \
    num; \
})

#define AOS_DLIST_HEAD_INIT(name) { &(name), &(name) }
#define AOS_DLIST_HEAD(name) \
        dlist_t name = AOS_DLIST_HEAD_INIT(name)

/* for single link list */
typedef struct slist_s {
    struct slist_s *next;
} slist_t;

static inline void slist_add(slist_t *node, slist_t *head)
{
    node->next = head->next;
    head->next = node;
}

static inline void slist_add_tail(slist_t *node, slist_t *head)
{
    while (head->next) {
        head = head->next;
    }

    slist_add(node, head);
}

static inline void slist_del(slist_t *node, slist_t *head)
{
    while (head->next) {
        if (head->next == node) {
            head->next = node->next;
            break;
        }

        head = head->next;
    }
}

static inline int slist_empty(const slist_t *head)
{
    return !head->next;
}

static inline void slist_init(slist_t *head)
{
    head->next = 0;
}

/**
 * iterate over list of given type
 *
 * @param[in]   queue   he head for your list
 * @param[in]   node    the type * to use as a loop cursor
 * @param[in]   type    the type of the struct this is embedded in
 * @param[in]   member  the name of the list_struct within the struct
 */
#define slist_for_each_entry(queue, node, type, member)    \
    for (node = aos_container_of((queue)->next, type, member); \
         &node->member;                                    \
         node = aos_container_of(node->member.next, type, member))

/**
 * iterate over list of given type safey
 *
 * @param[in]   queue   he head for your list
 * @param[in]   node    the type * to use as a loop cursor
 * @param[in]   n       the type * to use as a temp.
 * @param[in]   type    the type of the struct this is embedded in
 * @param[in]   member  the name of the list_struct within the struct
 */
#define slist_for_each_entry_safe(queue, tmp, node, type, member)    \
    for (node = aos_container_of((queue)->next, type, member), \
         tmp = (queue)->next ? (queue)->next->next : NULL; \
         &node->member;                                    \
         node = aos_container_of(tmp, type, member), tmp = tmp ? tmp->next : tmp)

#define AOS_SLIST_HEAD_INIT(name) { }
#define AOS_SLIST_HEAD(name) \
        slist_t name = AOS_SLIST_HEAD_INIT(name)

/**
 * get the struct for this entry
 * @param[in]   ptr      the &struct list_head pointer.
 * @param[in]   type     the type of the struct this is embedded in.
 * @param[in]   member   the name of the list_struct within the struct.
 */
#define slist_entry(addr, type, member) ({                               \
    addr ? (type *)((long)addr - aos_offsetof(type, member)) : (type *)addr; \
})

/**
 * get the struct for this entry
 * @param[in]   ptr      the &struct list_head pointer.
 * @param[in]   type     the type of the struct this is embedded in.
 * @param[in]   member   the name of the list_struct within the struct.
 */
#define slist_first_entry(ptr, type, member) \
    slist_entry((ptr)->next, type, member)

/**
 * get zhe list length
 *
 * @param[in]   queue    he head for your list
 */
#define slist_entry_number(queue) ({ \
    int num; \
    slist_t *cur = queue; \
    for (num=0;cur->next;cur=cur->next, num++); \
    num; \
})

#ifdef __cplusplus
}
#endif

#endif /* AOS_LIST_H */


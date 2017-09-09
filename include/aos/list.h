/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef YOS_LIST_H
#define YOS_LIST_H

#ifdef __cplusplus
extern "C"
{
#endif

#define aos_offsetof(type, member) ({ \
    type tmp;                         \
    (long)(&tmp.member) - (long)&tmp; \
})

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

static inline void INIT_YOS_DLIST_HEAD(dlist_t *list)
{
    list->next = list;
    list->prev = list;
}

static inline int dlist_empty(const dlist_t *head)
{
    return head->next == head;
}

/* initialize list head staticly */
#define YOS_DLIST_INIT(list)  {&(list), &(list)}

#define dlist_first_entry(ptr, type, member) \
    dlist_entry((ptr)->next, type, member)

#define dlist_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define dlist_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
    pos = n, n = pos->next)

#define dlist_for_each_entry(queue, node, type, member) \
    for (node = aos_container_of((queue)->next, type, member); \
         &node->member != (queue); \
         node = aos_container_of(node->member.next, type, member))

#define dlist_for_each_entry_safe(queue, n, node, type, member) \
    for (node = aos_container_of((queue)->next, type, member), \
         n = (queue)->next ? (queue)->next->next : NULL; \
         &node->member != (queue); \
         node = aos_container_of(n, type, member), n = n ? n->next : NULL)

/**
* list_entry - get the struct for this entry
* @ptr:        the &struct list_head pointer.
* @type:       the type of the struct this is embedded in.
* @member:     the name of the list_struct within the struct.
*/
#define list_entry(ptr, type, member) \
        aos_container_of(ptr, type, member)

/**
* list_for_each_entry_reverse_t - iterate backwards over list of given type.
* @pos:        the type * to use as a loop cursor.
* @head:       the head for your list.
* @member:     the name of the list_head within the struct.
* @type:       the type of the struct this is embedded in.
*/
#define dlist_for_each_entry_reverse(pos, head, member, type) \
    for (pos = list_entry((head)->prev, type, member);  \
         &pos->member != (head);    \
         pos = list_entry(pos->member.prev, type, member))


#define dlist_entry_number(queue) ({ \
    int num; \
    dlist_t *cur = queue; \
    for (num=0;cur->next != queue;cur=cur->next, num++); \
    num; \
})

#define YOS_DLIST_HEAD_INIT(name) { &(name), &(name) }
#define YOS_DLIST_HEAD(name) \
        dlist_t name = YOS_DLIST_HEAD_INIT(name)

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

/*
 * slist_t *queue
 * type *node
 * type
 * member
 */
#define slist_for_each_entry(queue, node, type, member)    \
    for (node = aos_container_of((queue)->next, type, member); \
         &node->member;                                    \
         node = aos_container_of(node->member.next, type, member))

/*
 * slist_t *queue
 * slist_t *tmp
 * type *node
 * type
 * member
 */
#define slist_for_each_entry_safe(queue, tmp, node, type, member)    \
    for (node = aos_container_of((queue)->next, type, member), \
         tmp = (queue)->next ? (queue)->next->next : NULL; \
         &node->member;                                    \
         node = aos_container_of(tmp, type, member), tmp = tmp ? tmp->next : tmp)

#define YOS_SLIST_HEAD_INIT(name) { }
#define YOS_SLIST_HEAD(name) \
        slist_t name = YOS_SLIST_HEAD_INIT(name)

#define slist_entry(addr, type, member) ({                               \
    addr ? (type *)((long)addr - aos_offsetof(type, member)) : (type *)addr; \
})

#define slist_first_entry(ptr, type, member) \
    slist_entry((ptr)->next, type, member)

#define slist_entry_number(queue) ({ \
    int num; \
    slist_t *cur = queue; \
    for (num=0;cur->next;cur=cur->next, num++); \
    num; \
})

#ifdef __cplusplus
}
#endif

#endif /* YOS_LIST_H */


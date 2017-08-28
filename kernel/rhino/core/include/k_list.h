#ifndef K_LIST_H
#define K_LIST_H

typedef struct klist_s {
    struct klist_s *next;
    struct klist_s *prev;
} klist_t;

#define yunos_list_entry(node, type, member) ((type *)((uint8_t *)(node) - (size_t)(&((type *)0)->member)))

#define yunos_list_for_each_entry(pos, head, member)              \
    for (pos = yunos_list_entry((head)->next, typeof(*pos), member);  \
         &pos->member != (head);    \
         pos = yunos_list_entry(pos->member.next, typeof(*pos), member))


YUNOS_INLINE void klist_init(klist_t *list_head)
{
    list_head->next = list_head;
    list_head->prev = list_head;
}

YUNOS_INLINE uint8_t is_klist_empty(klist_t *list)
{
    return (list->next == list);
}

YUNOS_INLINE void klist_insert(klist_t *head, klist_t *element)
{
    element->prev = head->prev;
    element->next = head;

    head->prev->next = element;
    head->prev       = element;
}

YUNOS_INLINE void klist_add(klist_t *head, klist_t *element)
{
    element->prev = head;
    element->next = head->next;

    head->next->prev = element;
    head->next = element;
}

YUNOS_INLINE void klist_rm(klist_t *element)
{
    element->prev->next = element->next;
    element->next->prev = element->prev;
}

YUNOS_INLINE void klist_rm_init(klist_t *element)
{
    element->prev->next = element->next;
    element->next->prev = element->prev;

    element->next = element->prev = element;
}


#endif /* K_LIST_H */


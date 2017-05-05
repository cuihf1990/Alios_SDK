/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
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

#ifndef K_LIST_H
#define K_LIST_H

typedef struct klist_s {
    struct klist_s *next;
    struct klist_s *prev;
} klist_t;

#define yunos_list_entry(node, type, member) ((type *)((uint8_t *)(node) - (size_t)(&((type *)0)->member)))

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

#endif /* K_LIST_H */


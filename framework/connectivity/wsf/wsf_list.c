/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
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

#include <string.h>
#include <stdio.h>

#include "os.h"
#include "wsf_list.h"

wsf_code wsf_list_push_back(wsf_list_t *list, void *data, size_t length)
{
    size_t len = sizeof(wsf_list_node_t);
    wsf_list_node_t *node = (wsf_list_node_t *)os_malloc(len);
    if (!node) {
        return WSF_FAIL;
    }

    node->next = NULL;
    node->data = data;
    node->length = length;

    if (list) {
        if (!list->head) {
            list->head = node;
            list->tail = node;
        } else {
            list->tail->next = node;
            list->tail = node;
        }
    }

    return WSF_SUCCESS;

}

wsf_list_node_t *wsf_list_pop_front(wsf_list_t *list)
{
    wsf_list_node_t *node = NULL;
    if (list) {
        if (list->head) {
            node = list->head;
            list->head = list->head->next;
            node->next = NULL;
        }
        if (!list->head) {
            list->tail = NULL;
        }
    }
    return node;
}


int wsf_list_is_empty(wsf_list_t *list)
{
    return !(list && list->head);
}

void wsf_list_init(wsf_list_t *list)
{
    list->head = NULL;
    list->tail = NULL;
}
void wsf_list_os_free(wsf_list_t *list)
{
    wsf_list_node_t *node = NULL;
    while ((node = wsf_list_pop_front(list)) != NULL) {
        os_free(node);
    }

}

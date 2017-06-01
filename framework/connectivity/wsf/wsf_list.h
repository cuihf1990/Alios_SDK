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

#ifndef WSF_LIST_H 
#define WSF_LIST_H
#include "wsf_defines.h"
#include <stdlib.h>

typedef struct wsf_list_node_t {
    void       *data;
    size_t        length;
    struct wsf_list_node_t *next;
} wsf_list_node_t;

typedef struct wsf_list_t {
    wsf_list_node_t *head;
    wsf_list_node_t *tail;
} wsf_list_t;

void wsf_list_init(wsf_list_t *list); 
/**
 * append the data to the list, the data size is 'length'
 *
 */
wsf_code wsf_list_push_back(wsf_list_t *list, void *data, size_t length);

int wsf_list_is_empty(wsf_list_t *list);

void wsf_list_free(wsf_list_t *list);

#endif

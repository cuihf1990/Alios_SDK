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

#ifndef K_MM_FIRSTFIT_H
#define K_MM_FIRSTFIT_H


/**
 * This function will alloc a mem from a pool
 * @param[in]       region_head     pointer to the memory region
 * @param[out]      mem             pointer to the mem
 * @param[in]       mem_size        size of the mem to malloc
 * @param[in]       allocator      allocator of memory
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_mm_ff_alloc(k_mm_region_head_t * region_head, void **mem,size_t size, size_t allocator);

/**
 * This function will free mem of a pool
 * @param[in]  pool  pointer to the pool
 * @param[in]  mem   pointer to the mem
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_mm_ff_free(k_mm_region_head_t * region_head, void *mem);

#endif /* K_MM_FIRSTFIT_H */


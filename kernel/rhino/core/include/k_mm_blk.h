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

#ifndef K_MM_BLK_H
#define K_MM_BLK_H

typedef struct {
    kobj_type_t   obj_type;
    const name_t *pool_name;
    size_t        blk_size;
    size_t        blk_avail;
    size_t        blk_whole;
    uint8_t      *avail_list;
#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    klist_t       mblkpool_stats_item;
#endif
} mblk_pool_t;

/**
 * This function will init a blk-pool
 * @param[in]  pool        pointer to the pool
 * @param[in]  name        name of the pool
 * @param[in]  pool_start  start addr of the pool
 * @param[in]  blk_size    size of the blk
 * @param[in]  pool_size   size of the pool
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_mblk_pool_init(mblk_pool_t *pool, const name_t *name,
                             void *pool_start,
                             size_t blk_size, size_t pool_size);

/**
 * This function will alloc a blk-pool
 * @param[in]  pool  pointer to a pool
 * @param[in]  blk   pointer to a blk
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_mblk_alloc(mblk_pool_t *pool, void **blk);

/**
 * This function will free a blk-pool
 * @param[in]  pool  pointer to the pool
 * @param[in]  blk   pointer to the blk
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_mblk_free(mblk_pool_t *pool, void *blk);

#endif /* K_MM_BLK_H */


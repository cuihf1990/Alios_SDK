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

#ifndef K_BITMAP_H
#define K_BITMAP_H

#define BITMAP_UNIT_SIZE 32U
#define BITMAP_UNIT_MASK 0X0000001F
#define BITMAP_UNIT_BITS 5U

#define BITMAP_MASK(nr) (1UL << (BITMAP_UNIT_SIZE - 1U - ((nr) & BITMAP_UNIT_MASK)))
#define BITMAP_WORD(nr) ((nr) >> BITMAP_UNIT_BITS)

/**
 ** This MACRO will declare a bitmap
 ** @param[in]  name  the name of the bitmap to declare
 ** @param[in]  bits  the bits of the bitmap
 ** @return  no return
 **/
#define BITMAP_DECLARE(name, bits) uint32_t name[((bits) + (BITMAP_UNIT_SIZE - 1U)) >> BITMAP_UNIT_BITS]

#if (YUNOS_CONFIG_BITMAP_HW != 0)
extern int32_t cpu_bitmap_clz(uint32_t val);
#endif

/**
 ** This function will set a bit of the bitmap
 ** @param[in]  bitmap  pointer to the bitmap
 ** @param[in]  nr      position of the bitmap to set
 ** @return  no return
 **/
YUNOS_INLINE void yunos_bitmap_set(uint32_t *bitmap, int32_t nr)
{
    bitmap[BITMAP_WORD(nr)] |= BITMAP_MASK(nr);
}

/**
 ** This function will clear a bit of the bitmap
 ** @param[in]  bitmap  pointer to the bitmap
 ** @param[in]  nr      position of the bitmap to clear
 ** @return  no return
 **/
YUNOS_INLINE void yunos_bitmap_clear(uint32_t *bitmap, int32_t nr)
{
    bitmap[BITMAP_WORD(nr)] &= ~BITMAP_MASK(nr);
}

/**
 ** This function will find the first bit(1) of the bitmap
 ** @param[in]  bitmap  pointer to the bitmap
 ** @return  the first bit position
 **/
YUNOS_INLINE int yunos_find_first_bit(uint32_t *bitmap)
{
    int32_t  nr  = 0;
    uint32_t tmp = 0;

    while (*bitmap == 0UL) {
        nr += BITMAP_UNIT_SIZE;
        bitmap++;
    }

    tmp = *bitmap;

#if (YUNOS_CONFIG_BITMAP_HW == 0)
    if (!(tmp & 0XFFFF0000)) {
        tmp <<= 16;
        nr   += 16;
    }

    if (!(tmp & 0XFF000000)) {
        tmp <<= 8;
        nr   += 8;
    }

    if (!(tmp & 0XF0000000)) {
        tmp <<= 4;
        nr   += 4;
    }

    if (!(tmp & 0XC0000000)) {
        tmp <<= 2;
        nr   += 2;
    }

    if (!(tmp & 0X80000000)) {
        nr   += 1;
    }
#else
    nr += cpu_bitmap_clz(tmp);
#endif

    return nr;
}

#endif /* K_BITMAP_H */


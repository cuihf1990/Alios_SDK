/****************************************************************************
 * Copyright (C) 2015 The YunOS Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/
#ifndef _MXD_TYPE_H_
#define _MXD_TYPE_H_

#include <stdint.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint8_t BD_ADDR[6];

typedef uint8_t BOOLEAN;
#define TRUE  1
#define FALSE 0

#define MS2BT(x) ((int)(x/0.625))
#define STREAM_TO_UINT8(u8, p)   {u8 = (uint8_t)(*(p)); (p) += 1;}
#define STREAM_TO_UINT16(u16, p) {u16 = ((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8)); (p) += 2;}
#define STREAM_TO_UINT32(u32, p) {u32 = (((uint32_t)(*(p))) + ((((uint32_t)(*((p) + 1)))) << 8) + ((((uint32_t)(*((p) + 2)))) << 16) + ((((uint32_t)(*((p) + 3)))) << 24)); (p) += 4;}
#define STREAM_TO_BDADDR(a, p)   {register int ijk; register uint8_t *pbda = (uint8_t *)a + 6 - 1; for (ijk = 0; ijk < 6; ijk++) *pbda-- = *p++;}
#define STREAM_TO_ARRAY(a, p, len) {register int ijk; for (ijk = 0; ijk < len; ijk++) ((uint8_t *) a)[ijk] = *p++;}
#define STREAM_TO_ARRAY8(a, p)   {register int ijk; register uint8_t *_pa = (uint8_t *)a + 7; for (ijk = 0; ijk < 8; ijk++) *_pa-- = *p++;}

#define UINT8_TO_STREAM(p, u8)   {*(p)++ = (uint8_t)(u8);}
#define UINT16_TO_STREAM(p, u16) {*(p)++ = (uint8_t)(u16); *(p)++ = (uint8_t)((u16) >> 8);}
#define UINT32_TO_STREAM(p, u32) {*(p)++ = (uint8_t)(u32); *(p)++ = (uint8_t)((u32) >> 8); *(p)++ = (uint8_t)((u32) >> 16); *(p)++ = (uint8_t)((u32) >> 24);}

#define ARRAY8_TO_STREAM(p, a)   {register int ijk; for (ijk = 0; ijk < 8;            ijk++) *(p)++ = (uint8_t) a[7 - ijk];}
#define BDADDR_TO_STREAM(p, a)   {register int ijk; for (ijk = 0; ijk < BD_ADDR_LEN;  ijk++) *(p)++ = (uint8_t) a[BD_ADDR_LEN - 1 - ijk];}
#define ARRAY_TO_STREAM(p, a, len) {register int ijk; for (ijk = 0; ijk < len;        ijk++) *(p)++ = (uint8_t) a[ijk];}

#define BD_ADDR_LEN     6                   /* Device address length */
typedef uint8_t BD_ADDR[BD_ADDR_LEN];         /* Device address */
typedef uint8_t *BD_ADDR_PTR;                 /* Pointer to Device Address */

#define BT_OCTET8_LEN    8
typedef uint8_t BT_OCTET8[BT_OCTET8_LEN];   /* octet array: size 16 */

#define LINK_KEY_LEN    16
typedef uint8_t LINK_KEY[LINK_KEY_LEN];       /* Link Key */

#define AMP_LINK_KEY_LEN        32
typedef uint8_t AMP_LINK_KEY[AMP_LINK_KEY_LEN];   /* Dedicated AMP and GAMP Link Keys */

#define BT_OCTET16_LEN    16
typedef uint8_t BT_OCTET16[BT_OCTET16_LEN];   /* octet array: size 16 */

#define PIN_CODE_LEN    16
typedef uint8_t PIN_CODE[PIN_CODE_LEN];       /* Pin Code (upto 128 bits) MSB is 0 */
typedef uint8_t *PIN_CODE_PTR;                /* Pointer to Pin Code */

#define DEV_CLASS_LEN   3
typedef uint8_t DEV_CLASS[DEV_CLASS_LEN];     /* Device class */
typedef uint8_t *DEV_CLASS_PTR;               /* Pointer to Device class */

#define EXT_INQ_RESP_LEN   3
typedef uint8_t EXT_INQ_RESP[EXT_INQ_RESP_LEN];/* Extended Inquiry Response */
typedef uint8_t *EXT_INQ_RESP_PTR;             /* Pointer to Extended Inquiry Response */

#define BD_NAME_LEN     248
typedef uint8_t BD_NAME[BD_NAME_LEN + 1];         /* Device name */
typedef uint8_t *BD_NAME_PTR;                 /* Pointer to Device name */

#define BD_FEATURES_LEN 8
typedef uint8_t BD_FEATURES[BD_FEATURES_LEN]; /* LMP features supported by device */

#define BT_EVENT_MASK_LEN  8
typedef uint8_t BT_EVENT_MASK[BT_EVENT_MASK_LEN];   /* Event Mask */

#endif

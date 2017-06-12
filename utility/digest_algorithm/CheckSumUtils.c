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

#include "CheckSumUtils.h"


uint8_t UpdateCRC8(uint8_t crcIn, uint8_t byte)
{
  uint8_t crc = crcIn;
  uint8_t i;
 
  crc^= byte;
     
  for(i=0;i<8;i++)
  {
      if(crc&0x01)
      {
          crc = (crc>>1)^0x8C;                          
      }
      else
         crc>>=1;                  
  } 
  return crc; 
}


void CRC8_Init( CRC8_Context *inContext )
{
  inContext->crc = 0;
}


void CRC8_Update( CRC8_Context *inContext, const void *inSrc, size_t inLen )
{
  const uint8_t * src = (const uint8_t *) inSrc;
  const uint8_t * srcEnd = src + inLen;
  while( src < srcEnd )
    inContext->crc = UpdateCRC8(inContext->crc, *src++); 
}


void CRC8_Final( CRC8_Context *inContext, uint8_t *outResult )
{
    //inContext->crc = UpdateCRC8(inContext->crc, 0);
    *outResult = inContext->crc&0xffu;
}
 

/*******************************************************************************/

uint16_t UpdateCRC16(uint16_t crcIn, uint8_t byte)
{
  uint32_t crc = crcIn;
  uint32_t in = byte | 0x100;
  
  do
  {
    crc <<= 1;
    in <<= 1;  
    if(in & 0x100)
      ++crc;
    if(crc & 0x10000)
      crc ^= 0x1021;
  }while(!(in & 0x10000));
  return crc & 0xffffu;
}

void CRC16_Init( CRC16_Context *inContext )
{
  inContext->crc = 0;
}


void CRC16_Update( CRC16_Context *inContext, const void *inSrc, size_t inLen )
{
  const uint8_t * src = (const uint8_t *) inSrc;
  const uint8_t * srcEnd = src + inLen;
  while( src < srcEnd )
    inContext->crc = UpdateCRC16(inContext->crc, *src++);
}


void CRC16_Final( CRC16_Context *inContext, uint16_t *outResult )
{
  inContext->crc = UpdateCRC16(inContext->crc, 0);
  inContext->crc = UpdateCRC16(inContext->crc, 0);
  *outResult = inContext->crc&0xffffu;
}


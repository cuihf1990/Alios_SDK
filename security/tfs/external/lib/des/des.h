
/***********************************************************************************
 * include/nuttx/crypto/des.h
 *
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
 *
 ************************************************************************************/

#ifndef _DES_H_
#define _DES_H_

#define MODE_ECB    0
#define MODE_CBC    1

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
/****************************************************************************
 * Name: des3_en
 *
 * Description:
 *   3des_encryption:  Given DES key and 16 bytes plain text, cipher
 *   text of 16 bytes is computed. The DES implementation is in mode ECB
 *   (Electronic Code Book).
 *
 * Input Parameters:
 *  key   DES key of size 16 bytes
 *  state 16 bytes of plain text and cipher text
 *
 * Returned Value
 *   None
 *
 ****************************************************************************/
int des3_en(unsigned char *key, int key_len, int mode, unsigned char *input, unsigned int input_len,
            unsigned char *output, unsigned int output_len);

/****************************************************************************
 * Name: des3_de
 *
 * Description:
 *   3des_decryption:  Given DES key and 16 bytes plain text, cipher
 *   text of 16 bytes is computed. The DES implementation is in mode ECB
 *   (Electronic Code Book).
 *
 * Input Parameters:
 *  key   DES key of size 16 bytes
 *  state 16 bytes of plain text and cipher text
 *
 * Returned Value
 *   None
 *
 ****************************************************************************/
int des3_de(unsigned char *key, int key_len, int mode, unsigned char *input, unsigned int input_len,
            unsigned char *output, unsigned int output_len);

int des3_mac(unsigned char *input, int inputLen, unsigned  char *key, int keyLen,
             unsigned char *output);


#endif

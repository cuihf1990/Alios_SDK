/************************************************************************************
 * crypto/des.c
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <des.h>


/************************************************************************************
 * Name: ini_permutation
 *
 * Description:
 *    This function is used to inital permutation with the data which to be encipher or decipher
 *
 * Parameters:
 * s    return deciphed data and deciphing data
 *
 * Return:
 *   void
 ************************************************************************************/
void ini_permutation(unsigned char *s)
{
    unsigned char p[8];
    unsigned char buf;
    unsigned char rule = 1;
    int i, j;

    //begin to inital permutation
    for (i = 0; i < 8; i++) {
        p[i] = 0;

        for (j = 7; j >= 0; j--) {
            buf = s[j];
            s[j] = s[j] >> 1;
            buf = buf & rule;
            p[i] = p[i] | buf;

            if (j == 0)
            { break; }

            p[i] = p[i] << 1;
        }
    }

    s[0] = p[6];
    s[1] = p[4];
    s[2] = p[2];
    s[3] = p[0];
    s[4] = p[7];
    s[5] = p[5];
    s[6] = p[3];
    s[7] = p[1];

    return;
}

/************************************************************************************
* Name: shift
*
* Description:
*    is used to move key left
*
* Parameters:
*    s     children key in C or D unit
*    n     the level of the loop
* Return:
*    void
************************************************************************************/
void shift(unsigned char *s, int n)
{
    const unsigned char lsh[16] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};
    unsigned char tmp[4];
    unsigned char rule = 128;
    int i, j;

    //begin to move left according the table lsh
    for (i = 0; i < lsh[n - 1]; i++) {
        for (j = 0; j < 4; j++) {
            tmp[j] = s[j] & rule;
            s[j] = s[j] << 1;
        }

        for (j = 0; j < 4; j++) {
            if (j == 3) {
                tmp[0] = tmp[0] >> 6;
                s[j] = s[j] | tmp[0];
                break;
            }

            tmp[j + 1] = tmp[j + 1] >> 6;
            s[j] = s[j] | tmp[j + 1];
        }
    }
}

/************************************************************************************
* Name: key_pc_1
*
* Description:
*    is used to call the PC-1 permutation
*
* Parameter Description:
*    s     the main key
* Return:
*     void
************************************************************************************/
void key_pc_1(unsigned char *s)
{
    const unsigned char table[8][8] = {{57, 49, 41, 33, 25, 17, 9, '\0'},
        {1,  58, 50, 42, 34, 26, 18, '\0'},
        {10, 2,  59, 51, 43, 35, 27, '\0'},
        {19, 11, 3,  60, 52, 44, 36, '\0'},
        {63, 55, 47, 39, 31, 23, 15, '\0'},
        {7,  62, 54, 46, 38, 30, 22, '\0'},
        {14, 6,  61, 53, 45, 37, 29, '\0'},
        {21, 13, 5,  28, 20, 12, 4, '\0'}
    };
    unsigned char p[9] = {0, 0, 0, 0, 0, 0, 0, 0, '\0'};
    unsigned char buf;
    unsigned char rule = 128;
    int  found = 0;
    int  i = 0, j = 0, k, l;

    //begin to the PC-1
    for (; i < 8; i++)
        for (; j < 64; j++) {
            buf = s[i] & rule;

            for (k = 0; k < 8; k++) {
                for (l = 0; l < 7; l++)
                    if ((j + 1) == table[k][l]) {
                        buf = buf >> l;
                        p[k] = p[k] | buf;
                        found = 1;
                        break;
                    }

                if (found) {
                    found = 0;
                    break;
                }
            }

            s[i] = s[i] << 1;

            if ((j + 1) % 8 == 0) {
                j++;
                break;
            }
        }

    for (i = 0; i < 8; i++)
    { s[i] = p[i]; }

    return;
}

/************************************************************************************
* Name key_pc_2
*
* Descrpition:
*    is used to call the PC-2 permutation
*
* Parameters:
*    s    the main key
* Return:
*    void
************************************************************************************/
void key_pc_2(unsigned char *s)
{
    const unsigned char table[8][7] = {{14, 17, 11, 24, 1,  5,  '\0'},
        {3,  28, 15, 6,  21, 10, '\0'},
        {23, 19, 12, 4,  26, 8,  '\0'},
        {16, 7,  27, 20, 13, 2,  '\0'},
        {41, 52, 31, 37, 47, 55, '\0'},
        {30, 40, 51, 45, 33, 48, '\0'},
        {44, 49, 39, 56, 34, 53, '\0'},
        {46, 42, 50, 36, 29, 32, '\0'}
    };
    unsigned char p[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char buf;
    unsigned char rule = 128;
    int  found = 0;
    int  i = 0, j = 0, k, l;

    //begin the PC-2
    for (; i < 8; i++)
        for (; j < 57; j++) {
            buf = s[i] & rule;

            for (k = 0; k < 8; k++) {
                for (l = 0; l < 6; l++)
                    if ((j + 1) == table[k][l]) {
                        buf = buf >> l;
                        p[k] = p[k] | buf;
                        found = 1;
                        break;
                    }

                if (found) {
                    found = 0;
                    break;
                }
            }

            s[i] = s[i] << 1;

            if ((j + 1) % 7 == 0) {
                j++;
                break;
            }
        }

    for (i = 0; i < 8; i++)
    { s[i] = p[i]; }

    return;
}

/************************************************************************************
* Name: exp_perm
* Description:
*    is used to call the e permutation
*
* Parameters:
*    s1    the right 4 bytes
*    key   the key in this level
*
* Return Value:
*     void
************************************************************************************/
void exp_perm(unsigned char *s1, unsigned char *s2)
{
    unsigned char top[4];
    unsigned char end[4];
    unsigned char rule_top = 1;
    unsigned char rule_end = 128;
    unsigned char tmp_top;
    unsigned char tmp_end;
    int  i;

    //begin the e permutation
    for (i = 0; i < 4; i++) {
        top[i] = s1[i] & rule_top;
        top[i] = top[i] << 7;
        end[i] = s1[i] & rule_end;
        end[i] = end[i] >> 5;
    }

    tmp_top = top[3];

    for (i = 3; i >= 0; i--) {
        if (i == 0) {
            top[i] = tmp_top;
            break;
        }

        top[i] = top[i - 1];
    }

    tmp_end = end[0];

    for (i = 0; i < 4; i++) {
        if (i == 3) {
            end[i] = tmp_end;
            break;
        }

        end[i] = end[i + 1];
    }

    for (i = 0; i < 4; i++) {
        tmp_top = s1[i] & 248;
        tmp_top = tmp_top >> 1;
        tmp_top = top[i] | tmp_top;
        s2[i * 2] = tmp_top;
    }

    for (i = 0; i < 4; i++) {
        tmp_end = s1[i] & 31;
        tmp_end = tmp_end << 3;
        tmp_end = end[i] | tmp_end;
        s2[i * 2 + 1] = tmp_end;
    }
}

/************************************************************************************
* Name: rk_xor
* Description:
*     is used to xor the right unit and the key in current level.
*
* Parameters:
*    s1  the right 4 bytes througth the e permutation
*    s2  the key in this level;
* Return:
*   void
************************************************************************************/

void rk_xor(unsigned char *s1, unsigned char *s2)
{
    int i;

    for (i = 0; i < 8; i++)
    { s1[i] = s1[i] ^ s2[i]; }
}

/************************************************************************************
* Name: s_box
*    is used to call the S_BOX
*
* Parameters:
*   p  the right 4 bytes througth the e permutation and xor
*
* Return:
*   void
************************************************************************************/
void s_box(unsigned char *p)
{
    const unsigned char s[8][4][16] = {
        {
            {14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7},
            {0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8},
            {4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0},
            {15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13}
        },
        {
            {15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10},
            {3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5},
            {0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15},
            {13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9}
        },
        {
            {10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8},
            {13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1},
            {13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7},
            {1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12}
        },
        {
            {7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15},
            {13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9},
            {10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4},
            {3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14}
        },
        {
            {2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9},
            {14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6},
            {4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14},
            {11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3}
        },
        {
            {12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11},
            {10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8},
            {9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6},
            {4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13}
        },
        {
            {4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1},
            {13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6},
            {1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2},
            {6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12}
        },
        {
            {13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7},
            {1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2},
            {7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8},
            {2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11}
        }
    };
    unsigned char line_rule = 132;
    unsigned char col_rule = 120;
    unsigned char line;
    unsigned char col;
    int  i;

    //to change in the S_BOX table
    for (i = 0; i < 8; i++) {
        line = p[i] & line_rule;
        col = p[i] & col_rule;
        col = col >> 3;

        switch (line) {
            case 0:
                p[i] = s[i][0][col];
                break;

            case 4:
                p[i] = s[i][1][col];
                break;

            case 128:
                p[i] = s[i][2][col];
                break;

            case 132:
                p[i] = s[i][3][col];
                break;

            default:
                break;
        }
    }
}

/************************************************************************************
* Name: p_perm
*    is used to the p permutation
*
* Parameters:
*    s  the right 4 bytes througth the e permutation and xor
*       and s_box
* Return:
*   void
************************************************************************************/

void p_perm(unsigned char *s)
{
    const unsigned char table[8][4] = {
        {16, 7, 20, 21},
        {29, 12, 28, 17},
        {1, 15, 23, 26},
        {5, 18, 31, 10},
        {2, 8, 24, 14},
        {32, 27, 3, 9},
        {19, 13, 30, 6},
        {22, 11, 4, 25}
    };
    unsigned char p[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char buf;
    unsigned char rule = 128;
    int found = 0;
    int i = 0, j = 0, k, l;

    //begin to the P permutation
    for (k = 0; k < 8; k++)
    { s[k] = s[k] << 4; }

    for (; i < 8; i++)
        for (; j < 32; j++) {
            buf = s[i] & rule;

            for (k = 0; k < 8; k++) {
                for (l = 0; l < 4; l++)
                    if ((j + 1) == table[k][l]) {
                        buf = buf >> l;
                        p[k] = p[k] | buf;
                        found = 1;
                        break;
                    }

                if (found) {
                    found = 0;
                    break;
                }
            }

            s[i] = s[i] << 1;

            if ((j + 1) % 4 == 0) {
                j++;
                break;
            }
        }

    for (i = 0; i < 4; i++) {
        s[i] = p[2 * i];
        p[2 * i + 1] = p[2 * i + 1] >> 4;
        s[i] = s[i] | p[2 * i + 1];
    }
}

////////////////////////////////////////////////////////////////
// Function lf_xor is used to xor the left unit and P(B) unit
// Function Pammes Description:
// lf_xor(s1,s2)
// Parameter Description:
//   s1:  the left unit
//   s2:  the right 4 bytes througth the F function
// Return Val:
//   void
/////////////////////////////////////////////////////////////////

void lf_xor(unsigned char *s1, unsigned char *s2)
{
    int i;

    for (i = 0; i < 4; i++)
    { s1[i] = s1[i] ^ s2[i]; }
}

////////////////////////////////////////////////////////////////
// Function f_rk is used to call the F encipher function
// Function Pammes Description
// f_rk(s1,s2)
// Parameter Description:
//   s1:     the right unit
//   s2:     the key in current level
// Return Val:
//     void
/////////////////////////////////////////////////////////////////

void f_rk(unsigned char *s1, unsigned char *s2)
{
    unsigned char p[8];
    int i;

    //Call the E permutation
    exp_perm(s1, p);

    //Xor the right unit and the key in current level
    rk_xor(p, s2);

    //Call the S_BOX
    s_box(p);

    //Call the P permutation
    p_perm(p);

    for (i = 0; i < 4; i++)
    { s1[i] = p[i]; }
}

////////////////////////////////////////////////////////////////
// Function lst_permutation is used to call the last permutation
//    (LP-1).
// Function Pammes Description:
// lst_permutation(s)
// Parameter Description:
//   s:  the data throught the decipher
// Return Val:
//   void
/////////////////////////////////////////////////////////////////

void lst_permutation(unsigned char *s)
{
    unsigned char p[8];
    unsigned char q[8];
    unsigned char buf;
    unsigned char rule = 1;
    int i, j;

    //begin the last permutation (LP-1)
    q[0] = s[3];
    q[1] = s[7];
    q[2] = s[2];
    q[3] = s[6];
    q[4] = s[1];
    q[5] = s[5];
    q[6] = s[0];
    q[7] = s[4];

    for (i = 0; i < 8; i++) {
        p[i] = 0;

        for (j = 7; j >= 0; j--) {
            buf = q[j];
            q[j] = q[j] >> 1;
            buf = buf & rule;
            p[i] = p[i] | buf;

            if (j == 0) { break; }

            p[i] = p[i] << 1;
        }
    }

    for (i = 0; i < 8; i++)
    { s[i] = p[i]; }

    return;
}

////////////////////////////////////////////////////////////////
// Function KEY_GEN is used to genation the children key
// Function Pammes Description
// key_gen(s1,s2)
// Parameter Description:
//   s1:     the main key
//   s2:     the children keys
// Return Val:
//     void
/////////////////////////////////////////////////////////////////

void key_gen(unsigned char *s1, unsigned char s2[][8])
{
    unsigned char c[4];
    unsigned char d[4];
    unsigned char p[8];
    int i, j;

    //Call the PC-1
    key_pc_1(s1);

    //Create the C and D
    memcpy(c, s1, 4);
    memcpy(d, s1 + 4, 4);

    //begin to loop 16
    for (i = 1; i < 17; i++) {
        //move C and D left
        shift(c, i);
        shift(d, i);

        //Call the PC-2
        for (j = 0; j < 4; j++) {
            p[j] = c[j];
            p[j + 4] = d[j];
        }

        key_pc_2(p);
        memcpy(s2[i - 1], p, 8);
    }

    return;
}

////////////////////////////////////////////////////////////////
// Function DESENCIPH is used to encipher the data
// Function Pammes Description
// desenciph(data,key)
// Parameter Description:
//   data:  return enciphed data and enciphing data
//   key:   the key to encipher;
// Return Val:
//     void
/////////////////////////////////////////////////////////////////

void desenciph(unsigned char *data, unsigned char *key)
{
    unsigned char l[4];
    unsigned char r[4];
    unsigned char proc[4];
    unsigned char(*key_level)[8] = malloc(16 * 8);
    int i = 1;

    //inital permutation
    ini_permutation(data);

    //key generation
    key_gen(key, key_level);

    //create right and left units
    memcpy(l, data, 4);
    memcpy(r, data + 4, 4);
    memcpy(proc, r, 4);

    //begin to encipher

    for (i = 0; i < 16; i++) {
        f_rk(proc, key_level[i]);      //call the function F
        lf_xor(l, proc);               //Xor the left and and P(B)

        if (i == 15) { break; }            //end when loop 16

        //create the left and right units in the next loop
        memcpy(proc, l, 4);
        memcpy(l, r, 4);
        memcpy(r, proc, 4);
    }

    //Call the lp-1
    memcpy(data, l, 4);
    memcpy(data + 4, r, 4);
    lst_permutation(data);

    free(key_level);
}

////////////////////////////////////////////////////////////////
// Function DESDECIPH is used to decipher the data
// Function Pammes Description
// desdeciph(data,key)
// Parameter Description:
//   data:  return deciphed data and deciphing data
//   key:    the key to decipher;
// Return Val:
//     void
/////////////////////////////////////////////////////////////////

void desdeciph(unsigned char *data, unsigned char *key)
{
    unsigned char l[4];
    unsigned char r[4];
    unsigned char proc[4];
    unsigned char(*key_level)[8] = malloc(16 * 8);
    int i;

    //inital permutation
    ini_permutation(data);

    //key generation
    key_gen(key, key_level);

    //create right and left units
    memcpy(l, data, 4);
    memcpy(r, data + 4, 4);
    memcpy(proc, r, 4);

    //begin to decipher

    for (i = 15; i >= 0; i--) {
        f_rk(proc, key_level[i]);    //call the function F
        lf_xor(l, proc);             //Xor the left and and P(B)

        if (i == 0) { break; }           //end when loop 16

        //create the left and right units in the next loop
        memcpy(proc, l, 4);
        memcpy(l, r, 4);
        memcpy(r, proc, 4);
    }

    //Call the lp-1

    memcpy(data, l, 4);
    memcpy(data + 4, r, 4);
    lst_permutation(data);

    free(key_level);
}

////////////////////////////////////////////////////////////////
// Function DESDECIPH is used to encipher the data by 3des
// Function Pammes Description
// desdeciph(data,key)
// Parameter Description:
//   data:  return deciphed data and deciphing data
//   key:    the key to decipher; keylen, mode, input, input_len, output, output_len
// Return Val:
//   zero on success.
/////////////////////////////////////////////////////////////////
int des3_en(unsigned char *key, int key_len, int mode, unsigned char *input, unsigned int input_len, unsigned char *output, unsigned int output_len)
{
    int i, ptr, len;
    int pos = 0;
    unsigned char seed[8], des_data[8], key_buf[8];

    if (mode != MODE_ECB && mode != MODE_CBC)
    { return -1; }

    if (key_len != 16)
    { return -1; }

    if (input_len % 8 != 0)
    { return -1; }      // len of DATA must be the time of 8

    if (mode == MODE_CBC && input_len < 16)     // len of DATA must be larger than 8
    { return -1; }

    len = input_len;

    // Init seed
    if (mode == MODE_CBC) {
        memcpy(seed, input, 8);
        ptr = 8;
        len = len - 8;
    } else {
        ptr = 0;
    }

    while (len / 8 != 0) {

        memcpy(des_data, input + ptr, 8);

        if (mode == MODE_CBC) {
            for (i = 0; i < 8; i++)
            { des_data[i] = des_data[i] ^ seed[i]; }
        }

        //call the desenciph to encipher
        // DES+
        memcpy(key_buf, key, 8);
        desenciph(des_data, key_buf);

        // DES-
        memcpy(key_buf, key + 8, 8);
        desdeciph(des_data, key_buf);

        // DES+
        memcpy(key_buf, key, 8);
        desenciph(des_data, key_buf);

        if (mode == MODE_CBC)
        { memcpy(seed, des_data, 8); }

        //get result
        memcpy(output + pos, des_data, 8);
        pos += 8;

        len = len - 8;
        ptr = ptr + 8;
    }

    return 0;
}

////////////////////////////////////////////////////////////////
// Function DESDECIPH is used to decipher the data by 3des
// Function Pammes Description
// desdeciph(data,key)
// Parameter Description:
//   data:  return deciphed data and deciphing data
//   key:    the key to decipher; keylen, mode, input, input_len, output, output_len
// Return Val:
//   zero on success.
/////////////////////////////////////////////////////////////////
int des3_de(unsigned char *key, int key_len, int mode, unsigned char *input, unsigned int input_len, unsigned char *output, unsigned int output_len)
{
    int i, ptr, len;
    int pos = 0;
    unsigned char seed[8], des_data[8], key_buf[8];

    if (mode != MODE_ECB && mode != MODE_CBC)
    { return -1; }

    if (key_len != 16)
    { return -1; }

    if (input_len % 8 != 0)
    { return -1; }      // len of DATA must be the time of 8

    if (mode == MODE_CBC && input_len < 16)     // len of DATA must be larger than 8
    { return -1; }

    len = input_len;

    // Init seed
    if (mode == MODE_CBC) {
        memcpy(seed, input, 8);
        ptr = 8;
        len = len - 8;
    } else {
        ptr = 0;
    }

    while (len / 8 != 0) {
        memcpy(des_data, input + ptr, 8);

        if (mode == MODE_CBC) {
            for (i = 0; i < 8; i++)
            { des_data[i] = des_data[i] ^ seed[i]; }
        }

        //call the desenciph to encipher
        // DES-
        memcpy(key_buf, key, 8);
        desdeciph(des_data, key_buf);
        // DES+
        memcpy(key_buf, key + 8, 8);
        desenciph(des_data, key_buf);
        // DES-
        memcpy(key_buf, key, 8);
        desdeciph(des_data, key_buf);

        if (mode == MODE_CBC)
        { memcpy(seed, des_data, 8); }

        //get result
        memcpy(output + pos, des_data, 8);
        pos += 8;
        len = len - 8;
        ptr = ptr + 8;
    }

    return 0;
}




int des3_mac(unsigned char *input, int inputLen, unsigned  char *key, int keyLen, unsigned char *output)
{
    int i, ptr, len;
    unsigned char seed2[8], des_data[8], key_buf[8];

    if (keyLen != 16)
    { return -1; }      // len of KEY must be 16 bytes

    len = inputLen;

    if (len % 8 != 0)
    { return -1; }      // len of DATA must be the time of 8

    if (inputLen < 16)  // len of DATA must be larger than 16
    { return -1; }

    // Init seed
    for (i = 0; i < 8; i++)
    { seed2[i] = input[i]; }

    ptr = 8;
    len = len - 8;

    while (len / 8 != 0) {
        //get plain data
        for (i = ptr; i < ptr + 8; i++)
        { des_data[i - ptr] = input[i]; }

        for (i = 0; i < 8; i++)
        { des_data[i] = des_data[i] ^ seed2[i]; }

        //call the desenciph to encipher
        for (i = 0; i < 8; i++) // DES+
        { key_buf[i] = key[i]; }

        desenciph(des_data, key_buf);

        memcpy(seed2, des_data, 8);

        len = len - 8;
        ptr = ptr + 8;
    }

    // Decipher
    for (i = 8; i < 16; i++)
    { key_buf[i - 8] = key[i]; }

    desdeciph(des_data, key_buf);

    // Encipher
    for (i = 0; i < 8; i++)
    { key_buf[i] = key[i]; }

    desenciph(des_data, key_buf);

    //get result
    for (i = 0; i < 8; i++)
    { output[i] = des_data[i]; }

    return 0;
}


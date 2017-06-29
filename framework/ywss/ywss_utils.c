/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 *
 * Alibaba Group retains all right, title and interest (including all
 * intellectual property rights) in and to this computer program, which is
 * protected by applicable intellectual property laws.  Unless you have
 * obtained a separate written license from Alibaba Group., you are not
 * authorized to utilize all or a part of this computer program for any
 * purpose (including reproduction, distribution, modification, and
 * compilation into object code), and you must immediately destroy or
 * return to Alibaba Group all copies of this computer program.  If you
 * are licensed by Alibaba Group, your rights to utilize this computer
 * program are limited by the terms of that license.  To obtain a license,
 * please contact Alibaba Group.
 *
 * This computer program contains trade secrets owned by Alibaba Group.
 * and, unless unauthorized by Alibaba Group in writing, you agree to
 * maintain the confidentiality of this computer program and related
 * information and to not disclose this computer program and related
 * information to any other person or entity.
 *
 * THIS COMPUTER PROGRAM IS PROVIDED AS IS WITHOUT ANY WARRANTIES, AND
 * Alibaba Group EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * INCLUDING THE WARRANTIES OF MERCHANTIBILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, TITLE, AND NONINFRINGEMENT.
 */
#include "ywss_utils.h"

static char a2x(char ch)
{
    switch (ch) {
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case 'A':
        case 'a':
            return 10;
        case 'B':
        case 'b':
            return 11;
        case 'C':
        case 'c':
            return 12;
        case 'D':
        case 'd':
            return 13;
        case 'E':
        case 'e':
            return 14;
        case 'F':
        case 'f':
            return 15;
        default:
            break;;
    }
    return 0;
}

static const char *x2a = "0123456789ABCDEF";

int utils_str_to_hex(const char *str, int str_len, uint8_t *out, int out_buf_len)
{
    int i = 0;

    if (!(str && out && (strlen(str) % 2 == 0))) {
        return 0;
    }
    if (!(str_len && strlen(str) == str_len)) {
        return 0;
    }

    if (strlen(str) > out_buf_len * 2) {
        return 0;
    }

    while (i <= strlen(str)) {
        out[i / 2] = (a2x(str[i]) << 4) | a2x(str[i + 1]);
        i += 2;
    }

    return strlen(str) / 2;
}

int utils_hex_to_str(const uint8_t *buf, int buf_len, char *str, int str_buf_len)
{
    int len = 0;
    int i = 0;

    if (!(buf && str)) {
        return 0;
    }

    if (str_buf_len < buf_len * 2) {
        return 0;
    }

    while (i < buf_len) {
        str[len++] = x2a[(buf[i] >> 4) & 0x0F];
        str[len++] = x2a[(buf[i] >> 0) & 0x0F];
        i++;
    }

    if (str_buf_len > buf_len * 2) {
        str[len] = '\0'; /* add NULL-terminiated for easier use */
    }

    return buf_len * 2;
}
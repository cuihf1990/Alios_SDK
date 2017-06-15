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

#include "string.h"
#include "stdd_datatype.h"

static data_type_t g_datatype[] =
{
    {ALINK_DATATYPE_CLASS_NODATA, 0, "no_data", ALINK_DATATYPE_CLASS_NODATA},
    {ALINK_DATATYPE_CLASS_DATA, 1, "data8", ZCL_DATATYPE_DATA8},        //data类型用小写hex string表示
    {ALINK_DATATYPE_CLASS_DATA, 2, "data16", ZCL_DATATYPE_DATA16},
    {ALINK_DATATYPE_CLASS_DATA, 3, "data24", ZCL_DATATYPE_DATA24},
    {ALINK_DATATYPE_CLASS_DATA, 4, "data32", ZCL_DATATYPE_DATA32},
    {ALINK_DATATYPE_CLASS_DATA, 5, "data40", ZCL_DATATYPE_DATA40},
    {ALINK_DATATYPE_CLASS_DATA, 6, "data48", ZCL_DATATYPE_DATA48},
    {ALINK_DATATYPE_CLASS_DATA, 7, "data56", ZCL_DATATYPE_DATA56},
    {ALINK_DATATYPE_CLASS_DATA, 8, "data64", ZCL_DATATYPE_DATA64},
    {ALINK_DATATYPE_CLASS_LOGIC, 1, "boolean", ZCL_DATATYPE_BOOLEAN},   //boolean 类型用10进制字符串表示
    {ALINK_DATATYPE_CLASS_BITMAP, 1, "bitmap8", ZCL_DATATYPE_BITMAP8},  //bitmap类型用小写hex string表示
    {ALINK_DATATYPE_CLASS_BITMAP, 2, "bitmap16", ZCL_DATATYPE_BITMAP16},
    {ALINK_DATATYPE_CLASS_BITMAP, 3, "bitmap24", ZCL_DATATYPE_BITMAP24},
    {ALINK_DATATYPE_CLASS_BITMAP, 4, "bitmap32", ZCL_DATATYPE_BITMAP32},
    {ALINK_DATATYPE_CLASS_BITMAP, 5, "bitmap40", ZCL_DATATYPE_BITMAP40},
    {ALINK_DATATYPE_CLASS_BITMAP, 6, "bitmap48", ZCL_DATATYPE_BITMAP48},
    {ALINK_DATATYPE_CLASS_BITMAP, 7, "bitmap56", ZCL_DATATYPE_BITMAP56},
    {ALINK_DATATYPE_CLASS_BITMAP, 8, "bitmap64", ZCL_DATATYPE_BITMAP64},
    {ALINK_DATATYPE_CLASS_UINT, 1, "uint8", ZCL_DATATYPE_UINT8},        //uint类型用10进制字符串表示
    {ALINK_DATATYPE_CLASS_UINT, 2, "uint16", ZCL_DATATYPE_UINT16},
    {ALINK_DATATYPE_CLASS_UINT, 3, "uint24", ZCL_DATATYPE_UINT24},
    {ALINK_DATATYPE_CLASS_UINT, 4, "uint32", ZCL_DATATYPE_UINT32},
    {ALINK_DATATYPE_CLASS_UINT, 5, "uint40", ZCL_DATATYPE_UINT40},
    {ALINK_DATATYPE_CLASS_UINT, 6, "uint48", ZCL_DATATYPE_UINT48},
    {ALINK_DATATYPE_CLASS_UINT, 7, "uint56", ZCL_DATATYPE_UINT56},
    {ALINK_DATATYPE_CLASS_UINT, 8, "uint64", ZCL_DATATYPE_UINT64},
    {ALINK_DATATYPE_CLASS_INT, 1, "int8", ZCL_DATATYPE_INT8},           //int类型用10进制字符串表示
    {ALINK_DATATYPE_CLASS_INT, 2, "int16", ZCL_DATATYPE_INT16},
    {ALINK_DATATYPE_CLASS_INT, 3, "int24", ZCL_DATATYPE_INT24},
    {ALINK_DATATYPE_CLASS_INT, 4, "int32", ZCL_DATATYPE_INT32},
    {ALINK_DATATYPE_CLASS_INT, 5, "int40", ZCL_DATATYPE_INT40},
    {ALINK_DATATYPE_CLASS_INT, 6, "int48", ZCL_DATATYPE_INT48},
    {ALINK_DATATYPE_CLASS_INT, 7, "int56", ZCL_DATATYPE_INT56},
    {ALINK_DATATYPE_CLASS_INT, 8, "int64", ZCL_DATATYPE_INT64},
    {ALINK_DATATYPE_CLASS_ENUM, 1, "enum8", ZCL_DATATYPE_ENUM8},        //enum类型用10进制字符串表示
    {ALINK_DATATYPE_CLASS_ENUM, 2, "enum16", ZCL_DATATYPE_ENUM16},
    {ALINK_DATATYPE_CLASS_FLOAT, 2, "semi_float", ZCL_DATATYPE_SEMI_PREC},//float类型用点分10进制字符串表示
    {ALINK_DATATYPE_CLASS_FLOAT, 4, "float", ZCL_DATATYPE_SINGLE_PREC},
    {ALINK_DATATYPE_CLASS_FLOAT, 8, "double_float", ZCL_DATATYPE_DOUBLE_PREC},
    {ALINK_DATATYPE_CLASS_STRING, -1, "octet_str", ZCL_DATATYPE_OCTET_STR},
    {ALINK_DATATYPE_CLASS_STRING, -1, "string", ZCL_DATATYPE_CHAR_STR},
    {ALINK_DATATYPE_CLASS_STRING, -1, "long_octet_str", ZCL_DATATYPE_LONG_OCTET_STR},
    {ALINK_DATATYPE_CLASS_STRING, -1, "long_string", ZCL_DATATYPE_LONG_CHAR_STR},
    {ALINK_DATATYPE_CLASS_COLLECTION, -1, "array", ZCL_DATATYPE_ARRAY},        //array类型用json array表示
    {ALINK_DATATYPE_CLASS_STRUCTURE, -1, "struct", ZCL_DATATYPE_STRUCT},      //struct类型用json object表示
    {ALINK_DATATYPE_CLASS_COLLECTION, -1, "set", ZCL_DATATYPE_SET},         //set类型用json array表示
    {ALINK_DATATYPE_CLASS_OTHER, -1, "bag", ZCL_DATATYPE_BAG},
    {ALINK_DATATYPE_CLASS_OTHER, -1, "tdo", ZCL_DATATYPE_TOD},
    {ALINK_DATATYPE_CLASS_OTHER, 4, "date", ZCL_DATATYPE_DATE},             //data类型用10进制时间戳数值表示
    {ALINK_DATATYPE_CLASS_OTHER, 4, "utc", ZCL_DATATYPE_UTC},               //utc类型用10进制时间戳数值表示
    {ALINK_DATATYPE_CLASS_OTHER, 2, "cluster_id", ZCL_DATATYPE_CLUSTER_ID}, //cluster_id类型用小写hex string表示
    {ALINK_DATATYPE_CLASS_OTHER, 2, "attr_id", ZCL_DATATYPE_ATTR_ID},       //attr_id类型用小写hex string表示
    {ALINK_DATATYPE_CLASS_OTHER, 2, "bac_oid", ZCL_DATATYPE_BAC_OID},       //bac_oid类型用小写hex string表示
    {ALINK_DATATYPE_CLASS_OTHER, 8, "ieee_addr", ZCL_DATATYPE_IEEE_ADDR},   //ieee_addr类型用小写hex string表示
    {ALINK_DATATYPE_CLASS_OTHER, 16, "sec_key", ZCL_DATATYPE_128_BIT_SEC_KEY}//sec_key类型用小写hex string表示
};


data_type_t *stdd_get_datatype_by_name(const char *name, int name_len)
{
    int i = 0;
    data_type_t *type = NULL;

    for(i = 0; i < sizeof(g_datatype)/sizeof(g_datatype[0]); i++)
    {
        if(strncmp(g_datatype[i].name, name, name_len) == 0){
            type = &g_datatype[i];
            break;
        }
    }

    return type;
}



data_type_t *stdd_get_datatype_by_id(uint8_t type_id)
{
    int i = 0;
    data_type_t *type = NULL;

    for(i = 0; i < sizeof(g_datatype)/sizeof(g_datatype[0]); i++)
    {
        if(g_datatype[i].id == type_id){
            type = &g_datatype[i];
            break;
        }
    }

    return type;
}





/****************************************************************************
 * id2kernel\rhino\fs\include\kv.h
 *
 *
 * Copyright (C) 2015-2016 The YunOS Open Source Project
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
 *
 ****************************************************************************/
#ifndef KV_H
#define KV_H

/**
 * This function will save string value correspond with the key
 * @param[in]  key    pointer to the name of the value
 * @param[in]  s_value    poniter the name's value
 * @return  the operation status, 0 is OK, -1 is error
 */
int kv_setstring(char *key, const char *s_value);

/**
 * This function will save float value correspond with the key
 * @param[in]  key    pointer to the name of the value
 * @param[in]  f_value    name's value
 * @return  the operation status, 0 is OK, -1 is error
 */
int kv_setfloat(char *key, float f_value);

/**
 * This function will save int value correspond with the key
 * @param[in]  key    pointer to the name of the value
 * @param[in]  i_value    name's value
 * @return  the operation status, 0 is OK, -1 is error
 */
int kv_setint(char *key, int i_value);

/**
 * This function will save binary value correspond with the key
 * @param[in]  key    pointer to the name of the value
 * @param[in]  buf    pinter to the buffer
 * @param[in]  bufsize    size of buffer
 * @return  the operation status, 0 is OK, -1 is error
 */
int kv_setdata(char *key, char *buf, int bufsize);

/**
 * This function will get string value correspond with the key
 * @param[in]  key    pointer to the name of the value
 * @param[out]  s_value    poniter the name's value
 * @param[in]  len    s_value's len
 * @return  the operation status, 0 is OK, -1 is error
 */
int kv_getstring(char *key, char *s_value, int len);

/**
 * This function will get float value correspond with the key
 * @param[in]  key    pointer to the name of the value
 * @param[out]  f_value    poniter the name's value
 * @return  the operation status, 0 is OK, -1 is error
 */
int kv_getfloat(char *key, float *f_value);

/**
 * This function will get int value correspond with the key
 * @param[in]  key    pointer to the name of the value
 * @param[out]  i_value    poniter the name's value
 * @return  the operation status, 0 is OK, -1 is error
 */
int kv_getint(char *key, int *i_value);

/**
 * This function will get binary value correspond with the key
 * @param[in]  key    pointer to the name of the value
 * @param[out]  buf    poniter to the buffer
 * @param[in]  bufsize    size of buffer
 * @return  the operation status, 0 is OK, -1 is error
 */
int kv_getdata(char *key, char *buf, int bufsize);

/**
 * This function will delete correspondding key/value
 * @param[in]  key    pointer to the name of the value
 * @return  the operation status, 0 is OK, -1 is error
 */
int kv_del(char *key);

#endif /* KV_H */

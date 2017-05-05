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

/**
 * @file hal/flash.h
 * @brief Flash HAL API for YOC
 * @version since 5.5.0
 */

#ifndef HAL_FLASH_H
#define HAL_FLASH_H

typedef struct hal_flash_module_s hal_flash_module_t;

struct hal_flash_module_s {
    hal_module_base_t base;

    /**
     * @brief it will call init func of all registered flash modules
     *
     * @return return 0 if init ok, negative value indicates an error
     */
    int (*init)(hal_flash_module_t *m, void *something);

    /**
     * @brief Read some data of configuration from flash,the data is searched by key value
     *
     * @m[in] Refer the flash module which will be used,default module will be used if value is NULL
     * @key[in] The key of the data which must be supplied
     * @buf[in] User data
     * @buf_size[in] User data length
     *
     * @return size of read, <0 for error code
     */
    int (*read_conf)(hal_flash_module_t *m, char *key, unsigned char *buf,
                     int buf_size);

    /**
     * @brief Write some data of configuration to flash,the data must be key value pair
     *
     * @m[in] Refer the flash module which will be used,default module will be used if value is NULL
     * @key[in] The key of the data which must be supplied
     * @buf[in] User data
     * @buf_size[in] User data length
     *
     * @return 0 for success, <0 for error code
     */
    int (*write_conf)(hal_flash_module_t *m, char *key, unsigned char *buf,
                      int buf_size);

    /**
     * @brief Get max flash size reserved for storage
     *
     * @m[in] Refer the flash module which will be used,default module will be used if value is NULL
     *
     * @return size of max flash size used for storage configuration
     */
    int (*get_conf_size)(hal_flash_module_t *m);

    /**
     * @brief reset all the storage data in flash module refered by m
     *
     * @m[in] Refer the flash module which will be used,default module will be used if value is NULL
     *
     * @return 0 for success, <0 for error code
     */
    int (*reset)(hal_flash_module_t *m);
};

/**
 * @brief Write some data of configuration to flash,the data must be key value pair
 *
 * @m[in] Refer the flash module which will be used,default module will be used if value is NULL
 * @key[in] The key of the data which must be supplied
 * @buf[in] User data
 * @buf_size[in] User data length
 *
 * @return 0 for success, <0 for error code
 */
int  hal_flash_conf_write(hal_flash_module_t *m, char *key,
                          unsigned char *buf, int buf_size);

/**
 * @brief Read some data of configuration from flash,the data is searched by key value
 *
 * @m[in] Refer the flash module which will be used,default module will be used if value is NULL
 * @key[in] The key of the data which must be supplied
 * @buf[in] User data
 * @buf_size[in] User data length
 *
 * @return size of read, <0 for error code
 */
int  hal_flash_conf_read(hal_flash_module_t *m, char *key,
                         unsigned char *buf, int buf_size);

/**
 * @brief Get max flash size reserved for storage
 *
 * @m[in] Refer the flash module which will be used,default module will be used if value is NULL
 *
 * @return size of max flash size used for storage configuration
 */
int  hal_flash_conf_get_max_size(hal_flash_module_t *m);

/**
 * @brief Register a new module before HAL startup,it will be added at tail of flash module list
 *
 * @m[in] Refer the flash module which be registered
 */
void hal_flash_register_module(hal_flash_module_t *module);

/**
 * @brief it will call init func of all registered flash modules
 *
 * @return return 0 if init ok, negative value indicates an error
 */
int  hal_flash_init(void);

/**
 * @brief Get the default flash module
 *
 * @return return the first registered flash module ,which is the head of module list
 */
hal_flash_module_t *hal_flash_get_default_module(void);

#endif /* HAL_FLASH_H */


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
 * @file hal/sensor.h
 * @brief SENSOR HAL API for YOC
 * @version since 5.7.0
 */

#ifndef HAL_SENSOR_H
#define HAL_SENSOR_H

#define SENSOR_TYPE_TEMPERATURE 1

typedef struct hal_sensor_module_s hal_sensor_module_t;

typedef int sensor_type;

typedef struct {
    sensor_type type;
    const char *sensor_name;
} sensor_node_t;

struct hal_sensor_module_s {
    hal_module_base_t base;

    /* Link to HW */
    int (*init)(hal_sensor_module_t *m, void *something);
    int (*get_sensor_list)(hal_sensor_module_t *m, sensor_node_t const **list);
    int (*enable)(hal_sensor_module_t *m, sensor_type type);
    int (*disable)(hal_sensor_module_t *m, sensor_type type);
    int (*read)(hal_sensor_module_t *m, sensor_type type, char *buf, int buf_size);
};

/**
 * @brief Arch register a new module before HAL startup
 */
void hal_sensor_register_module(hal_sensor_module_t *module);

/**
 * HAL sensor init .
 *
 * @return
 *     return result, 0 if init success, -1 if fail
 */
int  hal_sensor_init(void);

/**
 * @brief enbale sensor with type
 *
 * @m[in] Refer the sensor module which will be used,default module will be used if value is NULL
 * @list[in] The list of the sensor which will be return
 *
 * @return
 *    return num ot sensor, negative value indicates an error
 */
int  hal_sensor_get_sensor_list(hal_sensor_module_t *m,
                                sensor_node_t const **list);

/**
 * @brief enable sensor with type
 *
 * @m[in] Refer the sensor module which will be used,default module will be used if value is NULL
 * @type[in] The type of the sensor which must be supplied
 *
 * @return
 *    return 0 if enable ok, negative value indicates an error
 */
int  hal_sensor_enable(hal_sensor_module_t *m, sensor_type type);

/**
 * @brief disable sensor with type
 *
 * @m[in] Refer the sensor module which will be used,default module will be used if value is NULL
 * @type[in] The type of the sensor which must be supplied
 *
 * @return
 *    return 0 if disable ok, negative value indicates an error
 */
int  hal_sensor_disable(hal_sensor_module_t *m, sensor_type type);

/**
 * @brief read sensor data when enable sensor
 *
 * @m[in] Refer the sensor module which will be used,default module will be used if value is NULL
 * @type[in] The type of the sensor which must be supplied
 * @buf[in] receive buf for sensor data
 * @buf_size[in] the size buf of input
 * @return
 *     return 0 if read ok, negative value indicates an error
 */
int  hal_sensor_read(hal_sensor_module_t *m, sensor_type type, char *buf,
                     int buf_size);

/**
 * @brief Get the default sensor module
 *
 * @return
 *     return the first registered sensor module ,which is the head of module list
 */
hal_sensor_module_t *hal_sensor_get_default_module(void);

/**
 * Get the next sensor HAL
 *
 * The system may have more than 1 sensor HAL instances.
 *
 * @return
 *     Instance pointer or NULL
 */
hal_sensor_module_t *hal_sensor_get_next_module(hal_sensor_module_t *m);

#endif /* HAL_SENSOR_H */


NAME := sensor

#$(NAME)_TYPE := kernel

$(NAME)_SOURCES += \
        hal/sensor_hal.c \
        hal/sensor_drv_api.c \
        drv/drv_baro_st_lps22hb.c \
        drv/drv_humi_bosch_bme280.c 

$(NAME)_CFLAGS      += -Wall -Werror

GLOBAL_INCLUDES += .
GLOBAL_DEFINES      += AOS_SENSOR

GLOBAL_DEFINES      += AOS_SENSOR_BARO_ST_LPS22HB
#GLOBAL_DEFINES      += AOS_SENSOR_HUMI_BOSCH_BME280




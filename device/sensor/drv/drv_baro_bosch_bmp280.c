/*
* Copyright (C) 2015-2017 Alibaba Group Holding Limited
*
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aos/aos.h>
#include <vfs_conf.h>
#include <vfs_err.h>
#include <vfs_register.h>
#include <hal/base.h>
#include "common.h"
#include "hal/sensor.h"

/* BOSCH BARO SENSOR REGISTER MAP */


/************************************************/
/**\name	CALIBRATION PARAMETERS DEFINITION       */
/***********************************************/
/*calibration parameters */
#define BMP280_TEMPERATURE_CALIB_DIG_T1_LSB_REG     (0x88)
#define BMP280_TEMPERATURE_CALIB_DIG_T1_MSB_REG     (0x89)
#define BMP280_TEMPERATURE_CALIB_DIG_T2_LSB_REG     (0x8A)
#define BMP280_TEMPERATURE_CALIB_DIG_T2_MSB_REG     (0x8B)
#define BMP280_TEMPERATURE_CALIB_DIG_T3_LSB_REG     (0x8C)
#define BMP280_TEMPERATURE_CALIB_DIG_T3_MSB_REG     (0x8D)
#define BMP280_PRESSURE_CALIB_DIG_P1_LSB_REG        (0x8E)
#define BMP280_PRESSURE_CALIB_DIG_P1_MSB_REG        (0x8F)
#define BMP280_PRESSURE_CALIB_DIG_P2_LSB_REG        (0x90)
#define BMP280_PRESSURE_CALIB_DIG_P2_MSB_REG        (0x91)
#define BMP280_PRESSURE_CALIB_DIG_P3_LSB_REG        (0x92)
#define BMP280_PRESSURE_CALIB_DIG_P3_MSB_REG        (0x93)
#define BMP280_PRESSURE_CALIB_DIG_P4_LSB_REG        (0x94)
#define BMP280_PRESSURE_CALIB_DIG_P4_MSB_REG        (0x95)
#define BMP280_PRESSURE_CALIB_DIG_P5_LSB_REG        (0x96)
#define BMP280_PRESSURE_CALIB_DIG_P5_MSB_REG        (0x97)
#define BMP280_PRESSURE_CALIB_DIG_P6_LSB_REG        (0x98)
#define BMP280_PRESSURE_CALIB_DIG_P6_MSB_REG        (0x99)
#define BMP280_PRESSURE_CALIB_DIG_P7_LSB_REG        (0x9A)
#define BMP280_PRESSURE_CALIB_DIG_P7_MSB_REG        (0x9B)
#define BMP280_PRESSURE_CALIB_DIG_P8_LSB_REG        (0x9C)
#define BMP280_PRESSURE_CALIB_DIG_P8_MSB_REG        (0x9D)
#define BMP280_PRESSURE_CALIB_DIG_P9_LSB_REG        (0x9E)
#define BMP280_PRESSURE_CALIB_DIG_P9_MSB_REG        (0x9F)
/************************************************/
/**\name	REGISTER ADDRESS DEFINITION       */
/***********************************************/
#define BMP280_CHIP_ID_REG                   (0xD0)  /*Chip ID Register */
#define BMP280_RST_REG                       (0xE0) /*Softreset Register */
#define BMP280_STAT_REG                      (0xF3)  /*Status Register */
#define BMP280_CTRL_MEAS_REG                 (0xF4)  /*Ctrl Measure Register */
#define BMP280_CONFIG_REG                    (0xF5)  /*Configuration Register */
#define BMP280_PRESSURE_MSB_REG              (0xF7)  /*Pressure MSB Register */
#define BMP280_PRESSURE_LSB_REG              (0xF8)  /*Pressure LSB Register */
#define BMP280_PRESSURE_XLSB_REG             (0xF9)  /*Pressure XLSB Register */
#define BMP280_TEMPERATURE_MSB_REG           (0xFA)  /*Temperature MSB Reg */
#define BMP280_TEMPERATURE_LSB_REG           (0xFB)  /*Temperature LSB Reg */
#define BMP280_TEMPERATURE_XLSB_REG          (0xFC)  /*Temperature XLSB Reg */


/* right shift definitions*/
#define BMP280_SHIFT_BIT_POSITION_BY_01_BIT     (1)
#define BMP280_SHIFT_BIT_POSITION_BY_02_BITS    (2)
#define BMP280_SHIFT_BIT_POSITION_BY_03_BITS    (3)
#define BMP280_SHIFT_BIT_POSITION_BY_04_BITS    (4)
#define BMP280_SHIFT_BIT_POSITION_BY_05_BITS    (5)
#define BMP280_SHIFT_BIT_POSITION_BY_08_BITS    (8)
#define BMP280_SHIFT_BIT_POSITION_BY_11_BITS    (11)
#define BMP280_SHIFT_BIT_POSITION_BY_12_BITS    (12)
#define BMP280_SHIFT_BIT_POSITION_BY_13_BITS    (13)
#define BMP280_SHIFT_BIT_POSITION_BY_14_BITS    (14)
#define BMP280_SHIFT_BIT_POSITION_BY_15_BITS    (15)
#define BMP280_SHIFT_BIT_POSITION_BY_16_BITS    (16)
#define BMP280_SHIFT_BIT_POSITION_BY_17_BITS    (17)
#define BMP280_SHIFT_BIT_POSITION_BY_18_BITS    (18)
#define BMP280_SHIFT_BIT_POSITION_BY_19_BITS    (19)
#define BMP280_SHIFT_BIT_POSITION_BY_25_BITS    (25)
#define BMP280_SHIFT_BIT_POSITION_BY_31_BITS    (31)
#define BMP280_SHIFT_BIT_POSITION_BY_33_BITS    (33)
#define BMP280_SHIFT_BIT_POSITION_BY_35_BITS    (35)
#define BMP280_SHIFT_BIT_POSITION_BY_47_BITS    (47)

/* numeric definitions */
#define	BMP280_PRESSURE_TEMPERATURE_CALIB_DATA_LENGTH   (24)
#define	BMP280_GEN_READ_WRITE_DATA_LENGTH               (1)
#define BMP280_REGISTER_READ_DELAY				        (1)
#define	BMP280_TEMPERATURE_DATA_LENGTH				    (3)
#define	BMP280_PRESSURE_DATA_LENGTH				        (3)
#define	BMP280_ALL_DATA_FRAME_LENGTH				    (6)
#define	BMP280_INIT_VALUE					            (0)
#define	BMP280_CHIP_ID_READ_COUNT				        (5)
#define	BMP280_CHIP_ID_READ_SUCCESS				        (0)
#define	BMP280_CHIP_ID_READ_FAIL				        ((int8_t)-1)
#define	BMP280_INVALID_DATA					            (0)



/****************************************************/
/**\name	DEFINITIONS FOR ARRAY SIZE OF DATA   */
/***************************************************/
#define	BMP280_TEMPERATURE_DATA_SIZE    (3)
#define	BMP280_PRESSURE_DATA_SIZE       (3)
#define	BMP280_DATA_FRAME_SIZE          (6)
#define	BMP280_CALIB_DATA_SIZE          (24)

#define	BMP280_TEMPERATURE_MSB_DATA		(0)
#define	BMP280_TEMPERATURE_LSB_DATA		(1)
#define	BMP280_TEMPERATURE_XLSB_DATA    (2)


/************************************************/
/**\name	I2C ADDRESS DEFINITION       */
/***********************************************/
#define BMP280_I2C_ADDRESS1     (0x76)
#define BMP280_I2C_ADDRESS2     (0x77)


#define BMP280_SLEEP_MODE       (0x00)
#define BMP280_FORCED_MODE      (0x01)
#define BMP280_NORMAL_MODE      (0x03)
#define BMP280_SOFT_RESET_CODE  (0xB6)


/************************************************/
/**\name	STANDBY TIME DEFINITION       */
/***********************************************/
#define BMP280_STANDBY_TIME_1_MS              (0x00)
#define BMP280_STANDBY_TIME_63_MS             (0x01)
#define BMP280_STANDBY_TIME_125_MS            (0x02)
#define BMP280_STANDBY_TIME_250_MS            (0x03)
#define BMP280_STANDBY_TIME_500_MS            (0x04)
#define BMP280_STANDBY_TIME_1000_MS           (0x05)
#define BMP280_STANDBY_TIME_2000_MS           (0x06)
#define BMP280_STANDBY_TIME_4000_MS           (0x07)
/************************************************/
/**\name	OVERSAMPLING DEFINITION       */
/***********************************************/
#define BMP280_OVERSAMP_SKIPPED     (0x00)
#define BMP280_OVERSAMP_1X          (0x01)
#define BMP280_OVERSAMP_2X          (0x02)
#define BMP280_OVERSAMP_4X          (0x03)
#define BMP280_OVERSAMP_8X          (0x04)
#define BMP280_OVERSAMP_16X         (0x05)

#define BMP280_FILTER_COEFF_OFF     (0x00)
#define BMP280_FILTER_COEFF_2       (0x01)
#define BMP280_FILTER_COEFF_4       (0x02)
#define BMP280_FILTER_COEFF_8       (0x03)
#define BMP280_FILTER_COEFF_16      (0x04)

/************************************************/
/**\name	WORKING MODE DEFINITION       */
/***********************************************/
#define BMP280_ULTRA_LOW_POWER_MODE          (0x00)
#define BMP280_LOW_POWER_MODE	             (0x01)
#define BMP280_STANDARD_RESOLUTION_MODE      (0x02)
#define BMP280_HIGH_RESOLUTION_MODE          (0x03)
#define BMP280_ULTRA_HIGH_RESOLUTION_MODE    (0x04)

#define BMP280_ULTRALOWPOWER_OVERSAMP_PRESSURE          BMP280_OVERSAMP_1X
#define BMP280_ULTRALOWPOWER_OVERSAMP_TEMPERATURE       BMP280_OVERSAMP_1X

#define BMP280_LOWPOWER_OVERSAMP_PRESSURE	            BMP280_OVERSAMP_2X
#define BMP280_LOWPOWER_OVERSAMP_TEMPERATURE	        BMP280_OVERSAMP_1X

#define BMP280_STANDARDRESOLUTION_OVERSAMP_PRESSURE     BMP280_OVERSAMP_4X
#define BMP280_STANDARDRESOLUTION_OVERSAMP_TEMPERATURE  BMP280_OVERSAMP_1X

#define BMP280_HIGHRESOLUTION_OVERSAMP_PRESSURE         BMP280_OVERSAMP_8X
#define BMP280_HIGHRESOLUTION_OVERSAMP_TEMPERATURE      BMP280_OVERSAMP_1X

#define BMP280_ULTRAHIGHRESOLUTION_OVERSAMP_PRESSURE    BMP280_OVERSAMP_16X
#define BMP280_ULTRAHIGHRESOLUTION_OVERSAMP_TEMPERATURE BMP280_OVERSAMP_2X


/************************************************/
/**\name	BIT LENGTH,POSITION AND MASK DEFINITION      */
/***********************************************/
/* Status Register */
#define BMP280_STATUS_REG_MEASURING__POS           (3)
#define BMP280_STATUS_REG_MEASURING__MSK           (0x08)
#define BMP280_STATUS_REG_MEASURING__LEN           (1)
#define BMP280_STATUS_REG_MEASURING__REG           (BMP280_STAT_REG)

#define BMP280_STATUS_REG_IM_UPDATE__POS            (0)
#define BMP280_STATUS_REG_IM_UPDATE__MSK            (0x01)
#define BMP280_STATUS_REG_IM_UPDATE__LEN            (1)
#define BMP280_STATUS_REG_IM_UPDATE__REG           (BMP280_STAT_REG)
/************************************************/
/**\name	BIT LENGTH,POSITION AND MASK DEFINITION
FOR TEMPERATURE OVERSAMPLING */
/***********************************************/
/* Control Measurement Register */
#define BMP280_CTRL_MEAS_REG_OVERSAMP_TEMPERATURE__POS             (5)
#define BMP280_CTRL_MEAS_REG_OVERSAMP_TEMPERATURE__MSK             (0xE0)
#define BMP280_CTRL_MEAS_REG_OVERSAMP_TEMPERATURE__LEN             (3)
#define BMP280_CTRL_MEAS_REG_OVERSAMP_TEMPERATURE__REG             \
(BMP280_CTRL_MEAS_REG)
/************************************************/
/**\name	BIT LENGTH,POSITION AND MASK DEFINITION
FOR PRESSURE OVERSAMPLING */
/***********************************************/
#define BMP280_CTRL_MEAS_REG_OVERSAMP_PRESSURE__POS             (2)
#define BMP280_CTRL_MEAS_REG_OVERSAMP_PRESSURE__MSK             (0x1C)
#define BMP280_CTRL_MEAS_REG_OVERSAMP_PRESSURE__LEN             (3)
#define BMP280_CTRL_MEAS_REG_OVERSAMP_PRESSURE__REG             \
(BMP280_CTRL_MEAS_REG)
/************************************************/
/**\name	BIT LENGTH,POSITION AND MASK DEFINITION
FOR POWER MODE */
/***********************************************/
#define BMP280_CTRL_MEAS_REG_POWER_MODE__POS              (0)
#define BMP280_CTRL_MEAS_REG_POWER_MODE__MSK              (0x03)
#define BMP280_CTRL_MEAS_REG_POWER_MODE__LEN              (2)
#define BMP280_CTRL_MEAS_REG_POWER_MODE__REG             (BMP280_CTRL_MEAS_REG)
/************************************************/
/**\name	BIT LENGTH,POSITION AND MASK DEFINITION
FOR STANDBY DURATION */
/***********************************************/
/* Configuration Register */
#define BMP280_CONFIG_REG_STANDBY_DURN__POS                 (5)
#define BMP280_CONFIG_REG_STANDBY_DURN__MSK                 (0xE0)
#define BMP280_CONFIG_REG_STANDBY_DURN__LEN                 (3)
#define BMP280_CONFIG_REG_STANDBY_DURN__REG                 (BMP280_CONFIG_REG)
/************************************************/
/**\name	BIT LENGTH,POSITION AND MASK DEFINITION
FOR IIR FILTER */
/***********************************************/
#define BMP280_CONFIG_REG_FILTER__POS              (2)
#define BMP280_CONFIG_REG_FILTER__MSK              (0x1C)
#define BMP280_CONFIG_REG_FILTER__LEN              (3)
#define BMP280_CONFIG_REG_FILTER__REG              (BMP280_CONFIG_REG)
/************************************************/
/**\name	BIT LENGTH,POSITION AND MASK DEFINITION
FOR SPI ENABLE*/
/***********************************************/
#define BMP280_CONFIG_REG_SPI3_ENABLE__POS             (0)
#define BMP280_CONFIG_REG_SPI3_ENABLE__MSK             (0x01)
#define BMP280_CONFIG_REG_SPI3_ENABLE__LEN             (1)
#define BMP280_CONFIG_REG_SPI3_ENABLE__REG             (BMP280_CONFIG_REG)
/************************************************/
/**\name	BIT LENGTH,POSITION AND MASK DEFINITION
FOR PRESSURE AND TEMPERATURE DATA REGISTERS */
/***********************************************/
/* Data Register */
#define BMP280_PRESSURE_XLSB_REG_DATA__POS         (4)
#define BMP280_PRESSURE_XLSB_REG_DATA__MSK         (0xF0)
#define BMP280_PRESSURE_XLSB_REG_DATA__LEN         (4)
#define BMP280_PRESSURE_XLSB_REG_DATA__REG         (BMP280_PRESSURE_XLSB_REG)

#define BMP280_TEMPERATURE_XLSB_REG_DATA__POS      (4)
#define BMP280_TEMPERATURE_XLSB_REG_DATA__MSK      (0xF0)
#define BMP280_TEMPERATURE_XLSB_REG_DATA__LEN      (4)
#define BMP280_TEMPERATURE_XLSB_REG_DATA__REG      (BMP280_TEMPERATURE_XLSB_REG)

#define	BMP280_TEMPERATURE_DATA_SIZE		(3)
#define	BMP280_PRESSURE_DATA_SIZE		(3)
#define	BMP280_DATA_FRAME_SIZE			(6)
#define	BMP280_CALIB_DATA_SIZE			(24)

#define	BMP280_TEMPERATURE_MSB_DATA		(0)
#define	BMP280_TEMPERATURE_LSB_DATA		(1)
#define	BMP280_TEMPERATURE_XLSB_DATA		(2)

#define	BMP280_PRESSURE_MSB_DATA		(0)
#define	BMP280_PRESSURE_LSB_DATA		(1)
#define	BMP280_PRESSURE_XLSB_DATA		(2)

#define	BMP280_DATA_FRAME_PRESSURE_MSB_BYTE	(0)
#define	BMP280_DATA_FRAME_PRESSURE_LSB_BYTE	(1)
#define	BMP280_DATA_FRAME_PRESSURE_XLSB_BYTE	(2)
#define	BMP280_DATA_FRAME_TEMPERATURE_MSB_BYTE	(3)
#define	BMP280_DATA_FRAME_TEMPERATURE_LSB_BYTE	(4)
#define	BMP280_DATA_FRAME_TEMPERATURE_XLSB_BYTE	(5)


/****************************************************/
/**\name	ARRAY PARAMETER FOR CALIBRATION     */
/***************************************************/
#define	BMP280_TEMPERATURE_CALIB_DIG_T1_LSB	(0)
#define	BMP280_TEMPERATURE_CALIB_DIG_T1_MSB	(1)
#define	BMP280_TEMPERATURE_CALIB_DIG_T2_LSB	(2)
#define	BMP280_TEMPERATURE_CALIB_DIG_T2_MSB	(3)
#define	BMP280_TEMPERATURE_CALIB_DIG_T3_LSB	(4)
#define	BMP280_TEMPERATURE_CALIB_DIG_T3_MSB	(5)
#define	BMP280_PRESSURE_CALIB_DIG_P1_LSB	(6)
#define	BMP280_PRESSURE_CALIB_DIG_P1_MSB	(7)
#define	BMP280_PRESSURE_CALIB_DIG_P2_LSB	(8)
#define	BMP280_PRESSURE_CALIB_DIG_P2_MSB	(9)
#define	BMP280_PRESSURE_CALIB_DIG_P3_LSB	(10)
#define	BMP280_PRESSURE_CALIB_DIG_P3_MSB	(11)
#define	BMP280_PRESSURE_CALIB_DIG_P4_LSB	(12)
#define	BMP280_PRESSURE_CALIB_DIG_P4_MSB	(13)
#define	BMP280_PRESSURE_CALIB_DIG_P5_LSB	(14)
#define	BMP280_PRESSURE_CALIB_DIG_P5_MSB	(15)
#define	BMP280_PRESSURE_CALIB_DIG_P6_LSB	(16)
#define	BMP280_PRESSURE_CALIB_DIG_P6_MSB	(17)
#define	BMP280_PRESSURE_CALIB_DIG_P7_LSB	(18)
#define	BMP280_PRESSURE_CALIB_DIG_P7_MSB	(19)
#define	BMP280_PRESSURE_CALIB_DIG_P8_LSB	(20)
#define	BMP280_PRESSURE_CALIB_DIG_P8_MSB	(21)
#define	BMP280_PRESSURE_CALIB_DIG_P9_LSB	(22)
#define	BMP280_PRESSURE_CALIB_DIG_P9_MSB	(23)

#define BMP280_SOFT_RESRT_VALUE             (0XB6)



/***************************************************************/
/**\name	GET AND SET BITSLICE FUNCTIONS       */
/***************************************************************/

#define BMP280_GET_BITSLICE(regvar, bitname)\
	((regvar & bitname##__MSK) >> bitname##__POS)

#define BMP280_SET_BITSLICE(regvar, bitname, val)\
	((regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK))


#define BMP280_BIT(x)                        ((uint8_t)(x))

#define BMP280_CHIP_ID_VAL                   BMP280_BIT(0X58)




typedef enum {
  BMP280_ODR_25HZ       = (uint8_t)0x00,         /*!< Output Data Rate: 150Hz    */
  BMP280_ODR_10HZ       = (uint8_t)0x01,         /*!< Output Data Rate: 10Hz      */
  BMP280_ODR_5HZ        = (uint8_t)0x02,         /*!< Output Data Rate:   5Hz      */
  BMP280_ODR_3HZ        = (uint8_t)0x03,         /*!< Output Data Rate:   2Hz      */
  BMP280_ODR_1HZ        = (uint8_t)0x04,         /*!< Output Data Rate:   1Hz      */
  BMP280_ODR_ONE_SHOT   = (uint8_t)0x10,         /*!< Output Data Rate: one shot */
} bmp280_odr_e;


typedef struct bmp280_calib_param_t {
	uint16_t    dig_T1;
	int16_t     dig_T2;
	int16_t     dig_T3;
	uint16_t    dig_P1;
	int16_t     dig_P2;
	int16_t     dig_P3;
	int16_t     dig_P4;
	int16_t     dig_P5;
	int16_t     dig_P6;
	int16_t     dig_P7;
	int16_t     dig_P8;
	int16_t     dig_P9;
	int         t_fine;/**<calibration t_fine data*/
}bmp280_calib_param_t;

typedef struct bmp280_device_cfg_t {
    bmp280_odr_e    odr;
    uint8_t         mode_filter;
    uint8_t         mode_baro;
    uint8_t         mode_temp;
    uint8_t         mode_power;
	uint8_t         oversamp_temp;
	uint8_t         oversamp_baro;
}bmp280_device_cfg_t;


static bmp280_device_cfg_t    g_bmp280_dev_cfg;
static bmp280_calib_param_t   g_bmp280_calib_table;


i2c_dev_t bmp280_ctx = {
    .port = 1,
    .config.address_width = 7,
    .config.freq = 400000,
    .config.dev_addr = 0x77,
};


static int  drv_baro_bosch_bmp280_get_calib_param(i2c_dev_t* drv)
{
    int     ret = 0;
    uint8_t a_data_u8[BMP280_CALIB_DATA_SIZE] = {BMP280_INIT_VALUE,
    BMP280_INIT_VALUE, BMP280_INIT_VALUE, BMP280_INIT_VALUE,
    BMP280_INIT_VALUE, BMP280_INIT_VALUE, BMP280_INIT_VALUE,
    BMP280_INIT_VALUE, BMP280_INIT_VALUE, BMP280_INIT_VALUE,
    BMP280_INIT_VALUE, BMP280_INIT_VALUE, BMP280_INIT_VALUE,
    BMP280_INIT_VALUE, BMP280_INIT_VALUE, BMP280_INIT_VALUE,
    BMP280_INIT_VALUE, BMP280_INIT_VALUE, BMP280_INIT_VALUE,
    BMP280_INIT_VALUE, BMP280_INIT_VALUE, BMP280_INIT_VALUE,
    BMP280_INIT_VALUE, BMP280_INIT_VALUE};


    ret = sensor_i2c_read(drv,BMP280_TEMPERATURE_CALIB_DIG_T1_LSB_REG,
        a_data_u8,BMP280_PRESSURE_TEMPERATURE_CALIB_DATA_LENGTH,I2C_OP_RETRIES);
    
    if(unlikely(ret)){
        return ret;
    }
    
    /* read calibration values*/
    g_bmp280_calib_table.dig_T1 = (uint16_t)((((uint16_t)((uint8_t)a_data_u8[BMP280_TEMPERATURE_CALIB_DIG_T1_MSB]))
            << BMP280_SHIFT_BIT_POSITION_BY_08_BITS) | a_data_u8[BMP280_TEMPERATURE_CALIB_DIG_T1_LSB]);
    g_bmp280_calib_table.dig_T2 = (int16_t)((((int16_t)((int8_t)a_data_u8[BMP280_TEMPERATURE_CALIB_DIG_T2_MSB]))
            << BMP280_SHIFT_BIT_POSITION_BY_08_BITS) | a_data_u8[BMP280_TEMPERATURE_CALIB_DIG_T2_LSB]);
    g_bmp280_calib_table.dig_T3 = (int16_t)((((int16_t)((int8_t)a_data_u8[BMP280_TEMPERATURE_CALIB_DIG_T3_MSB]))
            << BMP280_SHIFT_BIT_POSITION_BY_08_BITS) | a_data_u8[BMP280_TEMPERATURE_CALIB_DIG_T3_LSB]);
    g_bmp280_calib_table.dig_P1 = (uint16_t)((((uint16_t)((uint8_t)a_data_u8[BMP280_PRESSURE_CALIB_DIG_P1_MSB]))
            << BMP280_SHIFT_BIT_POSITION_BY_08_BITS) | a_data_u8[BMP280_PRESSURE_CALIB_DIG_P1_LSB]);
    g_bmp280_calib_table.dig_P2 = (int16_t)((((int16_t)((int8_t)a_data_u8[BMP280_PRESSURE_CALIB_DIG_P2_MSB]))
            << BMP280_SHIFT_BIT_POSITION_BY_08_BITS) | a_data_u8[BMP280_PRESSURE_CALIB_DIG_P2_LSB]);
    g_bmp280_calib_table.dig_P3 = (int16_t)((((int16_t)((int8_t)a_data_u8[BMP280_PRESSURE_CALIB_DIG_P3_MSB]))
            << BMP280_SHIFT_BIT_POSITION_BY_08_BITS) | a_data_u8[BMP280_PRESSURE_CALIB_DIG_P3_LSB]);
    g_bmp280_calib_table.dig_P4 = (int16_t)((((int16_t)((int8_t)a_data_u8[BMP280_PRESSURE_CALIB_DIG_P4_MSB]))
            << BMP280_SHIFT_BIT_POSITION_BY_08_BITS) | a_data_u8[BMP280_PRESSURE_CALIB_DIG_P4_LSB]);
    g_bmp280_calib_table.dig_P5 = (int16_t)((((int16_t)((int8_t)a_data_u8[BMP280_PRESSURE_CALIB_DIG_P5_MSB]))
            << BMP280_SHIFT_BIT_POSITION_BY_08_BITS) | a_data_u8[BMP280_PRESSURE_CALIB_DIG_P5_LSB]);
    g_bmp280_calib_table.dig_P6 = (int16_t)((((int16_t)((int8_t)a_data_u8[BMP280_PRESSURE_CALIB_DIG_P6_MSB]))
            << BMP280_SHIFT_BIT_POSITION_BY_08_BITS) | a_data_u8[BMP280_PRESSURE_CALIB_DIG_P6_LSB]);
    g_bmp280_calib_table.dig_P7 = (int16_t)((((int16_t)((int8_t)a_data_u8[BMP280_PRESSURE_CALIB_DIG_P7_MSB]))
            << BMP280_SHIFT_BIT_POSITION_BY_08_BITS) | a_data_u8[BMP280_PRESSURE_CALIB_DIG_P7_LSB]);
    g_bmp280_calib_table.dig_P8 = (int16_t)((((int16_t)((int8_t)a_data_u8[BMP280_PRESSURE_CALIB_DIG_P8_MSB]))
            << BMP280_SHIFT_BIT_POSITION_BY_08_BITS) | a_data_u8[BMP280_PRESSURE_CALIB_DIG_P8_LSB]);
    g_bmp280_calib_table.dig_P9 = (int16_t)((((int16_t)((int8_t)a_data_u8[BMP280_PRESSURE_CALIB_DIG_P9_MSB]))
            << BMP280_SHIFT_BIT_POSITION_BY_08_BITS) | a_data_u8[BMP280_PRESSURE_CALIB_DIG_P9_LSB]);

    return ret;
}




static int drv_baro_bosch_bmp280_get_oversamp_temp(i2c_dev_t* drv, uint8_t *value)
{
	int     ret = 0;
	uint8_t   v_data_u8 = 0;

    ret = sensor_i2c_read(drv,BMP280_CTRL_MEAS_REG_OVERSAMP_TEMPERATURE__REG,&v_data_u8,I2C_DATA_LEN,I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }

	*value = BMP280_GET_BITSLICE(v_data_u8,
			BMP280_CTRL_MEAS_REG_OVERSAMP_TEMPERATURE);
   	g_bmp280_dev_cfg.oversamp_temp = *value;

	return ret;
}

static int  drv_baro_bosch_bmp280_set_oversamp_temp(i2c_dev_t* drv, uint8_t value)
{
	int     ret = 0;
	uint8_t v_data_u8 = 0;

    ret = sensor_i2c_read(drv,BMP280_CTRL_MEAS_REG_OVERSAMP_TEMPERATURE__REG,
                &v_data_u8,I2C_DATA_LEN,I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }

	v_data_u8 = BMP280_SET_BITSLICE(v_data_u8,BMP280_CTRL_MEAS_REG_OVERSAMP_TEMPERATURE,value);
	ret = sensor_i2c_write(drv,BMP280_CTRL_MEAS_REG_OVERSAMP_TEMPERATURE__REG,
                &v_data_u8,I2C_DATA_LEN,I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
    g_bmp280_dev_cfg.oversamp_temp = value;
    
	return ret;
}



static int drv_baro_bosch_bmp280_get_oversamp_baro(i2c_dev_t* drv, uint8_t *value)
{
	int     ret = 0;
	uint8_t   v_data_u8 = 0;

    ret = sensor_i2c_read(drv,BMP280_CTRL_MEAS_REG_OVERSAMP_PRESSURE__REG,&v_data_u8,I2C_DATA_LEN,I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }

	*value = BMP280_GET_BITSLICE(v_data_u8,BMP280_CTRL_MEAS_REG_OVERSAMP_PRESSURE);
    
	g_bmp280_dev_cfg.oversamp_baro = *value;

	return ret;
}

static int  drv_baro_bosch_bmp280_set_oversamp_baro(i2c_dev_t* drv, uint8_t value)
{
	int     ret = 0;
	uint8_t   v_data_u8 = 0;

    ret = sensor_i2c_read(drv,BMP280_CTRL_MEAS_REG_OVERSAMP_PRESSURE__REG,
                            &v_data_u8,I2C_DATA_LEN,I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }

	v_data_u8 = BMP280_SET_BITSLICE(v_data_u8,BMP280_CTRL_MEAS_REG_OVERSAMP_PRESSURE,value);
	ret = sensor_i2c_write(drv,BMP280_CTRL_MEAS_REG_OVERSAMP_PRESSURE__REG,
                            &v_data_u8,I2C_DATA_LEN,I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
    g_bmp280_dev_cfg.oversamp_baro = value;
    
	return ret;
}


static int drv_baro_bosch_bmp280_get_filter(i2c_dev_t* drv, uint8_t *value)
{
	int     ret = 0;
	uint8_t   v_data_u8 = 0;

    ret = sensor_i2c_read(drv,BMP280_CONFIG_REG_FILTER__REG,&v_data_u8,I2C_DATA_LEN,I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }

	*value = BMP280_GET_BITSLICE(v_data_u8,BMP280_CONFIG_REG_FILTER);
    
   	g_bmp280_dev_cfg.mode_filter = *value;

	return ret;
}

static int  drv_baro_bosch_bmp280_set_filter(i2c_dev_t* drv, uint8_t value)
{
	int     ret = 0;
	uint8_t   v_data_u8 = 0;

    ret = sensor_i2c_read(drv,BMP280_CONFIG_REG_FILTER__REG,
                            &v_data_u8,I2C_DATA_LEN,I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }

	v_data_u8 = BMP280_SET_BITSLICE(v_data_u8,BMP280_CONFIG_REG_FILTER,value);
	ret = sensor_i2c_write(drv,BMP280_CONFIG_REG_FILTER__REG,
                            &v_data_u8,I2C_DATA_LEN,I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
    
    g_bmp280_dev_cfg.mode_filter = value;
	return ret;
}


static int drv_baro_bosch_bmp280_get_standby(i2c_dev_t* drv, uint8_t *value)
{
	int     ret = 0;
	uint8_t   v_data_u8 = 0;

    ret = sensor_i2c_read(drv,BMP280_CONFIG_REG_STANDBY_DURN__REG,
                            &v_data_u8,I2C_DATA_LEN,I2C_OP_RETRIES);
    
    if(unlikely(ret)){
        return ret;
    }
    
	*value = BMP280_GET_BITSLICE(v_data_u8,BMP280_CONFIG_REG_STANDBY_DURN);
    
    g_bmp280_dev_cfg.odr = *value;
	return ret;
}

static int  drv_baro_bosch_bmp280_set_standby(i2c_dev_t* drv, uint8_t value)
{
	int     ret = 0;
	uint8_t   v_data_u8 = 0;

    ret = sensor_i2c_read(drv,BMP280_CONFIG_REG_STANDBY_DURN__REG,
                            &v_data_u8,I2C_DATA_LEN,I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }

	v_data_u8 = BMP280_SET_BITSLICE(v_data_u8,BMP280_CONFIG_REG_STANDBY_DURN,value);
	ret = sensor_i2c_write(drv,BMP280_CONFIG_REG_STANDBY_DURN__REG,
                            &v_data_u8,I2C_DATA_LEN,I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
    
    g_bmp280_dev_cfg.odr = value;
	return ret;
}





static int drv_baro_bosch_bmp280_validate_id(i2c_dev_t* drv, uint8_t id_value)
{
    uint8_t value = 0;
    int ret = 0;

    if(drv == NULL){
        return -1;
    }
    
    ret = sensor_i2c_read(drv, BMP280_CHIP_ID_REG, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
    
    if (id_value != value){
        return -1;
    }
    return 0;
}

static int drv_baro_bosch_bmp280_set_power_mode(i2c_dev_t* drv, dev_power_mode_e mode)
{
    uint8_t value = 0x00;
    uint8_t dev_mode;
    int ret = 0;

    
    switch(mode){
        case DEV_POWER_OFF:
        case DEV_SLEEP:{
            dev_mode = (uint8_t)BMP280_SLEEP_MODE;
            break;
            }
        case DEV_POWER_ON:{
            dev_mode = (uint8_t)BMP280_NORMAL_MODE;
            break;
            }
        
        default:return -1;
    }

    
    ret = sensor_i2c_read(drv, BMP280_CTRL_MEAS_REG, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
    
    //value  = (UINT8)dev_mode | ((g_bmp280_dev_cfg.oversamp_temp << BMP280_SHIFT_BIT_POSITION_BY_05_BITS) 
    //         | (g_bmp280_dev_cfg.oversamp_baro  << BMP280_SHIFT_BIT_POSITION_BY_02_BITS));

    value |= (uint8_t)dev_mode;
    
    ret = sensor_i2c_write(drv, BMP280_CTRL_MEAS_REG, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
    
    g_bmp280_dev_cfg.mode_power = dev_mode;
    return 0;
}



static bmp280_odr_e drv_baro_bosch_bmp280_hz2odr(int hz)
{
    if(hz > 10)
        return BMP280_ODR_25HZ;
    else if(hz > 5)
        return BMP280_ODR_10HZ;
    else if(hz > 3)
        return BMP280_ODR_5HZ;
    else if(hz > 1)
        return BMP280_ODR_3HZ;
    else
        return BMP280_ODR_1HZ;
}


static int drv_baro_bosch_bmp280_set_odr(i2c_dev_t* drv, bmp280_odr_e odr)
{
    int ret;
    if(odr >= BMP280_ODR_ONE_SHOT)
    {
        return -1;
    }

    ret = drv_baro_bosch_bmp280_set_standby(drv,odr);
   
    return ret;
}


static int  drv_baro_bosch_bmp280_soft_reset(i2c_dev_t* drv)
{
	int     ret = 0;
	uint8_t   v_data_u8 = BMP280_SOFT_RESRT_VALUE;
    
	ret = sensor_i2c_write(drv,BMP280_CONFIG_REG_STANDBY_DURN__REG,
                            &v_data_u8,I2C_DATA_LEN,I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
	return ret;
}




static int drv_baro_bosch_bmp280_set_default_config(i2c_dev_t* drv)
{
    uint8_t value = 0x00;
    int ret = 0;
    ret = drv_baro_bosch_bmp280_set_power_mode(drv, DEV_SLEEP);
    if(unlikely(ret)){
        return ret;
    }

    ret = drv_baro_bosch_bmp280_set_odr(drv, BMP280_ODR_25HZ);
    if(unlikely(ret)){
        return ret;
    }

    ret = drv_baro_bosch_bmp280_set_oversamp_temp(drv, BMP280_OVERSAMP_SKIPPED);
    if(unlikely(ret)){
        return ret;
    }
    
    ret = drv_baro_bosch_bmp280_set_oversamp_baro(drv, BMP280_ULTRA_HIGH_RESOLUTION_MODE);
    if(unlikely(ret)){
        return ret;
    }
    
    ret = drv_baro_bosch_bmp280_set_filter(drv, BMP280_FILTER_COEFF_16);
    if(unlikely(ret)){
        return ret;
    }
    
    return 0;
}




static int bmp280_read_uncomp_baro(i2c_dev_t* drv, int *uncomp_baro)
{
	int ret = 0;
    
	uint8_t a_data_u8[BMP280_PRESSURE_DATA_SIZE] = {0, 0, 0};

    ret = sensor_i2c_read(drv, BMP280_PRESSURE_MSB_REG, a_data_u8, BMP280_PRESSURE_DATA_LENGTH, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
    
	*uncomp_baro = (int)((((uint32_t)(a_data_u8[BMP280_PRESSURE_MSB_DATA]))<< BMP280_SHIFT_BIT_POSITION_BY_12_BITS)
			| (((uint32_t)(a_data_u8[BMP280_PRESSURE_LSB_DATA]))<< BMP280_SHIFT_BIT_POSITION_BY_04_BITS)
			| ((uint32_t)a_data_u8[BMP280_PRESSURE_XLSB_DATA]>> BMP280_SHIFT_BIT_POSITION_BY_04_BITS));
	return ret;
}


uint32_t bmp280_compensate_baro(int uncomp_baro)
{
	int v_x1_u32r = 0;
	int v_x2_u32r = 0;
	uint32_t comp_baro = 0;
    
	/* calculate x1*/
	v_x1_u32r = (((int)g_bmp280_calib_table.t_fine)
			>> BMP280_SHIFT_BIT_POSITION_BY_01_BIT) - (int)64000;
	/* calculate x2*/
	v_x2_u32r = (((v_x1_u32r >> BMP280_SHIFT_BIT_POSITION_BY_02_BITS)
			* (v_x1_u32r >> BMP280_SHIFT_BIT_POSITION_BY_02_BITS))
			>> BMP280_SHIFT_BIT_POSITION_BY_11_BITS)
			* ((int)g_bmp280_calib_table.dig_P6);
	v_x2_u32r = v_x2_u32r + ((v_x1_u32r *
			((int)g_bmp280_calib_table.dig_P5))
			<< BMP280_SHIFT_BIT_POSITION_BY_01_BIT);
	v_x2_u32r = (v_x2_u32r >> BMP280_SHIFT_BIT_POSITION_BY_02_BITS)
			+ (((int)g_bmp280_calib_table.dig_P4)
			<< BMP280_SHIFT_BIT_POSITION_BY_16_BITS);
	/* calculate x1*/
	v_x1_u32r = (((g_bmp280_calib_table.dig_P3
			* (((v_x1_u32r
			>> BMP280_SHIFT_BIT_POSITION_BY_02_BITS) * (v_x1_u32r
			>> BMP280_SHIFT_BIT_POSITION_BY_02_BITS))
			>> BMP280_SHIFT_BIT_POSITION_BY_13_BITS))
			>> BMP280_SHIFT_BIT_POSITION_BY_03_BITS)
			+ ((((int)g_bmp280_calib_table.dig_P2)
			* v_x1_u32r)
			>> BMP280_SHIFT_BIT_POSITION_BY_01_BIT))
			>> BMP280_SHIFT_BIT_POSITION_BY_18_BITS;
	v_x1_u32r = ((((32768 + v_x1_u32r))
			* ((int)g_bmp280_calib_table.dig_P1))
			>> BMP280_SHIFT_BIT_POSITION_BY_15_BITS);
	/* calculate pressure*/
	comp_baro = (((uint32_t)(((int)1048576) - uncomp_baro)
			- (v_x2_u32r >> BMP280_SHIFT_BIT_POSITION_BY_12_BITS)))
			* 3125;
	/* check overflow*/
	if (comp_baro < 0x80000000)
		/* Avoid exception caused by division by zero */
		if (v_x1_u32r != BMP280_INIT_VALUE)
			comp_baro = (comp_baro
					<< BMP280_SHIFT_BIT_POSITION_BY_01_BIT)
					/ ((uint32_t)v_x1_u32r);
		else
			return BMP280_INVALID_DATA;
	else
	/* Avoid exception caused by division by zero */
	if (v_x1_u32r != BMP280_INIT_VALUE)
		comp_baro = (comp_baro / (uint32_t)v_x1_u32r) * 2;
	else
		return BMP280_INVALID_DATA;
	/* calculate x1*/
	v_x1_u32r = (((int)g_bmp280_calib_table.dig_P9) * ((int)(
			((comp_baro
			>> BMP280_SHIFT_BIT_POSITION_BY_03_BITS)
			* (comp_baro
			>> BMP280_SHIFT_BIT_POSITION_BY_03_BITS))
			>> BMP280_SHIFT_BIT_POSITION_BY_13_BITS)))
			>> BMP280_SHIFT_BIT_POSITION_BY_12_BITS;
	/* calculate x2*/
	v_x2_u32r = (((int)(comp_baro >>
			BMP280_SHIFT_BIT_POSITION_BY_02_BITS))
			* ((int)g_bmp280_calib_table.dig_P8))
			>> BMP280_SHIFT_BIT_POSITION_BY_13_BITS;
	/* calculate true pressure*/
	comp_baro = (uint32_t)((int)comp_baro + ((v_x1_u32r + v_x2_u32r
			+ g_bmp280_calib_table.dig_P7)
			>> BMP280_SHIFT_BIT_POSITION_BY_04_BITS));

	return comp_baro;
}



static void drv_baro_bosch_bmp280_irq_handle(void)
{
    /* no handle so far */
}

static int drv_baro_bosch_bmp280_open(void)
{
    int ret = 0;
    ret  =  drv_baro_bosch_bmp280_set_power_mode(&bmp280_ctx, DEV_POWER_ON);
    if(unlikely(ret)){
        return -1;
    }
    LOG("%s %s successfully \n", SENSOR_STR, __func__);
    return 0;

}

static int drv_baro_bosch_bmp280_close(void)
{
    int ret = 0;
    ret  = drv_baro_bosch_bmp280_set_power_mode(&bmp280_ctx, DEV_POWER_OFF);
    if(unlikely(ret)){
        return -1;
    }
    LOG("%s %s successfully \n", SENSOR_STR, __func__);
    return 0;
}


static int drv_baro_bosch_bmp280_read(void *buf, size_t len)
{
    int ret = 0;
    int data = 0;
    barometer_data_t* pdata = (barometer_data_t*)buf;
    if(buf == NULL){
        return -1;
    }
    
    ret = bmp280_read_uncomp_baro(&bmp280_ctx, &data);
    if(unlikely(ret)){
        return ret;
    }

    ret = bmp280_compensate_baro(data);
    if(unlikely(ret)){
        return ret;
    }

    pdata->p = data;

    pdata->timestamp = aos_now_ms();
    len = sizeof(barometer_data_t);
    
    return 0;
}

static int drv_baro_bosch_bmp280_write(const void *buf, size_t len)
{
    (void)buf;
    (void)len;
    return 0;
}

static int drv_baro_bosch_bmp280_ioctl(int cmd, unsigned long arg)
{

    int ret = 0;
    
    switch(cmd){
        case SENSOR_IOCTL_ODR_SET:{
            bmp280_odr_e odr = drv_baro_bosch_bmp280_hz2odr(arg);
            ret = drv_baro_bosch_bmp280_set_odr(&bmp280_ctx, odr);
            if(unlikely(ret)){
                return -1;
            }
        }break;
        case SENSOR_IOCTL_SET_POWER:{
            ret = drv_baro_bosch_bmp280_set_power_mode(&bmp280_ctx, arg);
            if(unlikely(ret)){
                return -1;
            }
        }break;
        case SENSOR_IOCTL_GET_INFO:{ 
            /* fill the dev info here */
            dev_sensor_info_t *info =arg;
            *(info->model) = "BMP280";
            info->range_max = 16;
            info->range_min = 4;
            info->unit = pa;

        }break;
       
       default:break;
    }

    LOG("%s %s successfully \n", SENSOR_STR, __func__);
    return 0;
}

int drv_baro_bosch_bmp280_init(void){
    int ret = 0;
    sensor_obj_t sensor;

    /* fill the sensor obj parameters here */
    sensor.tag = TAG_DEV_BARO;
    sensor.path = dev_baro_path;
    sensor.io_port = I2C_PORT;
    sensor.open = drv_baro_bosch_bmp280_open;
    sensor.close = drv_baro_bosch_bmp280_close;
    sensor.read = drv_baro_bosch_bmp280_read;
    sensor.write = drv_baro_bosch_bmp280_write;
    sensor.ioctl = drv_baro_bosch_bmp280_ioctl;
    sensor.irq_handle = drv_baro_bosch_bmp280_irq_handle;
    sensor.bus = &bmp280_ctx;


    ret = sensor_create_obj(&sensor);
    if(unlikely(ret)){
        return -1;
    }

    ret = drv_baro_bosch_bmp280_validate_id(&bmp280_ctx, BMP280_CHIP_ID_VAL);
    if(unlikely(ret)){
        return -1;
    }

    ret = drv_baro_bosch_bmp280_soft_reset(&bmp280_ctx);
    if(unlikely(ret)){
        return -1;
    }
    
    /* set the default config for the sensor here */
    ret = drv_baro_bosch_bmp280_set_default_config(&bmp280_ctx);
    if(unlikely(ret)){
        return -1;
    }

    ret = drv_baro_bosch_bmp280_get_calib_param(&bmp280_ctx);
    if(unlikely(ret)){
        return -1;
    }

    LOG("%s %s successfully \n", SENSOR_STR, __func__);
    return 0;
}


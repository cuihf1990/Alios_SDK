#ifndef UART_DEVICE_H
#define UART_DEVICE_H

#include <stdlib.h>
#include <stdarg.h>

#define UART_ONE_PACKAGE_LENGTH 1024
#define UART_WAIT_FOREVER    0xffffffffu
#define Refresh_Time 4*1000


int app_uart_init(void);
void uart_recv_thread(void *p);
void uart_cmd_process(uint8_t * inDataBuffer , int recv_len);
uint16_t _calc_sum(void *data, uint32_t len);
uint32_t _uart_get_one_packet(uint8_t *inBuf, int inBufLen);
void parseDeviceStatus(uint8_t *inBuf, uint8_t send);
void cloud_cmd_process(uint8_t * buf , int rec_len);
uint8_t start_property_report_cloud(uint8_t type);
void Report_total_value();

#define MAX_PARAMS 15


enum{
CURRENTTEMPERATURE = 0,
TARGETTEMPERATURE,
POWERSWITCH,
WINDSPEED,
WORKMODE,
SLEEP,
ECO,
IONS,
PTC,
SCREEN,
VERTICALSWITCH,
CLEAN,
MILDEWPROOF,
POWERLIMITPERCENT,
ERROR,
};

typedef struct _device_cmd_head{
  uint16_t flag; // Allways BB 00
  uint16_t cmd; // commands, return cmd=cmd|0x8000  0x0900
  uint16_t cmd_status; //return result
  uint16_t datalen; 
  uint8_t src;
  uint8_t dst;
  uint8_t a_data, b_data,c_data, d_data,e_data, f_data, g_data, h_data, i_data, j_data, k_data, l_data, m_data ,n_data, o_data, p_data, q_data, r_data, s_data, t_data,u_data,v_data,w_data;
  uint16_t fcc;
}device_cmd_head_t;

//d 空调应答控制数据
enum {
  CMD_READ_VERSION = 1,    //Deprecated in MICO, use bonjour to read device related info
  CMD_READ_CONFIG,         //Deprecated in MICO, use Easylink to R/W configuration
  CMD_WRITE_CONFIG,        //Deprecated in MICO, use Easylink to R/W configuration
  CMD_SCAN,                //Deprecated in MICO, that is no use for HA application
  CMD_OTA, 
  CMD_NET2COM,             //Deprecated in MICO, use Easylink to R/W configuration
  CMD_COM2NET,            //7
  CMD_GET_STATUS, 
  CMD_CONTROL,   //9 
  CMD_SEARCH, 
  MSG_TYPE_TEST = 0x65,
};


typedef struct
{
  int idx;
  const char *name;
} device_func_index;

typedef struct _uart_cmd
{
	  uint8_t CurrentTemperature;
	  uint8_t TargetTemperature;
	  uint8_t PowerSwitch;
	  uint8_t WindSpeed;
	  uint8_t WorkMode;
	  uint8_t Sleep;
	  uint8_t ECO;
	  uint8_t Ions;
	  uint8_t PTC;
	  uint8_t Screen;
	  uint8_t VerticalSwitch;
	  uint8_t Clean;
	  uint8_t MildewProof;
	  uint8_t PowerLimitPercent;
    uint8_t ErrorCode;
} uart_cmd_t;


#endif
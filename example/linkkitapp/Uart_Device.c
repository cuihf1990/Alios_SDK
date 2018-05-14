#include <stdio.h>
#include "Uart_Device.h"
#include "aos/aos.h"
#include "hal/soc/uart.h"
#include "hal/soc/flash.h"
#include "hal/soc/soc.h"
#include "board.h"
#include "linkkit_app.h"
#include "linkkit_export.h"

static aos_timer_t refresh_timer = {NULL};


//#define  UART_FOR_APP MICO_UART_2

extern sample_context_t g_sample_context;
extern uint8_t cloud_connect_status;
char Report_Cloud_Data[1500] = {0};

uart_dev_t uart_dev ;
uart_cmd_t uartcmd ;


uint8_t Storage_Uart2Wifi_Data[100] = {0};
uint8_t Storage_Wifi2Uart_Data[100] = {0};


static uint8_t Get_Deviceinfo[] = {0xBB, 0x00, 0x06, 0x80, 0x00, 0x00, 0x02, 0x00, 0x11, 0x01, 0x2B, 0x7E};
static uint8_t Get_Info[] =       {0xBB, 0x00, 0x06, 0x80, 0x00, 0x00, 0x02, 0x00, 0x21, 0x01, 0x1B, 0x7E};

device_func_index hiFuncStr[MAX_PARAMS] = {
  {-1,"CurrentTemperature"},	
  {-1, "TargetTemperature"},
  {-1, "PowerSwitch"},
  {-1, "WindSpeed"},
  {-1, "WorkMode"},
  {-1, "Sleep"},
  {-1, "ECO"},
  {-1, "Ions"},
  {-1, "PTC"},
  {-1, "Screen"},
  {-1,"VerticalSwitch"},
  {-1,"Clean"},
  {-1,"MildewProof"},
  {-1,"PowerLimitPercent"},
  {-1,"ERRORCODE"},
  {-1, NULL}
};



void linkkit_post_awss_reset(void);

void Timer_Function()   ////定时查询4s
{
	//LOG("get device status");
	aos_msleep(100);
	hal_uart_send(&uart_dev,Get_Deviceinfo,sizeof(Get_Deviceinfo),UART_WAIT_FOREVER);
	aos_msleep(500);
	hal_uart_send(&uart_dev,Get_Info,sizeof(Get_Info),UART_WAIT_FOREVER);
	aos_msleep(500);
}


uint16_t _calc_sum(void *data, uint32_t len)
{
  uint32_t cksum = 0;
  uint16_t *p = (uint16_t *)data;

  while (len > 1)
  {
    cksum += *p++;
    len -=2;
  }
  if (0 != len)
  {
  cksum += *(uint8_t *)p;
  }
  cksum = (cksum >> 16) + (cksum & 0xffff);
  cksum += (cksum >>16);
  return ~cksum;
}
/////
/////
/////串口初始化
/////
/////
int app_uart_init(void)
{
	uart_dev.port = UART_FOR_APP;
	uart_dev.config.baud_rate = 4800;
	uart_dev.config.data_width = DATA_WIDTH_8BIT;
	uart_dev.config.stop_bits = STOP_BITS_1;
	uart_dev.config.parity = EVEN_PARITY;
	uart_dev.config.flow_control = FLOW_CONTROL_DISABLED;

	memset(&uartcmd,-1,sizeof(uart_cmd_t));

	hal_uart_init(&uart_dev);
	aos_task_new("uart_recv", uart_recv_thread, NULL, 0x2000);

	return 1;
}

void uart_recv_thread(void *p)
{
	int recv_len;
	int ret = -1;
	uint8_t *inDataBuffer = NULL;
    
    if(inDataBuffer == NULL)
	   inDataBuffer = malloc(UART_ONE_PACKAGE_LENGTH);
	if (inDataBuffer == NULL)
		goto exit;

  	  ret = aos_timer_new(&refresh_timer, Timer_Function,NULL,Refresh_Time,1);
	  if(ret != 0)	LOG("init timer error");

	  ret = aos_timer_start(&refresh_timer);
	  if(ret != 0)	LOG("start timer error");

   	LOG("start get data");

	char property_output_identifier[64];
 
	while (1)
	{
		memset(inDataBuffer, 0x00, UART_ONE_PACKAGE_LENGTH);
		recv_len = _uart_get_one_packet(inDataBuffer, UART_ONE_PACKAGE_LENGTH);
		if (recv_len <= 0)
			continue;
		uart_cmd_process(inDataBuffer, recv_len);
	}

exit:
	if (inDataBuffer)
		free(inDataBuffer);
	aos_task_exit(0);

}

uint32_t _uart_get_one_packet(uint8_t *inBuf, int inBufLen)
{
    uint32_t expect_size = 1;
    uint32_t recv_size = 0;
    int datalen;
    uint32_t pos = 0;
    int i=0;

    while ( 1 )
    {
        pos = 0;
        //start
        hal_uart_recv_II( &uart_dev, inBuf+pos, 1, &recv_size, UART_WAIT_FOREVER );
        if(inBuf[pos] != 0xBB ) goto exit;
        pos += recv_size;

        hal_uart_recv_II( &uart_dev, inBuf+pos, 1, &recv_size, 500 );
        if(inBuf[pos] != 0x00 ) goto exit;
        pos += recv_size;

        hal_uart_recv_II( &uart_dev, inBuf+pos, 6, &recv_size, 500 );
        datalen = (inBuf[pos+4] + (inBuf[pos+5]<<8));
        pos += recv_size;
		

        hal_uart_recv_II( &uart_dev, inBuf+pos, datalen+2, &recv_size, 500 );
        pos += recv_size;

		return  pos;

	}
exit:
		LOG("get data value = %02x",inBuf[pos]);
        return -1;
}
/////
/////
///// 		数据处理以及上报处理
/////
/////
void uart_cmd_process(uint8_t * buf , int recv_len)
{

	uint16_t cksum;
 	device_cmd_head_t *cmd_header;

	cmd_header = (device_cmd_head_t *)buf;

	//LOG("datalen = %d",recv_len);
	
	// for(int i =0 ;i<recv_len ;i++){
	// 	LOG(" buf[%d] = %02x",i,buf[i]);
	// 	}

	switch(cmd_header->cmd){
		case CMD_READ_VERSION:
			cmd_header->cmd |= 0x8000;
       	 	buf[6] = 0x02;
         	buf[8] = 0x02;
         	buf[9] = 0x01;
         	cksum = _calc_sum(buf, 10);
          	buf[10] = cksum & 0x00ff;
          	buf[11] = (cksum & 0x0ff00) >> 8;
			hal_uart_send(&uart_dev,buf,12,UART_WAIT_FOREVER);
		break;
		case CMD_COM2NET:
        if((buf[9]>>4) ==1)     // control command    
   		{

			if(cloud_connect_status == 0)
				return ;
    		if(memcmp(Storage_Uart2Wifi_Data,buf,recv_len)==0)
            	return;
			memset(Storage_Uart2Wifi_Data,0,sizeof(Storage_Uart2Wifi_Data));
         	memcpy(Storage_Uart2Wifi_Data,buf,recv_len);
			memset(Storage_Wifi2Uart_Data,0,sizeof(Storage_Wifi2Uart_Data));
			memcpy(Storage_Wifi2Uart_Data,buf,recv_len);
       		parseDeviceStatus((uint8_t*) (&cmd_header->flag),1);
   		}
        if((buf[9]>>4) ==2)    //status command 
        {	
			if(cloud_connect_status == 0)
				return ;
			if(buf[6] ==0x19){
				hiFuncStr[CURRENTTEMPERATURE].idx = (cmd_header->f_data - 32);		
				if(hiFuncStr[CURRENTTEMPERATURE].idx <= 0)
					hiFuncStr[CURRENTTEMPERATURE].idx =0;		
				if(hiFuncStr[CURRENTTEMPERATURE].idx  != uartcmd.CurrentTemperature ){
					uartcmd.CurrentTemperature = hiFuncStr[CURRENTTEMPERATURE].idx;
					start_property_report_cloud(CURRENTTEMPERATURE);						
				}

				hiFuncStr[ERROR].idx = cmd_header->t_data;;
				if(hiFuncStr[ERROR].idx  != uartcmd.ErrorCode ){
					uartcmd.ErrorCode = hiFuncStr[ERROR].idx;
				char event_output_identifier[64];
				snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s", EVENT_ERROR_IDENTIFIER, EVENT_ERROR_OUTPUT_INFO_IDENTIFIER);
				linkkit_set_value(linkkit_method_set_event_output_value,   
								g_sample_context.thing,
								event_output_identifier,
								&uartcmd.ErrorCode, NULL);				
				}
			}
		}	   
		break;
		case CMD_CONTROL:
			cmd_header->cmd |= 0x8000;
			buf[4] = 1;
			cksum = _calc_sum(buf, 8);
          	buf[10] = cksum & 0x00ff;
          	buf[11] = (cksum & 0x0ff00) >> 8;
			hal_uart_send(&uart_dev,buf,12,UART_WAIT_FOREVER);
			//do_awss_reset();
			linkkit_post_awss_reset();
		break;
		case MSG_TYPE_TEST:
		break;
		default:
		break;
	}

}

void parseDeviceStatus(uint8_t *inBuf, uint8_t send)
{
 	device_cmd_head_t *cmd_header =(device_cmd_head_t *)inBuf;

	if((cmd_header->i_data&0x20)>>5)
		hiFuncStr[POWERSWITCH].idx = 1;//uartcmd.PowerSwitch = 1;
	else 
		hiFuncStr[POWERSWITCH].idx = 0;

	if(hiFuncStr[POWERSWITCH].idx != uartcmd.PowerSwitch){
		uartcmd.PowerSwitch = hiFuncStr[POWERSWITCH].idx;
		start_property_report_cloud(POWERSWITCH);	
	}

	if(((cmd_header->f_data)>>5)==1)
		hiFuncStr[WORKMODE].idx = 1;
	else if(((cmd_header->f_data)>>5)==4)
		hiFuncStr[WORKMODE].idx = 2;
	else if(((cmd_header->f_data)>>5)==2)
		hiFuncStr[WORKMODE].idx = 4;
	else if(((cmd_header->f_data)>>5)==6)
		hiFuncStr[WORKMODE].idx = 3;
	else if(((cmd_header->f_data)>>5)==0)
		hiFuncStr[WORKMODE].idx = 0;

	if(hiFuncStr[WORKMODE].idx != uartcmd.WorkMode){
		uartcmd.WorkMode = hiFuncStr[WORKMODE].idx;
		start_property_report_cloud(WORKMODE);	
	}
	////先判断强力 静音，之后再处理风速

//	LOG("  windspeed  = %02x ======%02x" , ((cmd_header->e_data)>>6),((cmd_header->d_data)>>5));
	if (((cmd_header->e_data)>>6) == 2)
    	hiFuncStr[WINDSPEED].idx = 1; //静音
	else if (((cmd_header->e_data)>>6) == 1)
	 	hiFuncStr[WINDSPEED].idx = 5; //强力
	else if(((cmd_header->e_data)>>6) == 0){
     	if (((cmd_header->d_data)>>5) == 5)
    		hiFuncStr[WINDSPEED].idx = 0; //自动
   		else if (((cmd_header->d_data)>>5) == 3)
       		hiFuncStr[WINDSPEED].idx = 2;  //低风
   		else if (((cmd_header->d_data)>>5) == 2)
      		hiFuncStr[WINDSPEED].idx = 3;   //中风
  		else if (((cmd_header->d_data)>>5) == 1)
     		hiFuncStr[WINDSPEED].idx = 4;	  //高风
	}

	if(hiFuncStr[WINDSPEED].idx != uartcmd.WindSpeed){
		uartcmd.WindSpeed = hiFuncStr[WINDSPEED].idx;
		start_property_report_cloud(WINDSPEED);	
	}

	if((cmd_header->f_data&(1<<2))>>2)
		hiFuncStr[SLEEP].idx = 1;
	else 
		hiFuncStr[SLEEP].idx = 0;
	if(hiFuncStr[SLEEP].idx != uartcmd.Sleep){
		uartcmd.Sleep = hiFuncStr[SLEEP].idx;
		start_property_report_cloud(SLEEP);	
	}

	if((cmd_header->i_data&(1<<3))>>3)
		hiFuncStr[ECO].idx = 1;
	else 
		hiFuncStr[ECO].idx = 0;
	if(hiFuncStr[ECO].idx != uartcmd.ECO){
		uartcmd.ECO = hiFuncStr[ECO].idx;
		start_property_report_cloud(ECO);	
	}

	if((cmd_header->i_data&(1<<1))>>1)
		hiFuncStr[IONS].idx  = 1;
	else 
		hiFuncStr[IONS].idx = 0;
	if(hiFuncStr[IONS].idx != uartcmd.Ions){
		uartcmd.Ions = hiFuncStr[IONS].idx;

		start_property_report_cloud(IONS);	
	}

	if((cmd_header->i_data&0x10)>>4)
		hiFuncStr[PTC].idx = 1;
	else 
		hiFuncStr[PTC].idx= 0;
	if(hiFuncStr[PTC].idx != uartcmd.PTC){
		uartcmd.PTC = hiFuncStr[PTC].idx;
		start_property_report_cloud(PTC);	
	}

	if((cmd_header->k_data&(1<<4))>>4)	
		hiFuncStr[SCREEN].idx = 1;
	else
	  	hiFuncStr[SCREEN].idx = 0;
	if(hiFuncStr[SCREEN].idx != uartcmd.Screen){
		uartcmd.Screen = hiFuncStr[SCREEN].idx;
		start_property_report_cloud(SCREEN);	
	}

	if((cmd_header->a_data)&0x07)
		hiFuncStr[VERTICALSWITCH].idx = 0;
	else 
		hiFuncStr[VERTICALSWITCH].idx  = 1; 
	if(hiFuncStr[VERTICALSWITCH].idx != uartcmd.VerticalSwitch){
		uartcmd.VerticalSwitch = hiFuncStr[VERTICALSWITCH].idx;
		start_property_report_cloud(VERTICALSWITCH);	
	}

	if((cmd_header->i_data&(1<<2))>>2)
		hiFuncStr[CLEAN].idx = 1;
	else 
		hiFuncStr[CLEAN].idx  = 0;
	if(hiFuncStr[CLEAN].idx != uartcmd.Clean){
		uartcmd.Clean = hiFuncStr[CLEAN].idx;
		start_property_report_cloud(CLEAN);	
	}

	if((cmd_header->k_data&(1<<3))>>3)
		hiFuncStr[MILDEWPROOF].idx = 1;
	else 
		hiFuncStr[MILDEWPROOF].idx = 0;
	if(hiFuncStr[MILDEWPROOF].idx != uartcmd.MildewProof){
		uartcmd.MildewProof = hiFuncStr[MILDEWPROOF].idx;
		start_property_report_cloud(MILDEWPROOF);	
	}

	if(cmd_header->l_data>>7 == 1){
		hiFuncStr[POWERLIMITPERCENT].idx = cmd_header->l_data;
	if(hiFuncStr[POWERLIMITPERCENT].idx != uartcmd.PowerLimitPercent){
		uartcmd.PowerLimitPercent = hiFuncStr[POWERLIMITPERCENT].idx;
		start_property_report_cloud(POWERLIMITPERCENT);	
	}
	}
    hiFuncStr[TARGETTEMPERATURE].idx = ((cmd_header->a_data)>>3) + 8;
	//	if(hiFuncStr[TARGETTEMPERATURE].idx >= 23)
	//		hiFuncStr[TARGETTEMPERATURE].idx = 23;
	if(hiFuncStr[TARGETTEMPERATURE].idx != uartcmd.TargetTemperature){
		uartcmd.TargetTemperature = hiFuncStr[TARGETTEMPERATURE].idx;
		start_property_report_cloud(TARGETTEMPERATURE);	
	}

}

uint8_t start_property_report_cloud(uint8_t type)
{  
 	char property_output_identifier[64];
    snprintf(property_output_identifier, sizeof(property_output_identifier), "%s", hiFuncStr[type].name);

	if(type ==TARGETTEMPERATURE ||type ==CURRENTTEMPERATURE)
	{
		double CurrentTemperature;
		CurrentTemperature= (double)hiFuncStr[type].idx;
  		linkkit_set_value(linkkit_method_set_property_value,   
                      g_sample_context.thing,
                      property_output_identifier,
                      &CurrentTemperature, NULL);		
		LOG("=============identifier = %s,%d,%d",property_output_identifier,hiFuncStr[type].idx,type);
	}else 
	{	linkkit_set_value(linkkit_method_set_property_value,   
                      g_sample_context.thing,
                      property_output_identifier,
                      &(hiFuncStr[type].idx), NULL);
		LOG("=============identifier = %s,%d,%d",property_output_identifier,hiFuncStr[type].idx,type);
	}
		linkkit_post_property(g_sample_context.thing,property_output_identifier);
		return 1;

}
/////
/////
///// 		云端解析 以及 下发处理
/////
/////
void cloud_cmd_process(uint8_t * input_name , int value)
{
	char *value_str = NULL, *attr_str = NULL;
    for (int i = 0; i<MAX_PARAMS; i++) {
		if(strstr(hiFuncStr[i].name,input_name) !=0){
			if(strstr(hiFuncStr[i].name , "CurrentTemperature")  != 0)
				return;
			LOG(" name = %s,idx = %d , value =%d",hiFuncStr[i].name,hiFuncStr[i].idx,value);
			if(hiFuncStr[i].idx != value)
			{
				Send_MCU_DATA(i,value);
			}
			break;
		}
    }

}


void Send_MCU_DATA(int type ,int value)
{
    uint16_t cksum;
	device_cmd_head_t *p_out;
	int send_datalen = 0;
	p_out = (device_cmd_head_t *)Storage_Wifi2Uart_Data;
	if(Storage_Wifi2Uart_Data[0] == 0 && Storage_Wifi2Uart_Data[1] ==0)    ///状态未同步前，APP无法下发
		return;
	p_out->flag = 0xBB;
	p_out->cmd  = 0x06;
	p_out->cmd |=0x8000;
	p_out->datalen = 0x0F;
	p_out->src = 0x01;
	p_out->dst = 0x01;
	p_out->c_data &= ~(1<<7);
	p_out->k_data &= ~(1<<5);
	send_datalen = (p_out->datalen +10);
	switch(type){
		case POWERSWITCH:
			if(value == 0)
			{
				p_out->e_data &= ~(3<<6);    //关闭静音和强力
				p_out->i_data &= ~(1<<4);  ///关闭辅热
				p_out->f_data &= ~(1<<2);  ///关闭睡眠
				p_out->i_data &= ~(1<<3);  //关闭ECO
				p_out->i_data &= ~(1<<1);  //关闭健康
			}
			else if(value == 1)
			{
        		p_out->i_data &=~(3<<2);
			}
  			p_out->i_data &= ~(1<<5);
			p_out->i_data |= value<<5;
		break;
		case TARGETTEMPERATURE:
			if(((p_out->f_data)>>5) == 0 || ((p_out->f_data)>>5) == 5){     //
				return ;
			}else{
			p_out->a_data &= ~(0x1f<<3);
			p_out->a_data |= (value-8)<<3;
			}	
		break;
		case WORKMODE:
			p_out->e_data &= ~(3<<6);   ///关闭静音和强力
			p_out->f_data &= ~(7<<5);
			p_out->i_data &= ~(1<<4);  ///关闭辅热
			p_out->f_data &= ~(1<<2);  ///关闭睡眠
			p_out->i_data &= ~(1<<3);  //关闭ECO
			if(value == 0)     //自动
			{
				p_out->f_data  |= 0<<5;
				p_out->a_data &= ~(0x1f<<3);
				p_out->a_data |= 16<<3;
			}

			else if(value == 1)//制冷
			p_out->f_data  |= 1<<5;
			else if(value == 2) //制热
			p_out->f_data  |= 4<<5;
			else if(value == 3) ///通风
			{
				p_out->f_data  |= 6<<5;
				if(((p_out->d_data)>>5) == 5)   //自动风情况下 转成低风
				{
				p_out->d_data &= ~(7<<5);
				p_out->d_data |= 3<<5;     // 设置成低风
				}
			}else if(value == 4)
			{
				p_out->f_data  |= 2<<5;
			}
			LOG("   ===== ====%02x",p_out->e_data);
		break;
		case WINDSPEED:
			p_out->d_data &= ~(7<<5);
			p_out->e_data &= ~(3<<6);   ///关闭静音和强力
			if(value == 0)   //自动  
			{
				if(((p_out->f_data)>>5) == 6)     ////送风模式下不能自动风	
					return;
				else 
					p_out->d_data |= 5<<5;
			}else if(value == 1){  //静音
				p_out->e_data = ~(3<<6);
				p_out->e_data |=  2<<6;
			}else if(value == 2){  //低风
				p_out->d_data |= 3<<5;
			}else if(value == 3){  //中风
				p_out->d_data |= 2<<5;
			}else if(value == 4){  //高风
				p_out->d_data |= 1<<5;
			}else if(value == 5){  //强力风
			if(((p_out->f_data)>>5) ==2|| ((p_out->f_data)>>5) ==0||((p_out->f_data)>>5) ==6){      ////除湿  自动  送风下 不能操纵强力风
				return;
			}else {
				p_out->e_data |=  1<<6;
			}
			}
			LOG("   ===== ====%02x",p_out->e_data);
			break;
		case PTC:
			if(((p_out->f_data)>>5) == 4 ){   ////辅热功能 仅在制热模式下有效
				p_out->i_data &= ~(1<<4);
				p_out->i_data |= value<<4;
				}else 
				return;
		break;
		case SCREEN:
			p_out->k_data &= ~(1<<4);
			p_out->k_data |= value<<4;
		break;
		case IONS:
			p_out->i_data &= ~(1<<1);
			p_out->i_data |= value<<1;
		break;
		case SLEEP:
			if(((p_out->f_data)>>5) ==6){   ///送风模式下 不能操作睡眠
				return;
			}else {
				p_out->f_data &= ~(1<<2);
				p_out->f_data |= value<<2;
			}
		break;
		case VERTICALSWITCH:
			p_out->a_data &=~0x07;
			if(value== 0)
			p_out->a_data |= 0x07;
		break;
		case ECO:
			if(((p_out->f_data)>>5) == 1 ){   ////ECO功能 仅在制冷模式下有效
			p_out->i_data &= ~(1<<3);
			p_out->i_data |= value<<3;
			}else 
				return;
		break;
		case POWERLIMITPERCENT:
			p_out->l_data=0x80;
			p_out->l_data |= value;
		break;
		case MILDEWPROOF:
			if( (p_out->i_data & 0x20) != 0)  ///开机状态
				return;
			else{
				p_out->k_data &=~(1<<3);
				p_out->k_data |= value<<3;
			}
				
				
			break;
		case CLEAN:
			LOG(" onoff =%02x",p_out->i_data);
			if( (p_out->i_data & 0x20) != 0)  ///开机状态
			{
				LOG("ON ");
				return;
			}
			else{
				p_out->i_data &=~(1<<2);
				p_out->i_data |= value<<2;
				p_out->k_data &= ~(1<<4);
				p_out->k_data |= 1<<4;
			}
				
			break;
		default:
		break;
	}
		cksum = _calc_sum(Storage_Wifi2Uart_Data, 23);
		Storage_Wifi2Uart_Data[23] = cksum & 0x00ff;
		Storage_Wifi2Uart_Data[24] = (cksum & 0x0ff00) >> 8;
		int ret =-1;	
		ret = aos_timer_stop(&refresh_timer);
	  	if(ret != 0)	LOG("stop timer error");

		LOG("send data   powerswitch %02x ",Storage_Wifi2Uart_Data[18]);

		hal_uart_send(&uart_dev,Storage_Wifi2Uart_Data,send_datalen,UART_WAIT_FOREVER);
		aos_msleep(150);	
/////发完控制命令之后，立马查询
		hal_uart_send(&uart_dev,Get_Deviceinfo,sizeof(Get_Deviceinfo),UART_WAIT_FOREVER);
		aos_msleep(150);
		hal_uart_send(&uart_dev,Get_Info,sizeof(Get_Info),UART_WAIT_FOREVER);
		aos_msleep(150);

		ret = aos_timer_start(&refresh_timer);
	  	if(ret != 0)	LOG("start timer error");
		
}

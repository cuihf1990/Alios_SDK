#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DEVICE_BUSY		-1
#define DEVICE_FREE		-2
#define ERROR_PARAM   		-3
#define ERROR_HWIO		-4
#define ERROR_FLAG		-5
#define	ERROR_HEAD		-6
#define	ERROR_SUM		-7
#define ERROR_MEM		-8
#define ERROR_DATA		-9
#define PARAM_MAX_LEN 		768

#define DEBUG	
#pragma pack(1)	
struct package_head {
    	uint8_t head_h;
    	uint8_t head_l;
    	uint8_t flag;
    	uint8_t pkglen_h;
    	uint8_t pkglen_l;
};


#ifdef CONFIG_AS608
struct rsa_sign_data{
	uint8_t	keyId;
	uint8_t input_len_h;
	uint8_t input_len_l;
	//uint8_t input[128];
	uint8_t input[512]; //cndy 2016/05/30
	uint8_t sign_type;
	uint8_t	padding;
};

struct rsa_verify_data{
	uint8_t	keyId;
	uint8_t input_len_h;
	uint8_t input_len_l;
	uint8_t	input[128];
	uint8_t	signature_len_h;
	uint8_t	signature_len_l;
	uint8_t	signature[128];
	uint8_t sign_type;
	uint8_t	padding;
};

struct rsa_pubkey_data{
	uint8_t	keyId;
	uint8_t	input_len_h;
	uint8_t	input_len_l;
	uint8_t	input[128];
	uint8_t	padding;  
};

struct rsa_privkey_data{
	uint8_t	keyId;
	uint8_t input_len_h;
	uint8_t input_len_l;
	uint8_t	input[128];
	uint8_t	padding; 
};
#endif 
struct response_data{
	uint8_t length_h;
	uint8_t	length_l;
	uint8_t data[0];
};

#ifdef CONFIG_AS300
struct Ddes_data{
	uint8_t keyId;
	uint8_t input_len_h;
	uint8_t input_len_l;
	uint8_t data[128];
	uint8_t padding;
};

#endif 

#ifdef CONFIG_SYNO_FINGER
struct syno_data{
	uint8_t data_len_h;
	uint8_t data_len_l;
	uint8_t data[384];
};
#endif 

#pragma pack()

static uint32_t   is_hwopend = 0;
static int calc_checksum(uint8_t *pdata, int length)
{
    int sum = 0;
    int i = 0;
    while(i < length) {
        sum += pdata[i];
        i++;
    }
    return sum;
}


static int package_unpack(int cmd,uint8_t *input,uint8_t *output_param)
{		
	uint16_t dlen,sum;
	uint16_t len1,len2;
	uint8_t	*pdata = NULL;
	struct package_head *pkg_head;
#ifdef CONFIG_AS608
	struct rsa_sign_data  	*sign_data;
	struct rsa_verify_data 	*verify_data;
	struct rsa_pubkey_data 	*pubkey_data;
	struct rsa_privkey_data *privkey_data;
#endif 
#ifdef CONFIG_AS300
	struct Ddes_data 	*des_data;
#endif 
	
	pkg_head = (struct package_head *)input;
	
	if (pkg_head->head_h != 0xEF && pkg_head->head_l != 0x01) {
		LOG("SYNO TFSL:check head error!\n");
       		return ERROR_HEAD;
   	}

   	if (pkg_head->flag != 0x01) {
		LOG("SYNO TFSL: check flag error!\n");
        	return ERROR_FLAG;
    	}
    
  	dlen = (pkg_head->pkglen_h << 8)  + pkg_head->pkglen_l;
  
  	pdata = input + sizeof(struct package_head) + dlen; //point to sum
  
 	sum = calc_checksum(input + sizeof(struct package_head) , dlen);
  
  	if(sum != ((*pdata << 8) + *(pdata + 1))){
		LOG("SYNO TFSL: input data checksum error\n");
		return ERROR_SUM;
	}
		
		
	pdata = input + sizeof(struct package_head) + 1; // point to data 
	switch(cmd){

	case 0x01:
	case 0x02:	
	case 0x08:
	case 0x34:
	case 0x41: 
		break;
#ifdef CONFIG_AS608
	case 0x53:
		sign_data = (struct rsa_sign_data *)output_param;
		sign_data->keyId = pdata[0];
		sign_data->input_len_h = pdata[1];
		sign_data->input_len_l = pdata[2];

		len1 = (pdata[1] << 8) + pdata[2];
		if(len1 > 512) { LOG("input data error ,too big length !\n");return ERROR_DATA;} 

		memcpy(sign_data->input,&pdata[3],len1);
		sign_data->sign_type = pdata[3 + len1];
		sign_data->padding = pdata[3 + len1 + 1];
		break;
	case 0x54:
		verify_data = (struct rsa_verify_data *)output_param;
		verify_data->keyId = pdata[0];
		verify_data->input_len_h = pdata[1];
		verify_data->input_len_l = pdata[2];
				
		len1 = (pdata[1] << 8) + pdata[2]; 
		if(len1 >128) return ERROR_DATA;
		memcpy(verify_data->input,&pdata[3],len1);
				
		verify_data->signature_len_h = pdata[3 + len1];
		verify_data->signature_len_l = pdata[3 + len1 + 1];
				
		len2 = (pdata[3 + len1] << 8) + pdata[3 + len1 + 1];
		if(len2 >128) return ERROR_DATA;
		memcpy(verify_data->signature,&pdata[3 + len1 + 1 + 1],len2);
				
		verify_data->sign_type = pdata[3 + len1 + 1 + 1 + len2];
		verify_data->padding = pdata[3 + len1 + 1 + 1 + len2 + 1];
		break;
	case 0x55:
		pubkey_data = (struct rsa_pubkey_data *)output_param;
		pubkey_data->keyId = pdata[0];
		pubkey_data->input_len_h = pdata[1];
		pubkey_data->input_len_l = pdata[2];
				
		len1 = (pdata[1] << 8) + pdata[2];
		if(len1 >128) return ERROR_DATA;

		memcpy(pubkey_data->input,&pdata[3],len1);
		pubkey_data->padding = pdata[3 + len1];
		break;
	case 0x56:
		privkey_data = (struct rsa_privkey_data *)output_param;
		privkey_data->keyId = pdata[0];
		privkey_data->input_len_h = pdata[1];
		privkey_data->input_len_l = pdata[2];
				
		len1 = (pdata[1] << 8) + pdata[2];
		if(len1 >128) return ERROR_DATA;

		memcpy(privkey_data->input,&pdata[3],len1);
		privkey_data->padding = pdata[3 + len1];
		break;
#endif 

#ifdef CONFIG_AS300
	case  0x57:
	case  0x58:
		des_data = (struct Ddes_data *)output_param;
		des_data->keyId = pdata[0];
		des_data->input_len_h = pdata[1];
		des_data->input_len_l = pdata[2];

		len1 = (pdata[1] << 8) + pdata[2];
		if(len1 >128) return ERROR_DATA;

		memcpy(des_data->data,&pdata[3],len1);
		des_data->padding = pdata[3 + len1];
		break;
#endif 
	default:
		LOG("SYNO TFSL:input cmd error!\n");
		return ERROR_PARAM;
			
	}
   	return 0;
}

static int package_contexts(int cmd ,uint8_t *param,int param_len,uint8_t *contexts)
{
	int len = 0;
	int sum = 0;
	if(param == NULL || contexts == NULL){
		LOG("SYNO TFSL:input memory error!\n");
		return -1;
	}
	/*head*/
	contexts[len++] = 0xef;
	contexts[len++] = 0x01;
	contexts[len++] = 0x01;
	contexts[len++] = ((param_len + 1) >> 8) & 0xff;
	contexts[len++] = ((param_len + 1) >> 0) & 0xff;
	
	/*cmd*/
	contexts[len++] = cmd;
	
	/*data*/
	if(param_len){
		memcpy(&contexts[len],param,param_len);
		len += param_len;
	}
	
	/*checksum*/
	sum = calc_checksum(&contexts[5],param_len + 1);
	contexts[len++] = (sum >> 8) & 0xff;
	contexts[len++] = (sum >> 0) & 0xff;
#ifdef DEBUG
	LOG("sending data:\n");
	for(sum = 0; sum < len; sum++)
		LOG("0x%02x,",contexts[sum]);
	LOG("\n\n");
#endif 	
	return len;
	
}

static int package_response_data(int cmd, uint8_t *output_data,uint8_t *output,uint32_t *out_len)
{
	int pkg_len,data_len,tmp,sum,len = 0;
	struct response_data  *response = NULL;
	if(output_data == NULL || output == NULL || out_len == NULL){
		LOG("SYNO TFSL:input memory error!\n");
		if(output_data == NULL)  LOG("SYNO TFSL:output data null\n");
		if(output == NULL)	LOG("SYNO TFSL:output null \n");
		if(out_len == NULL)	LOG("SYNO TFSL:out len null\n");
		return -1;
	}
	
	if (output_data[0] != 0xEF && output_data[1] != 0x01) {
		LOG("SYNO TFSL:check head error!\n");
        	*out_len = 0;
		return -2;
   	}

    	if (output_data[2] != 0x07) {
		LOG("SYNO TFSL:check flag error!\n");
       		*out_len = 0;
		return -3;
    	}
	
	if(output_data[5] != cmd){
		LOG("SYNO TFSL:cmd error:send cmd is 0x%02x  recive :0x%02x\n",cmd,output_data[5]);
		*out_len = 0;
		return -4;
	}


	data_len = tmp = 0;
	if(cmd != 0x54 && cmd != 0x01 && cmd != 0x02){  //cmd 0x54 have no data
		response = (void *)&output_data[sizeof(struct package_head) + 1];//point to data
		data_len = (response->length_h << 8) +  response->length_l;
		//if(pkg_len > (512 - (sizeof(struct package_head) + 2))){
		if(data_len > (PARAM_MAX_LEN - (sizeof(struct package_head) + 2))){
			LOG("SYNO TFSL:data length error!data length is to big!\n");
			*out_len = 0;
			return -5;
		}
		if(cmd == 0x34 || cmd == 0x08)
			pkg_len = data_len + 4;
		else
			pkg_len = 128 + 4;
	}
	else {
		pkg_len = 2; //onle cmd + ack
	}
	tmp = output_data[sizeof(struct package_head) + pkg_len - 1];
	if(tmp != 0x00){ //ack error
		LOG("SYNO TFSL:check ack data error![ack : 0x%02x]\n",tmp);
		*out_len = 0;
		return -6;
	}
	if(cmd == 0x41)
	  pkg_len = data_len + 3;
	else
	  pkg_len = data_len + 4;
	output[len++]	= output_data[0];
	output[len++]	= output_data[1];
	output[len++]	= output_data[2];
	output[len++]	= pkg_len >> 8; //????
	output[len++]	= pkg_len >> 0;
	output[len++]	= output_data[5];


	if(cmd == 0x01 || cmd == 0x02 ||cmd == 54){
		tmp = 0;
	}
	else if (cmd == 0x41){
		tmp = 2 + 128;
		memcpy(&output[len],response->data,data_len);
		len += data_len;
		output[len++] = response->length_l;
	}
	else if (cmd == 0x08 || cmd == 0x34){
		output[len++] = response->length_h;
		output[len++] = response->length_l;
		memcpy(&output[len],response->data,data_len);
		len += data_len;
		tmp = 2 + 384;
	}
	else {
		output[len++] = response->length_h;
		output[len++] = response->length_l;
		memcpy(&output[len],response->data,data_len);
		len += data_len;
		tmp = 2 + 128;
	}

	/*if(cmd != 0x54 ){
		tmp += 2;	//have two byte length
		if(cmd == 0x41){ //not normal datas + 1 byte length 
			memcpy(&output[len],response->data,pkg_len);
			len += pkg_len;
			output[len++]	= response->length_l;
			
		}else{ // 2byte length + datas
			output[len++]	= response->length_h;
			output[len++]	= response->length_l;
			memcpy(&output[len],response->data,pkg_len);
			len += pkg_len;
		}
		if(tmp)  tmp += 128;	
	}*/
	
	sum = calc_checksum(&output_data[5],pkg_len);
	output[len++]	= output_data[5 + tmp + 1]; //ack
	//output[len++]	= output_data[5 + tmp + 2];//sum h
	//output[len++]	= output_data[5 + tmp + 3];//sum l
	output[len++] = (sum >> 8)  & 0xff;
	output[len++] = (sum >> 0)  & 0xff;

#ifdef DEBUG
	LOG("report data: len %d\n",len);
	for(tmp = 0; tmp < len; tmp++)
		LOG("0x%02x,",output[tmp]);	
	LOG("\n\n");
#endif 	

	*out_len = len;
	return 0;
}
int open_session(void **handle){
	
	if(is_hwopend){
		LOG("AS608 hardware id busying!\n");
		return DEVICE_BUSY;
	}
	
	is_hwopend = 1;
	return DeviceOpen(handle);
}

int close_session(void *handle){
	
	if(!is_hwopend){
		LOG("AS608 no need free!\n");
		return DEVICE_FREE;
	}
	
	
	is_hwopend = 0;
	DeviceClose(handle);
	return 0;
}

int invoke_command(void *handle,uint32_t cmd,void *input,uint32_t in_len,void *output,uint32_t *out_len)
{
	uint32_t length = 0, ret = 0;
	uint8_t *pdata;
	//uint8_t contexts[1024] = {0};
	int contexts_len;
	//uint8_t output_data[1024] = {0};
	int output_data_len;
	uint8_t *contexts = 0;
	uint8_t *output_data = 0;
	
	LOG("invoke_command is in 2233445566\n");
	
#ifdef DEBUG
	int i = 0;
	char *p = input;
	LOG("input data: %p \n", p);
	for(i = 0 ; i < in_len;i++)
		LOG("0x%02x,", *(p + i));
	LOG("\n cmd = 0x%02x \n",cmd);
#endif 
	
	if (input == NULL || output == NULL || out_len == NULL) {
       	 	return ERROR_MEM;
    	}
 
	if(cmd == 0x41)
		length = 0;   
#ifdef CONFIG_AS608   
    	else if(cmd == 0x53)
    		length = sizeof(struct rsa_sign_data);
        else if(cmd == 0x54)
    		length = sizeof(struct rsa_verify_data);
    	else if(cmd == 0x55)
    		length =  sizeof(struct rsa_pubkey_data);
    	else if(cmd == 0x56)
    		length = sizeof(struct rsa_privkey_data);
#endif 

#ifdef CONFIG_AS300
	else if(cmd == 0x57 || cmd == 0x58)
    		length = sizeof(struct Ddes_data);
#endif 
#ifdef CONFIG_SYNO_FINGER
	else if (cmd == 0x08 || cmd == 0x34)
		length = 0;
	else if(cmd == 0x01|| cmd == 0x02)
		length = 0;
#endif 
	*out_len = 0;   		
	pdata =  (void *)DEBUG_MALLOC(length * sizeof(uint8_t) + 1);
	if(pdata == NULL){
		LOG("SYNO TFSL:malloc pdata memory error!\n");
		return ERROR_MEM;
	}

	memset(pdata,0,(length * sizeof(uint8_t) + 1));
    	ret = package_unpack(cmd,input,pdata);
    	if(ret != 0){
    		DEBUG_FREE(pdata);
    		LOG("SYNO TFSL:unpack package error!\n");
    		return ret;
    	}
    
      contexts = (uint8_t*)DEBUG_MALLOC(PARAM_MAX_LEN);
      memset(contexts, 0, PARAM_MAX_LEN);
    	contexts_len = package_contexts(cmd,pdata,length, contexts);
    	if(contexts_len < 0){
    		DEBUG_FREE(pdata);
    		LOG("SYNO TFSL:package sending context error!\n");
    		DEBUG_FREE(contexts);
    		return ERROR_PARAM;
    	}
  output_data = (uint8_t*) DEBUG_MALLOC(PARAM_MAX_LEN);
  memset(output_data, 0, PARAM_MAX_LEN);
  
  //LOG("===== 1111111111111111111111111111 memset done\n ");
  //LOG("===== [before DeviceTransmit] args: %p %s %p\n\n", handle, contexts, output_data);
  //LOG("===== 2222222222222222222222222222\n ");
	ret =  DeviceTransmit(handle,contexts,contexts_len, output_data, &output_data_len);
	//LOG("===== 3333333333333333333333333333\n ");
	//LOG("===== [after DeviceTransmit] args: %p %s\n\n", handle, output_data);
	DEBUG_FREE(contexts);
	if(ret != 0){
		 DEBUG_FREE(pdata);
		 DEBUG_FREE(output_data);
		 LOG("SYNO TFSHAL:Hwardware transmit error!\n");
		 return ERROR_HWIO;
	}
#ifdef DEBUG
	LOG("+++++++++++=hal recive len : %d+++++++++ \n",output_data_len);
	LOG("recive data :\n");
	for (i = 0 ; i < output_data_len;i++ )
		LOG("0x%02x,",output_data[i]);
	LOG("\n\n\n");
#endif 	
		
	ret = package_response_data(cmd,output_data,output,out_len);
	DEBUG_FREE(output_data);
	if(ret != 0){
		DEBUG_FREE(pdata);
		LOG("SYNO TFSL:package response data error!\n");
		return ERROR_PARAM;
	}
		
	DEBUG_FREE(pdata);
	return ret;
}

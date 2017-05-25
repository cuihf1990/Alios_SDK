#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include "yos/log.h"
#include "alink_export.h"
#include "json_parser.h"
#include "yos/framework.h"
#include "kvmgr.h"

/* raw data device means device post byte stream to cloud,
 * cloud translate byte stream to json value by lua script
 * for each product model, vendor need to sumbit a lua script
 * doing this job
 *
 * undefine RAW_DATA_DEVICE, sample will post json string to cloud
 */
//#define RAW_DATA_DEVICE

#define Method_PostData    "postDeviceData"
#define Method_PostRawData "postDeviceRawData"
#define Method_GetAlinkTime "getAlinkTime"

#define PostDataFormat      "{\"ErrorCode\":{\"value\":\"%d\"},\"Hue\":{\"value\":\"%d\"},\"Luminance\":{\"value\":\"%d\"},\"Switch\":{\"value\":\"%d\"},\"WorkMode\":{\"value\":\"%d\"}}"
#define post_data_buffer_size    (512)

#ifndef RAW_DATA_DEVICE
static char post_data_buffer[post_data_buffer_size];
#else
static char raw_data_buffer[post_data_buffer_size];

/* rawdata byte order
 *
 * rawdata[0] = 0xaa;
 * rawdata[1] = 0x07;
 * rawdata[2] = device.power;
 * rawdata[3] = device.work_mode;
 * rawdata[4] = device.temp_value;
 * rawdata[5] = device.light_value;
 * rawdata[6] = device.time_delay;
 * rawdata[7] = 0x55;
 */
#define RAW_DATA_SIZE           (8)
uint8_t device_state_raw_data[RAW_DATA_SIZE] = {
    0xaa, 0x07, 1, 8, 10, 50, 10, 0x55
};
#endif

enum {
    ATTR_ERRORCODE_INDEX,
    ATTR_HUE_INDEX,
    ATTR_LUMINANCE_INDEX,
    ATTR_SWITCH_INDEX,
    ATTR_WORKMODE_INDEX,
    ATTR_MAX_NUMS
};
#define DEVICE_ATTRS_NUM   (ATTR_MAX_NUMS)

int device_state[] = {0, 10, 50, 1, 2};/* default value */
char *device_attr[] = {
    "ErrorCode",
    "Hue",
    "Luminance",
    "Switch",
    "WorkMode",
    NULL
};

static int need_report = 1; /* force to report when device login */

void helper_api_test(void);
int activate_button_pressed(void);
void cloud_connected(void) { 
    printf("alink cloud connected!\n"); 
}
void cloud_disconnected(void) { printf("alink cloud disconnected!\n"); }

int callback_upgrade_device(const char *params)
{
    printf("alink device start to upgrade. \n");
}

int callback_cancel_upgrade_device(const char *params)
{
    printf("alink device stop to upgrade. \n");
}

#ifndef RAW_DATA_DEVICE
void cloud_get_device_status(char *json_buffer) 
{ 
    need_report = 1; 
    printf("---> get device status :  %s\n",json_buffer);
}

int cloud_set_device_status(char *json_buffer)
{
    int attr_len = 0, value_len = 0, value = 0, i;
    char *value_str = NULL, *attr_str = NULL;

    need_report = 1;
    printf("---> set device status :  %s\n",json_buffer);
    for (i = 0; device_attr[i]; i++) {
        attr_str = json_get_value_by_name(json_buffer, strlen(json_buffer),
                device_attr[i], &attr_len, NULL);
        value_str = json_get_value_by_name(attr_str, attr_len,
                "value", &value_len, NULL);

        if (value_str && value_len > 0) {
            char last_char = *(value_str+value_len);
            *(value_str + value_len) = 0;
            value = atoi(value_str);
            *(value_str + value_len) = last_char;
            device_state[i] = value;
        }
    }

    return 0;
}

#else
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

int uart_send(const uint8_t *raw_data, int raw_data_len)
{
    //TODO: implement uart send here
    int i;
    for (i = 0; i < raw_data_len; i++)
        printf("%02x ", raw_data[i]);

    printf("\n");
    return 0;
}

int raw_data_unserialize(char *json_buffer, uint8_t *raw_data, int *raw_data_len)
{
    int attr_len = 0, i = 0;
    char *attr_str = NULL;

    assert(json_buffer && raw_data && raw_data_len);

    attr_str = json_get_value_by_name(json_buffer, strlen(json_buffer),
            "rawData", &attr_len, NULL);

    if (!attr_str || !attr_len || attr_len > *raw_data_len * 2)
        return -1;

    for (i = 0; i < attr_len; i += 2) {
        raw_data[i / 2] = a2x(attr_str[i]) << 4;
        raw_data[i / 2] += a2x(attr_str[i + 1]);
    }

    raw_data[i / 2] = '\0';
    *raw_data_len = i / 2;

    return 0;
}

int cloud_get_device_raw_data(char *json_buffer)
{
    int ret = 0, raw_data_len = RAW_DATA_SIZE;
    uint8_t raw_data[RAW_DATA_SIZE] = { 0 };

    need_report = 1;

    ret = raw_data_unserialize(json_buffer, raw_data, &raw_data_len);
    if (!ret)
        return uart_send(raw_data, raw_data_len);
    else
        return -1;
}

int cloud_set_device_raw_data(char *json_buffer)
{
    int ret = 0, raw_data_len = RAW_DATA_SIZE;
    uint8_t raw_data[RAW_DATA_SIZE] = { 0 };

    need_report = 1;

    ret = raw_data_unserialize(json_buffer, raw_data, &raw_data_len);
    if (!ret) {
        /* update device state */
        memcpy(device_state_raw_data, raw_data, raw_data_len);
        return uart_send(raw_data, raw_data_len);
    } else
        return -1;
}

int alink_post_raw_data(uint8_t *byte_stream, int len)
{
        int i, size;
#define RawDataHeader   "{\"rawData\":\""
#define RawDataTail     "\", \"length\":\"%d\"}"

        size = strlen(RawDataHeader);
        strncpy(raw_data_buffer, RawDataHeader, post_data_buffer_size);
        for (i = 0; i < len; i++) {
           size += snprintf(raw_data_buffer + size,
                   post_data_buffer_size - size, "%02X", byte_stream[i]);
        }

        size += snprintf(raw_data_buffer + size,
                post_data_buffer_size - size, RawDataTail, len * 2);

        return alink_report(Method_PostRawData, raw_data_buffer);
}
#endif

static uint32_t work_time = 60*60*10; //default work time 1s

void main_loop(void *arg)
{
    //TODO: async
    //activate_button_pressed();
    //helper_api_test();
    if(need_report){
#ifdef RAW_DATA_DEVICE
        /*
         * Note: post data to cloud,
         * use sample alink_post_raw_data()
         * or alink_post()
         */
        /* sample for raw data device */
        alink_post_raw_data(device_state_raw_data, RAW_DATA_SIZE);

#else
        /* sample for json data device */
        snprintf(post_data_buffer, post_data_buffer_size, PostDataFormat,
                device_state[ATTR_ERRORCODE_INDEX],
                device_state[ATTR_HUE_INDEX],
                device_state[ATTR_LUMINANCE_INDEX],
                device_state[ATTR_SWITCH_INDEX],
                device_state[ATTR_WORKMODE_INDEX]);
        printf("start report async\n");
        alink_report_async(Method_PostData, post_data_buffer,NULL, NULL);
#endif
        need_report = 0;
    }
    if(--work_time)
        yos_post_delayed_action(100,main_loop, NULL);
    else{
        yos_loop_exit();
        printf("alink sample will stop. \n"); 
    }
}


int alink_get_time(unsigned int *utc_time)
{
#define TIME_STR_LEN    (32)
    char buf[TIME_STR_LEN] = { 0 }, *attr_str;
    int size = TIME_STR_LEN, attr_len = 0;
    int ret;

    ret = alink_query(Method_GetAlinkTime, "{}", buf, &size);
    if (!ret) {
        attr_str = json_get_value_by_name(buf, size, "time", &attr_len, NULL);
        if (attr_str && utc_time) {
            sscanf(attr_str, "%u", utc_time);
        }
    }

    return ret;
}

void helper_api_test(void)
{
    unsigned int time;
    int ret = alink_get_time(&time);
    assert(!ret);

    printf("get alink utc time: %d\n", time);
}

void awss_demo(void)
{
#if 0
    char ssid[PLATFORM_MAX_SSID_LEN] = { 0 };
    char passwd[PLATFORM_MAX_PASSWD_LEN] = { 0 };
#define WLAN_CONNECTION_TIMEOUT     (30 * 1000) //30 seconds

    /* after system booting, read ssid & passwd from flash */
    vendor_read_ssid_passwd_from_flash(ssid, passwd);

    if (ssid is empty)
        awss_start(); /* Note: awss_start() will block until success */
#endif

    /* call alink_start() after system got IP address */

    /* activate device after alink connnected, see activate_button_pressed */
}

/* activate sample */
char active_data_tx_buffer[128];
#define ActivateDataFormat    "{\"ErrorCode\": { \"value\": \"%d\" }}"
int activate_button_pressed(void)
{
    int errorcode = 0;
    if (0 == errorcode){
         errorcode = 1;
     }else {
         errorcode = 0;
     }
    sprintf(active_data_tx_buffer, ActivateDataFormat, errorcode);
    printf("send:%s", active_data_tx_buffer);
    return alink_report(Method_PostData, (char *)active_data_tx_buffer);
}


enum SERVER_ENV {
    DAILY = 0,
    SANDBOX,
    ONLINE,
    DEFAULT
};
const char *env_str[] = {"daily", "sandbox", "online", "default"};

void usage(void)
{
    printf("\nalink_sample -e enviroment -t work_time -l log_level\n");
    printf("\t -e alink server environment, 'daily', 'sandbox' or 'online'(default)\n");
    printf("\t -t work time, unit is s\n");
    printf("\t -l log level, trace/debug/info/warn/error/fatal/none\n");
    printf("\t -h show help text\n");
}

static int env = DEFAULT;
static char log_level = 0;
extern char *optarg;

void parse_opt(int argc, char *argv[])
{
    char ch;

    while ((ch = getopt(argc, argv, "e:t:l:h")) != -1) {
        switch (ch) {
        case 'e':
            if (!strcmp(optarg, "daily"))
                env = DAILY;
            else if (!strcmp(optarg, "sandbox"))
                env = SANDBOX;
            else if (!strcmp(optarg, "online"))
                env = ONLINE;
            else {
                env = ONLINE;
                printf("unknow opt %s, use default env\n", optarg);
            }
            break;
        case 't':
            work_time = atoi(optarg);
            break;
        case 'l':
            /*
            if (!strcmp(optarg, "trace"))
                log_level = YOS_LL_TRACE;
            else if (!strcmp(optarg, "debug"))
                log_level = ALINK_LL_DEBUG;
            else if (!strcmp(optarg, "info"))
                log_level = ALINK_LL_INFO;
            else if (!strcmp(optarg, "warn"))
                log_level = ALINK_LL_WARN;
            else if (!strcmp(optarg, "error"))
                log_level = ALINK_LL_ERROR;
            else if (!strcmp(optarg, "fatal"))
                log_level = ALINK_LL_FATAL;
            else if (!strcmp(optarg, "none"))
                log_level = ALINK_LL_NONE;
            else
                log_level = ALINK_LL_INFO;
                */
            break;
        case 'h':
        default:
            usage();
            exit(0);
        }
    }

    printf("alink server: %s, work_time: %ds, log level: %d\n",
            env_str[env], work_time, log_level);
}

int application_start(int argc, char *argv[])
{
    parse_opt(argc, argv);
    yos_set_log_level(YOS_LL_DEBUG);
    //awss_demo();
    if (env == SANDBOX)
        alink_enable_sandbox_mode();
    else if (env == DAILY)
        alink_enable_daily_mode(NULL, 0);

    alink_register_callback(ALINK_CLOUD_CONNECTED, &cloud_connected);
    alink_register_callback(ALINK_CLOUD_DISCONNECTED, &cloud_disconnected);
    /*
     * NOTE: register ALINK_GET/SET_DEVICE_STATUS or ALINK_GET/SET_DEVICE_RAWDATA
     */
#ifdef RAW_DATA_DEVICE
    /*
     * TODO: before using callback ALINK_GET/SET_DEVICE_RAWDATA,
     * submit product_model_xxx.lua script to ALINK cloud.
     * ALINKTEST_LIVING_LIGHT_SMARTLED_LUA is done with it.
     */
    alink_register_callback(ALINK_GET_DEVICE_RAWDATA, &cloud_get_device_raw_data);
    alink_register_callback(ALINK_SET_DEVICE_RAWDATA, &cloud_set_device_raw_data);
#else
    alink_register_callback(ALINK_GET_DEVICE_STATUS, &cloud_get_device_status);
    alink_register_callback(ALINK_SET_DEVICE_STATUS, &cloud_set_device_status);
#endif
    alink_register_callback(ALINK_UPGRADE_DEVICE,&callback_upgrade_device);
    alink_register_callback(ALINK_CANCEL_UPGRADE_DEVICE,&callback_cancel_upgrade_device);
    alink_start();

    yos_post_delayed_action(4*1000,main_loop, NULL);
    yos_loop_run();

    //ÔİÆÁ±Î¸´Î»´úÂë£¬±ÜÃâ²âÊÔÖĞÎóÖØÆô
    //alink_factory_reset();
    printf("alink end.\n");
    alink_end();

    return 0;
}

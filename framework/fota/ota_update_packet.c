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
#include <unistd.h>
#include <stdint.h>
#include <md5.h>
#include <stdbool.h>
#include <yos/log.h>
#include <yos/network.h>
#include "ota_constants.h"
#include "ota_log.h"
#include "ota_util.h"
#ifndef CONFIG_CODECS_HASH_MD5
#define CONFIG_CODECS_HASH_MD5
#endif


#include "ota_update_manifest.h"
#include "ota_update_packet.h"

#define OTA_CONFIG_MAXHOSTNAME 40
#define OTA_CONFIG_MAXFILENAME 256
#define OTA_RANGE_LEN 1024

#define min(a, b) (((a) > (b)) ? (b) : (a))

typedef struct {
    int32_t packet_size;
    int32_t processed_size;
    int8_t  file_num;
} packet_format_t;

typedef struct {
    uint32_t file_size;
    uint32_t writed_size;
    uint32_t readed_size;
    enum FILE_TYPE file_type;
} file_format_t;

typedef enum {
    PARSE_PACKET_HEADER,
    PARSE_FILE_HEADER,
    PARSE_FILE_CONTENT,
    PARSE_PACKET_MD5,
} E_PARSE_PREF;

typedef struct {
    int32_t offset;
    int32_t offend;
    E_PARSE_PREF pref;
} http_param_t;

typedef enum {
    REQUEST_OK = 0,
    REQUEST_CONTINUE,
    REQUEST_OVER,
} E_REQUEST_RESULT;

typedef struct _ota_http_head_info{
  int32_t offset;
  int32_t offend;
  int32_t port;
  E_PARSE_PREF pref;
  char hostname[OTA_CONFIG_MAXHOSTNAME];
  char filename[ OTA_CONFIG_MAXFILENAME];
}ota_http_head_info;


static packet_format_t    g_packet_info;   // update packet
static file_format_t      g_file_block;
static ota_http_head_info g_pkg_head_info;
static MD5_CTX            g_ctx;
static E_REQUEST_RESULT   g_request_result = REQUEST_OK;
static int32_t            g_consume_num    = 0;
static bool               g_file_eof       = false;
static E_PARSE_PREF       g_pref           = PARSE_PACKET_HEADER;
static E_PARSE_PREF       g_pref_next      = PARSE_PACKET_HEADER;
static write_flash_cb_t   g_write_flash_cb = NULL;
static int request_packet(void);
static void ota_finish(const char *buffer, int32_t len);
static void clear_header_info(void)
{
    memset(&g_packet_info, 0, sizeof(packet_format_t));
}

static void clear_file_block(void)
{
    memset(&g_file_block, 0, sizeof(file_format_t));
}

static void clear_static_variant(void) {
    clear_header_info();
    clear_file_block();
    g_consume_num    = 0;
    g_request_result = REQUEST_OK;
    g_file_eof       = false;
    g_pref           = PARSE_PACKET_HEADER;
    g_pref_next      = PARSE_PACKET_HEADER;
}

static bool endian_check(void)
{
    short a = 0x1234;
    char *c = (char*)&a;
    if (c[0] == 0x12)
        return 0; // big endian
    else
        return 1;
}

// return 0 if a number of 1 is even
static bool parity_check(const uint32_t num)
{
    uint32_t tmp = num;
    tmp ^= tmp>>16;
    tmp ^= tmp>>8;
    tmp ^= tmp>>4;
    tmp ^= tmp>>2;
    tmp ^= tmp>>1;
    return tmp&1;
}

static uint32_t c2int32(const char* buf)
{
    union {
        char c[4];
        uint32_t num;
    } util;
    if (endian_check()) {
        memcpy(util.c, buf, 4);
    } else {
        int i=0;
        for ( ; i<4; ++i) {
            util.c[3-i] = buf[i];
        }
    }
    return util.num;
}

static uint8_t c2int8(const char* buf)
{
    union {
        char c;
        uint8_t num;
    } util;
    util.c = buf[0];
    return util.num;
}

static enum FILE_TYPE get_file_type(const char* buf)
{
    uint8_t i = c2int8(buf);
    OTA_LOG_I("file_type is %d.\n",i);
    switch(i) {
        case 0x80:
            return MANIFEST;
        case 0x81:
            return BOOTLOADER;
        case 0x82:
            return IMAGE;
#if defined(CONFIG_MODULE_OTA_ENABLE)
        case 0x83:
            return MODULE_APP;
#endif
        default:
            OTA_LOG_E("file_type is wrong.\n");
    }
    return UNDEFINED;
}

static uint32_t get_length(const char* buf)
{
    if(buf == NULL) {
        OTA_LOG_I("buf is empty.\n");
        return 0;
    }
    uint32_t len = c2int32(buf);
    if (parity_check(len)) {
        // odd
        return 0;
    }
    return len >> 1;
}

static bool check_logo(const char* buf)
{
    char logo[8] = {'Y','u','n','O','S','Y','o','C'};
    if(memcmp(logo, buf, 8)) {
        OTA_LOG_E("Logo error!\n");
        return false;
    }
    return true;
}

static bool parse_packet_header(const char *buffer)
{
    if (check_logo(buffer)) {
        g_packet_info.packet_size = get_length(buffer+8);
        g_packet_info.file_num = c2int8(buffer+12);
        OTA_LOG_I("file num in packet:%d, packet size:%d",
                  g_packet_info.file_num, g_packet_info.packet_size);
        return true;
    }
    return false;
}

static bool parse_file_header(const char *buffer)
{
   clear_file_block();
   g_file_block.file_type = get_file_type(buffer);
#if defined(CONFIG_MODULE_OTA_ENABLE)
  // ota_tag.filetype = UNDEFINED;
   if (g_file_block.file_type == MODULE_APP) {

   }
#endif
   if (g_file_block.file_type != UNDEFINED) {
        g_file_block.file_size = get_length(buffer+1);
        if (g_file_block.file_size) {
            OTA_LOG_I("file type;%d, file size:%d", g_file_block.file_type, g_file_block.file_size);
            return true;
        }
   }
   return false;
}

static int32_t get_range(void)
{
    int32_t need = g_file_block.file_size - g_file_block.readed_size;
    return OTA_RANGE_LEN > need ? need : OTA_RANGE_LEN;
}

static bool check_md5(const char *buffer, const int32_t len)
{
    char digest[16] = {0};
    MD5Final((unsigned char*)digest, &g_ctx);
    if (!strncmp(digest, buffer, 16)) {
        OTA_LOG_I("Download update_packet SUCCESS!");
        return true;
    }
    return false;
}

static void set_request_result(E_REQUEST_RESULT result) {
    g_request_result = result;
}

static E_REQUEST_RESULT request_packet_result(void) {
    return g_request_result;
}

static void ota_write_flash(char* buf, int32_t len)
{
    OTA_LOG_I("ota_write_flash, buf_len: %d  %02x %02x", len, buf[0], buf[1]);
    g_write_flash_cb(g_file_block.writed_size, (uint8_t *)buf, len, g_file_block.file_type);
    g_file_block.writed_size += len;
}

static int parse_http_response(E_PARSE_PREF pref, char *buffer, int32_t len)
{
    OTA_LOG_D("parse_http_response, start. buf");
    int ret = 0;
    switch(pref) {
        case PARSE_PACKET_HEADER:
            if(!parse_packet_header(buffer)) {
                OTA_LOG_E("parse_packet_header, error!");
                ret = -1;
            }
            break;
        case PARSE_FILE_HEADER:
            if (!parse_file_header(buffer)) {
                OTA_LOG_E("parse_file_header, error!");
                ret = -1;
            }
            break;
        case PARSE_FILE_CONTENT:
            ota_write_flash(buffer, len);
            break;
        case PARSE_PACKET_MD5:
            ota_finish(buffer, len);
            break;
        default:
            OTA_LOG_E("default, error!");
            ret = -1;
    }
    return  ret;
}

static int read_cb(char *buffer, int readlen, int body)
{
    int32_t range = g_pkg_head_info.offend - g_pkg_head_info.offset + 1;
    E_PARSE_PREF pref = g_pkg_head_info.pref;
    int ret = 0;;
    int vlen = 0;
    OTA_LOG_D("readlen= %d, expect receive: %d - %d,", readlen, g_pkg_head_info.offset, g_pkg_head_info.offend);
    char *valid = strstr(buffer, "\r\n\r\n");
    if(body) {
        valid = buffer;
        range = readlen;
    }
    else if (!valid || valid == buffer) {
        OTA_LOG_E("Not find body.");
        return -1;
    }
    else
        valid += 4;
    vlen = readlen-(valid-buffer);
    OTA_LOG_D("readlen= %d, range: %d vlen %d,", readlen, range, vlen);
    vlen =  min(vlen, range);
    if (pref != PARSE_PACKET_MD5) {
        if (pref == PARSE_FILE_CONTENT) {
            g_file_block.readed_size += range;
        }
        g_packet_info.processed_size += range;
        OTA_LOG_D("read size %d, processed_size:%d", g_file_block.readed_size, g_packet_info.processed_size);
        MD5Update(&g_ctx, (unsigned char*)valid, range);
    }
    ret = parse_http_response(pref, valid, range);
    if (ret < 0){
        set_request_result(REQUEST_CONTINUE);
    }
    return ret;
}

static bool parse_url(const char *url)
{
    char *offset = strstr(url, "://");
    char *port;
    if(!offset)
    {
        OTA_LOG_E("URL illegal1.");
        return -1;
    }
    offset += 3;
    char *datend = strstr(offset, "/");
    if (!datend || strlen(datend) > OTA_CONFIG_MAXFILENAME-1) {
        OTA_LOG_E("URL illegal2.");
        return false;
    }
//    if ((datend-offset) > OTA_CONFIG_MAXHOSTNAME -1) {
//        OTA_LOG_E("URL illegal3.");
//        return -1;
//    }
    port = strrchr(offset, ':');
    if (port) {
        g_pkg_head_info.port = atoi(port+1);
    }
    else {
        port = datend;
        g_pkg_head_info.port = 80;
    }
    strncpy(g_pkg_head_info.hostname, offset, port-offset);
    strcpy(g_pkg_head_info.filename, datend);

    return 0;
}

static int ota_send_request(void){
    int ret = 0;
    int cur = 0;
    int fd = ota_http_init(g_pkg_head_info.hostname, g_pkg_head_info.port);
    if(-1 == fd) {
        OTA_LOG_D("ota_send_request connnect faield");
        return -1;
    }

    int hlen = OTA_HEDAER_MAX_LEN;
    char *header = malloc(hlen);
    if(!header){
        OTA_LOG_D("ota_send_request malloc http header faield");
        free(header);
        return -1;
    }
    memset(header, 0 , OTA_HEDAER_MAX_LEN);

    char range[32];

    /* Updated by laogong for optimizing downloading */
    if(g_pkg_head_info.pref != PARSE_FILE_CONTENT)
        sprintf(range, "Range: bytes=%d-%d\r\n", g_pkg_head_info.offset, g_pkg_head_info.offend);
    else
        sprintf(range, "Range: bytes=%d-%d\r\n", g_pkg_head_info.offset, g_packet_info.packet_size + 12);
    /* Ended laogong */

    snprintf(header, hlen, "GET %s HTTP/1.1\r\n", g_pkg_head_info.filename);
    strcat(header, "Host: ");
    strcat(header, g_pkg_head_info.hostname);
    strcat(header,  "\r\n");
    strcat(header, "Accept: */* \r\n");
    strcat(header, range);
    strcat(header, "Connection: keep-alive\r\n");
    strcat(header, "Cache-Control: no-cache\r\n");
    strcat(header, "User-Agent: Mozilla/5.0\r\n");
    strcat(header, "\r\n");

    OTA_LOG_D("ota_send_request header request: %s", header);
    ret = ota_write(fd, (void*)header, strlen(header));
    OTA_LOG_D("ota_send_request return result: %d", ret);
    free(header);

    /**
     * updated by laogong
     * Optimized request speed, use one http sync mostly
     * this logic is fine in single file OTA model
     * if it is multi-files OTA, it has problems
     * so far i checked the old code, it will cause problems in multi-files too,
     * somethat we need to develop more code for the multi-files OTA
    */
    /* Optimize logic begins */
    char *read_buf = malloc(OTA_READ_BUF_LEN);
    if(!read_buf){
        OTA_LOG_D("ota_send_request malloc read buf failed");
        return -1;
    }

    /* Just figure to PARSE_FILE_CONTENT, no bussiness with other request process */
    if(g_pkg_head_info.pref == PARSE_FILE_CONTENT) {
        memset(read_buf, 0 , 2048);

        /*(1) use ota_read as it is a nice funtion to treat \r\n\r\b at beginning of one http request*/
        ret = ota_read(fd, read_buf, OTA_READ_BUF_LEN, g_pkg_head_info.offend - g_pkg_head_info.offset);
        if(ret < 0) goto exit;

        /*(2) find \r\n\r\n position */
        char *valid = strstr(read_buf, "\r\n\r\n");
        do {
            /*(3) calculate body length, then move valid to the first char of the body
             *    HTTP_RESPONSE is like: [HEADER]\r\n\r\n[BODY]
                  if valid is NULL, valid play role of read_buf
            */
            if(valid) {
                ret = ret - (valid - read_buf) - 4;
                valid = valid + 4;
            }
            else valid = read_buf;

            /*(4) Watch this!
             *    The end of the pkg is MD5, 16-Bytes
             *    pkg consist like: [logo][header][file][md5]
             *    so we have to avoid the md5 bytes, drop it!
             *    cur+ret+1 means the next length, if it is the last buffer
             *    let ret be as long as the file without md5 on the tail
             */
            if(cur + ret + 1 >= g_file_block.file_size)
                ret = g_file_block.file_size - cur;

            /* (5) Write to flash, do md5 update */
            ota_write_flash(valid, ret);
            MD5Update(&g_ctx, (unsigned char *)valid, ret);
            valid = NULL;
            cur += ret;

            /* (6) Judge if finished! if cur equals file_size, we got finish*/
            if(cur == g_file_block.file_size) {
                OTA_LOG_D("\n\n************* OTA BODY FINISHED !*******************\n");
                /* Here no need to compare md5, factory companies always do this */
                char digest[16] = {0};
                MD5Final((unsigned char*)digest, &g_ctx);
#ifdef  CONFIG_MODULE_OTA_ENABLE
                OTA_LOG_D("digestMD5=");
                int i= 0;
                for(; i< 16 ;i++)
                    printf("%02X",digest[i]);
                OTA_LOG_D("digestMD5=END");
#endif
                ota_set_status(E_OTA_DOWNLOAD_SUC);
                break;
            }

            /* read once more time*/
            memset(read_buf, 0 , 2048);
            do {
                ret = read(fd, read_buf, OTA_READ_BUF_LEN);
                if (ret > 0) break;
                if (ret <= 0 && errno != EINTR) goto exit;
            } while (1);
        }
        while(1);
    }
    else {
        /* The original logic remains all the same*/
        memset(read_buf, 0 , 2048);
        ret = ota_read(fd, (void*)read_buf, OTA_READ_BUF_LEN, g_pkg_head_info.offend - g_pkg_head_info.offset);
        OTA_LOG_D("ota_send_request ota first read buf: len:%d",ret);
        ret = read_cb(read_buf, ret, 0);
    }
exit:
    free(read_buf);
    close(fd);
    return ret;
}


static int request_packet(void)
{
    int ret = 0;
    OTA_LOG_D("request_packet, start");
    if (request_packet_result() == REQUEST_OK) {
        g_pref = g_pref_next;
    }
    OTA_LOG_D("finish condition: filename:%d packet_size:%d, processed_size:%d ",g_packet_info.file_num, g_packet_info.packet_size, g_packet_info.processed_size);
    if (!g_packet_info.file_num &&
        g_packet_info.packet_size + 12 == g_packet_info.processed_size + 16) {
        g_pref = PARSE_PACKET_MD5;
    }
    set_request_result(REQUEST_OK);

    int32_t range = 0;
    switch(g_pref) {
        case PARSE_PACKET_HEADER:
            range = 13;
            g_pref_next = PARSE_FILE_HEADER;
            break;
        case PARSE_FILE_HEADER:
            range = 5;
            g_pref_next = PARSE_FILE_CONTENT;
            break;
        case PARSE_FILE_CONTENT:
            range = get_range();
            OTA_LOG_D("range:%d", range);
            if (range < (g_file_block.file_size-g_file_block.readed_size)) {
                g_file_eof = false;
                g_pref_next = PARSE_FILE_CONTENT;
            } else {
                g_file_eof = true;
                g_pref_next = PARSE_FILE_HEADER;
                --g_packet_info.file_num;
            }
            break;
        case PARSE_PACKET_MD5:
            range = 16;
            g_pref_next = PARSE_PACKET_HEADER;
            break;
        default:
            goto request_exit;
    }
    g_pkg_head_info.pref = g_pref;
    g_pkg_head_info.offset = g_packet_info.processed_size;
    g_pkg_head_info.offend = g_pkg_head_info.offset + range -1;
    ret = ota_send_request();
    if( ret < 0) {
        OTA_LOG_E("request_packet send_request return < 0");
        goto request_exit;
    }
    OTA_LOG_E("request_packet out");
    return 0;
request_exit:
    ota_set_status(E_OTA_DOWNLOAD_FAIL);
    OTA_LOG_E("request_packet error out");
    return -1;
}

int get_update_packet(const char* url, write_flash_cb_t func)
{
    int ret = 0;
    OTA_LOG_D("get_update_packet in. url:%s",url);
    if(NULL == func) {
        OTA_LOG_E("get_update_packet write flash cb is null!");
        return -1;
    }
    g_write_flash_cb = func;
    clear_static_variant();
    ret = parse_url(url);
    if (ret) {
        goto error_exit;
    }
    MD5Init(&g_ctx);
    while( E_OTA_DOWNLOAD_SUC != ota_get_status()
           && E_OTA_DOWNLOAD_FAIL != ota_get_status()){
        ret = request_packet();
    }
    OTA_LOG_I("ota_get_status()=%d\n",ota_get_status());
    return ret;

error_exit:
    ota_set_status(E_OTA_DOWNLOAD_FAIL);
    return -1;
}

#ifdef CONFIG_YOS_UT
void inject_ota_finish(const char *buffer, int32_t len)
{
   ota_finish(buffer,len);
}
#endif

static void ota_finish(const char *buffer, int32_t len)
{
    bool is_success = check_md5(buffer, len);
    OTA_LOG_I("ota_finish:%d", is_success);
    clear_static_variant();
    if ( is_success) {
        OTA_LOG_I("\n******write_flash finish. next: palse reboot system******");
        ota_set_status(E_OTA_DOWNLOAD_SUC);
    } else {
        ota_set_status(E_OTA_DOWNLOAD_FAIL);
    }
}


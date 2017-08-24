/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "yos/log.h"
#include <yos/cloud.h>
#include <yos/kernel.h>
#include "platform.h"
#include "wifimonitor.h"

#ifndef Method_PostData
#define Method_PostData "postDeviceData"
#endif
#define post_data_buffer_size (512)
static char post_data_buffer[post_data_buffer_size];

#define MODULE_NAME "wifimonitor"
#ifndef MAC_KV_NAME
#define MAC_KV_NAME "mac_sum"
#endif

#define MAC_HASH_IDX_BYTES 1
#define MAC_HASH_TABLE_SIZE (1<<(8*MAC_HASH_IDX_BYTES))
/*
 * low 2 bytes as hash index, store high 4 bytes
 * |--------|
 * |  head1 | -> mac_hash_t node -> node -> null
 * |  head2 | -> node -> null
 * |  head3 | -> null
 * |  ...   |
 * | head256| -> node -> null
 *
 * table size: 4B * 2 ^ 8
 */
struct mac_hash_t {
    struct mac_hash_t * next;
    uint8_t high_mac[6-MAC_HASH_IDX_BYTES];
}*g_mac_table[MAC_HASH_TABLE_SIZE] = {NULL};

static int g_mac_sum = 0;

static void free_mac_table()
{
    int tbl_idx;
    struct mac_hash_t *h, *tmp;

    for (tbl_idx = 0; tbl_idx < MAC_HASH_TABLE_SIZE; tbl_idx++) {
        h = g_mac_table[tbl_idx];
        while (h != NULL) {
            tmp = h;
            h = h->next;
            yos_free(tmp);
        }
    }

    return;
}

static void wifimonitor_wifi_mgnt_frame_callback(uint8_t *buffer, int length, char rssi);
void handle_count_mac_cmd(char *pwbuf, int blen, int argc, char **argv)
{
    int val = 0;
    const char *rtype= argc > 1 ? argv[1] : "";

    if (strcmp(rtype, "stop") == 0) {
        LOG("Will stop the mac count process.");
        /*
         * We simply register a NULL mgnt frame callback here to stop mac count process.
         * Later we can create a new hal, e.g. named as hal_wlan_get_mgnt_monitor_cb, to
         * save the old mgnt frame callback (if any) so that we can save it. After the mac
         * count process is stopped, we can resume the old mgnt frame callback. A typical
         * example of the old mngt frame callback is the registrar's.
         * <TODO>
         */
        hal_wlan_register_mgnt_monitor_cb(NULL, NULL);
        free_mac_table();
        return;
    } else if (strcmp(rtype, "suspend") == 0) {
        LOG("Will suspend the mac count process");
        hal_wlan_register_mgnt_monitor_cb(NULL, NULL);
    } else if (strcmp(rtype, "resume") == 0) {
        LOG("Will resume the mac count process.");
        hal_wlan_register_mgnt_monitor_cb(NULL, wifimonitor_wifi_mgnt_frame_callback);
    } else {
        g_mac_sum = 0;
        memset(g_mac_table, 0, sizeof(g_mac_table));
        LOG("Will start the mac count process.");
        hal_wlan_register_mgnt_monitor_cb(NULL, wifimonitor_wifi_mgnt_frame_callback);
    }
}

/*
 * This command will register a mgnt frame callback, which will overwrite 
 * the registrar callback, to handle wifi probe req frame. And the wifi monitor 
 * needs to be in open state.
 */
struct cli_command count_mac_cmd = {
    .name = "count_mac",
    .help = "count_mac [start|stop|suspend|resume]",
    .function = handle_count_mac_cmd
};

static int compare_high_mac(uint8_t * mac1, uint8_t * mac2, int len)
{
    while (len) {
        if (mac1[len-1] != mac2[len-1]) return 1;
        len--;
    }

    return 0;
}

#ifndef MAC_KV_NAME
#define MAC_KV_NAME "mac_sum"
#endif

static int check_same_mac_and_add_new(uint8_t * mac)
{
    struct mac_hash_t * node, * tail;
    uint8_t entry = *mac & 0xff;
    struct mac_hash_t * new_n;

    node = g_mac_table[entry];
    tail = g_mac_table[entry];

    while (node != NULL) {
        if (0 == compare_high_mac(node->high_mac,
          mac+MAC_HASH_IDX_BYTES, 6-MAC_HASH_IDX_BYTES)) {
            return 1;
        }
        tail = node;
        node = node->next;
    }

    new_n = (struct mac_hash_t *)yos_malloc(sizeof(struct mac_hash_t));
    LOGD(MODULE_NAME, "New mac (%02x:%02x:%02x:%02x:%02x:%02x) found, let's add it.",
      mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    if (new_n == NULL) {
        LOGI(MODULE_NAME, "Error: yos_malloc failed");
        return -1;
    }
    new_n->next = NULL;
    memcpy(new_n->high_mac, mac+MAC_HASH_IDX_BYTES, 6-MAC_HASH_IDX_BYTES);
    if (tail == NULL)  g_mac_table[entry] = new_n;
    else tail->next = new_n;

    g_mac_sum++;
    LOGD(MODULE_NAME, "g_mac_sum is %d", g_mac_sum);

    return 0;
}

#define Method_PostNewProbeReqData "postDeviceData"
#define PostMacDataFormat "{\"NewProbeReqMac\":{\"value\":\"%02x:%02x:%02x:%02x:%02x:%02x\"}, \"rssi\":{\"value\":\"%d\"}}"
static void report_new_mac(const uint8_t *mac, char rssi)
{
    if (!mac) return;

    snprintf(post_data_buffer, post_data_buffer_size, PostMacDataFormat,
      mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], rssi);
    LOG("Start report new found mac. %s", post_data_buffer);
    yos_cloud_report(Method_PostNewProbeReqData, post_data_buffer, NULL, NULL);
}

#ifndef MGMT_PROBE_REQ
#define MGMT_PROBE_REQ  (0x40)
#endif
static void wifimonitor_wifi_mgnt_frame_callback(uint8_t *buffer, int length, char rssi)
{
    int type = buffer[0];
    uint8_t * mac = &buffer[10]; // SA in the probe req frame

    switch (type) {
    case MGMT_PROBE_REQ:
        if (rssi < -60) return;
        if (check_same_mac_and_add_new(mac) == 0) report_new_mac(mac, rssi);
        break;
    default:
        break;
    }
}

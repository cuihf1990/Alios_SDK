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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <lwip/def.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>

#include <yos/framework.h>
#include <yos/log.h>
#include <yos/list.h>
#include <lwip/sockets.h>
#include <yos/cloud.h>
#include "umesh.h"
#include "cJSON.h"
#include "core/mesh_mgmt.h"
#include "devmgr.h"

#include "mqtt_sn.h"
#include "gateway_service.h"

#define MODULE_NAME "gateway"

typedef struct reg_info_s {
    char model_id[sizeof(uint32_t) + 1];
    unsigned char ieee_addr[IEEE_ADDR_BYTES + 1];
    char rand[SUBDEV_RAND_BYTES + 1];
    char sign[STR_SIGN_LEN + 1];
} reg_info_t;

typedef struct {
    dlist_t next;
    reg_info_t *reginfo;
    dev_info_t *devinfo;
    struct sockaddr_in6 addr;
} client_t;

typedef struct {
    int sockfd;
    bool gateway_mode;
    bool yunio_connected;
    bool mesh_connected;
    bool mqtt_connected;
    struct sockaddr_in6 gw_addr;
    struct sockaddr_in6 src_addr;
    dlist_t clients;
    dlist_t callbacks;
} gateway_state_t;

static gateway_state_t gateway_state;

char *config_get_main_uuid(void);
static void gateway_service_event(input_event_t *eventinfo, void *priv_data);

static char *get_uuid_by_ip6addr(struct sockaddr_in6 *addr)
{
    client_t *client = NULL;
    dlist_for_each_entry(&gateway_state.clients, client, client_t, next) {
        if (memcmp(addr, &client->addr, sizeof(client->addr)) == 0) {
            return client->devinfo->dev_base.uuid;
        }
    }
    return NULL;
}

static struct sockaddr_in6 *get_ip6addr_by_uuid(const char *uuid)
{
    client_t *client = NULL;
    dlist_for_each_entry(&gateway_state.clients, client, client_t, next) {
        if (memcmp(uuid, client->devinfo->dev_base.uuid, STR_UUID_LEN) == 0) {
            return &client->addr;
        }
    }
    return NULL;
}

static int yos_cloud_get_attr(const char* uuid, const char *attr_name)
{
    struct sockaddr_in6 *paddr = NULL;
    paddr = get_ip6addr_by_uuid(uuid);
    if (paddr == NULL)
        return -1;

    void *buf;
    int len;
    pub_body_t *pub_body = msn_alloc(PUBLISH, 4 + strlen(attr_name)+1, &buf, &len);
    memcpy(pub_body->payload, "get", 4);
    memcpy(pub_body->payload + 4, attr_name, strlen(attr_name) + 1);

    sendto(gateway_state.sockfd, buf, len, MSG_DONTWAIT,
           (struct sockaddr *)paddr, sizeof(*paddr));

    free(buf);
    return 0;
}

static int yos_cloud_set_attr(const char *uuid, const char *attr_name, const char *attr_value)
{
    struct sockaddr_in6 *paddr = NULL;
    paddr = get_ip6addr_by_uuid(uuid);
    if (paddr == NULL)
        return -1;

    void *buf;
    int len;
    pub_body_t *pub_body = msn_alloc(PUBLISH, 4+strlen(attr_name)+1+strlen(attr_value)+1, &buf, &len);
    memcpy(pub_body->payload, "set", 4);
    memcpy(pub_body->payload + 4, attr_name, strlen(attr_name) + 1);
    memcpy(pub_body->payload + 4 + strlen(attr_name) + 1, attr_value, strlen(attr_value) + 1);

    sendto(gateway_state.sockfd, buf, len, MSG_DONTWAIT,
           (struct sockaddr *)paddr, sizeof(*paddr));

    free(buf);
    return 0;
}

static void connect_to_gateway(gateway_state_t *pstate, struct sockaddr_in6 *paddr)
{
    void *buf;
    int len;
    reg_info_t *reginfo;
    const mac_address_t *mac_addr;
    uint32_t model_id = 0x0050099a;

    LOGD(MODULE_NAME, "connect to new gateway");
    reginfo = (reg_info_t *)msn_alloc(CONNECT, sizeof(reg_info_t), &buf, &len);
    memset(reginfo, 0, sizeof(reg_info_t));
    memcpy(reginfo->model_id, &model_id, sizeof(model_id));
    mac_addr = ur_mesh_net_get_mac_address(UR_MESH_NET_DFL);
    memcpy(reginfo->ieee_addr, mac_addr->addr, IEEE_ADDR_BYTES);
    memcpy(reginfo->rand,"randrandrandrand", sizeof(reginfo->rand));
    devmgr_get_device_signature(model_id, reginfo->rand, reginfo->sign, sizeof(reginfo->sign));

    sendto(pstate->sockfd, buf, len, MSG_DONTWAIT,
           (struct sockaddr *)paddr, sizeof(*paddr));

    free(buf);
}

static void handle_adv(gateway_state_t *pstate, void *pmsg, int len)
{
    LOGD(MODULE_NAME, "handle_adv");
    if (pstate->gateway_mode) {
        return;
    }

    adv_body_t *adv_msg = pmsg;
    struct sockaddr_in6 *gw_addr = &pstate->gw_addr;

    if (pstate->mqtt_connected &&
        memcmp(&gw_addr->sin6_addr, adv_msg->payload, sizeof(gw_addr->sin6_addr)) == 0)
        return;

    memcpy(&gw_addr->sin6_addr, adv_msg->payload, sizeof(gw_addr->sin6_addr));
    gw_addr->sin6_family = AF_INET6;
    gw_addr->sin6_port = htons(MQTT_SN_PORT);
    connect_to_gateway(pstate, &pstate->gw_addr);
}


static void handle_publish(gateway_state_t *pstate, void *pmsg, int len)
{
    pub_body_t *pub_msg = pmsg;
    const char *op_code = (const char *)(pub_msg + 1);
    const char *attr_name = op_code + 4;
    const char *attr_value = attr_name + strlen(attr_name) + 1;
    if (strcmp(op_code, "rpt") == 0) {
        if (!pstate->gateway_mode) {
            LOGE(MODULE_NAME, "error: cloud report data received at non-gateway node");
            return;
        }

        const char *uuid = get_uuid_by_ip6addr(&pstate->src_addr);
        if (uuid == NULL) {
            LOGE(MODULE_NAME, "error: cloud report data from unconnected node");
            return;
        }

#define POST_ATTR_VALUE_STRING_FMT      "{\"uuid\":\"%s\",%s"
        char *buf;
        int sz = sizeof(POST_ATTR_VALUE_STRING_FMT) + strlen(uuid) + strlen(attr_value) + 16;

        buf = yos_malloc(sz);
        /* tricky : attr_value = {"key":"value"} -> {"uuid":"xx", "key":"value"} */
        snprintf(buf, sz, POST_ATTR_VALUE_STRING_FMT, uuid, attr_value+1);
        alink_report_async("postDeviceData", buf, NULL, NULL);
        yos_free(buf);
        return;
    }

    if (strcmp(op_code, "get") == 0) {
        LOGD(MODULE_NAME, "recv %s - %s", op_code, attr_name);
        yos_cloud_trigger(GET_DEVICE_STATUS, attr_name);
    } else if (strcmp(op_code, "set") == 0) {
        LOGD(MODULE_NAME, "recv %s - %s - %s", op_code, attr_name, attr_value);
        yos_cloud_trigger(SET_DEVICE_STATUS, attr_name);
    } else {
        const char *payload = (const char *)(pub_msg+1);
        LOGD(MODULE_NAME, "recv unkown %s", payload);
    }
}

static client_t *new_client(gateway_state_t *pstate, reg_info_t *reginfo)
{
    client_t *client = NULL;
    dlist_for_each_entry(&pstate->clients, client, client_t, next) {
        if (memcmp(reginfo->ieee_addr, client->devinfo->dev_base.u.ieee_addr, IEEE_ADDR_BYTES) == 0) {
            LOGD(MODULE_NAME, "existing client %s", client->devinfo->dev_base.u.ieee_addr);
            return client;
        }
    }

    client = malloc(sizeof(*client));
    PTR_RETURN(client, NULL, "alloc memory failed");
    bzero(client, sizeof(*client));
    client->reginfo = malloc(sizeof(reg_info_t));
    PTR_RETURN(client->reginfo, NULL, "alloc memory failed");
    memcpy(client->reginfo, reginfo, sizeof(reg_info_t));
    uint32_t model_id;
    memcpy(&model_id, reginfo->model_id, sizeof(model_id));
    int ret = devmgr_join_zigbee_device(reginfo->ieee_addr, model_id, reginfo->rand, reginfo->sign);
    if (ret ==  SERVICE_RESULT_ERR) {
        LOGD(MODULE_NAME, "register device:%s to alink server failed", reginfo->ieee_addr);
        free(client->reginfo);
        free(client);
        return NULL;
    }
    client->devinfo = devmgr_get_devinfo_by_ieeeaddr(client->reginfo->ieee_addr);
    free(client->reginfo);
    dlist_add_tail(&client->next, &pstate->clients);

    LOGD(MODULE_NAME, "new client %s", client->reginfo->ieee_addr);

    return client;
}

static void handle_connect(gateway_state_t *pstate, void *pmsg, int len)
{
    client_t *client = NULL;
    if (!pstate->gateway_mode) {
        LOGE(MODULE_NAME, "error recv CONNECT cmd!");
        return;
    }

    if (pstate->yunio_connected) {
        client = new_client(pstate, (reg_info_t *)pmsg);
        if (client != NULL)
            memcpy(&client->addr, &pstate->src_addr, sizeof(client->addr));
    }

    void *buf;
    conn_ack_t *conn_ack = msn_alloc(CONNACK, 0, &buf, &len);
    conn_ack->ReturnCode = (client == NULL) ? -1 : 0;
    sendto(pstate->sockfd, buf, len, MSG_DONTWAIT,
           (struct sockaddr *)&pstate->src_addr, sizeof(pstate->src_addr));
    free(buf);
}

static int gateway_cloud_report(const char *method, const char *json_buffer)
{
    gateway_state_t *pstate = &gateway_state;

    if (pstate->yunio_connected) {
        LOGE(MODULE_NAME, "strange, yunio is ready!");
        return 0;
    }

    if (!pstate->mqtt_connected) {
        LOGE(MODULE_NAME, "mqtt not ready!");
        return -1;
    }

    void *buf;
    int len;
    struct sockaddr_in6 *paddr = &pstate->gw_addr;
    int sz = 4+strlen(method)+1+strlen(json_buffer)+1;
    pub_body_t *pub_body = msn_alloc(PUBLISH, sz, &buf, &len);

    memcpy(pub_body->payload, "rpt", 4);
    memcpy(pub_body->payload + 4, method, strlen(method) + 1);
    memcpy(pub_body->payload + 4 + strlen(method) + 1, json_buffer, strlen(json_buffer) + 1);

    sendto(pstate->sockfd, buf, len, MSG_DONTWAIT,
           (struct sockaddr *)paddr, sizeof(*paddr));

    free(buf);
}

static void handle_connack(gateway_state_t *pstate, void *pmsg, int len)
{
    if (pstate->gateway_mode) {
        LOGE(MODULE_NAME, "error recv CONNACK cmd!");
        return;
    }

    conn_ack_t *conn_ack = pmsg;
    if (conn_ack->ReturnCode != 0) {
        LOGE(MODULE_NAME, "connect fail %d!", conn_ack->ReturnCode);
        return;
    }

    pstate->mqtt_connected = true;

    yos_cloud_register_backend(&gateway_cloud_report);
    yos_cloud_trigger(CLOUD_CONNECTED, NULL);

    LOGD(MODULE_NAME, "connack");
}

static void handle_msg(gateway_state_t *pstate, uint8_t *pmsg, int len)
{
    uint8_t msg_type = *pmsg++;
    len --;
    switch (msg_type) {
    case PUBLISH:
        handle_publish(pstate, pmsg, len);
        break;
    case CONNECT:
        handle_connect(pstate, pmsg, len);
        break;
    case CONNACK:
        handle_connack(pstate, pmsg, len);
        break;
    case ADVERTISE:
        handle_adv(pstate, pmsg, len);
        break;
    }
}

static void gateway_sock_read_cb(int fd, void *priv)
{
    gateway_state_t *pstate = priv;
#define BUF_LEN 256
    uint8_t buf[BUF_LEN], *pmsg;
    int len;
    socklen_t slen = sizeof(pstate->src_addr);

    len = recvfrom(pstate->sockfd, buf, BUF_LEN,
                   MSG_DONTWAIT,
                   (struct sockaddr *)&pstate->src_addr, &slen);
    if (len <= 2) {
        LOGE(MODULE_NAME, "error read count %d", len);
        return;
    }

    len = msn_parse_header(buf, len, &pmsg);
    if (len < 0) {
        LOGE(MODULE_NAME, "error parsing header %x", buf[0]);
        return;
    }

    handle_msg(pstate, pmsg, len);
}

static void gateway_advertise(void *arg)
{
    gateway_state_t *pstate = arg;
    void *buf;
    int len;
    adv_body_t *adv;

    yos_post_delayed_action(10 * 1000, gateway_advertise, arg);

    ur_ip6_addr_t *mcast_addr = (ur_ip6_addr_t *)ur_mesh_get_mcast_addr();
    ur_ip6_addr_t *ucast_addr = (ur_ip6_addr_t *)ur_mesh_get_ucast_addr();
    if (!mcast_addr || !ucast_addr) {
        return;
    }

    adv = msn_alloc(ADVERTISE, sizeof(*ucast_addr), &buf, &len);
    bzero(adv, sizeof(*adv));
    memcpy(adv->payload, ucast_addr, UR_IP6_ADDR_SIZE);

    struct sockaddr_in6 addr;
    bzero(&addr, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(MQTT_SN_PORT);
    memcpy(&addr.sin6_addr, mcast_addr, sizeof(addr.sin6_addr));

    sendto(pstate->sockfd, buf, len, MSG_DONTWAIT,
           (struct sockaddr *)&addr, sizeof(addr));
    LOGD(MODULE_NAME, "gateway_advertise");

    free(buf);
}

int gateway_service_init(void)
{
    gateway_state_t *pstate = &gateway_state;

    pstate->gateway_mode = false;
    pstate->yunio_connected = false;
    pstate->mesh_connected = false;
    pstate->mqtt_connected = false;
    pstate->sockfd = -1;
    dlist_init(&gateway_state.clients);
    yos_register_event_filter(EV_YUNIO, gateway_service_event, NULL);
    yos_register_event_filter(EV_MESH, gateway_service_event, NULL);
    return 0;
}

void gateway_service_deinit(void)
{
    yos_unregister_event_filter(EV_YUNIO, gateway_service_event, NULL);
    yos_unregister_event_filter(EV_MESH, gateway_service_event, NULL);
}

#define GATEWAY_WORKER_THREAD
#ifdef GATEWAY_WORKER_THREAD
static void gateway_worker(void *arg)
{
    int maxfd = gateway_state.sockfd;

    while(1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(gateway_state.sockfd, &rfds);

        int ret = lwip_select(maxfd+1, &rfds, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno != EINTR) {
                LOGE(MODULE_NAME, "select error %d, quit", errno);
                break;
            }
        }
        if (FD_ISSET(gateway_state.sockfd, &rfds))
            gateway_sock_read_cb(gateway_state.sockfd, &gateway_state);
    }
    LOGE(MODULE_NAME, "return");
}
#endif

static int init_socket(void)
{
    gateway_state_t *pstate = &gateway_state;
    struct sockaddr_in6 addr;
    int sockfd;
    int ret;

    sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        LOGE(MODULE_NAME, "error open socket %d", errno);
        return -1;
    }

    if (pstate->sockfd >= 0) {
#ifndef GATEWAY_WORKER_THREAD
        yos_cancel_poll_read_fd(pstate->sockfd, gateway_sock_read_cb, pstate);
#endif
        close(pstate->sockfd);
    }

    int val = 1;
    setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &val, sizeof(val));
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(MQTT_SN_PORT);
    ret = bind(sockfd, (const struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        LOGE(MODULE_NAME, "error bind socket %d", errno);
        close(sockfd);
        return -1;
    }

    pstate->sockfd = sockfd;
#ifdef GATEWAY_WORKER_THREAD
    yos_task_new("gatewayworker", gateway_worker, NULL, 4096);
#else
    yos_poll_read_fd(sockfd, gateway_sock_read_cb, pstate);
#endif

    return 0;
}

#include "json_parser.h"
#define MAX_UUID_LEN        33
#define JSON_KEY_UUID       "uuid"
static void gateway_handle_sub_status(int event, const char *json_buffer)
{
    char *str_pos = NULL;
    int str_len = 0;
    char uuid[MAX_UUID_LEN] = {0};

    str_pos = json_get_value_by_name((char *)json_buffer, strlen(json_buffer), JSON_KEY_UUID, &str_len, NULL);
    strncpy(uuid, str_pos, str_len);

    if (event == GET_SUB_DEVICE_STATUS)
        yos_cloud_get_attr(uuid, json_buffer);
    else if (event == SET_SUB_DEVICE_STATUS)
        yos_cloud_set_attr(uuid, json_buffer, "");
}

int gateway_service_start(void)
{
    init_socket();

    if (gateway_state.gateway_mode) {
        yos_cloud_register_callback(GET_SUB_DEVICE_STATUS, gateway_handle_sub_status);
        yos_cloud_register_callback(SET_SUB_DEVICE_STATUS, gateway_handle_sub_status);

        yos_cancel_delayed_action(-1, gateway_advertise, &gateway_state);
        yos_post_delayed_action(5 * 1000, gateway_advertise, &gateway_state);
    }

    return 0;
}

void gateway_service_stop(void) {
    close(gateway_state.sockfd);
    gateway_state.sockfd = -1;
    gateway_state.mqtt_connected = false;
}

static void gateway_service_event(input_event_t *eventinfo, void *priv_data)
{
    if (eventinfo->type == EV_YUNIO) {
        if(eventinfo->code == CODE_YUNIO_ON_CONNECTED)
            gateway_state.yunio_connected = true;
        else
            return;
    }

    if (eventinfo->type == EV_MESH) {
        if (eventinfo->code == CODE_MESH_CONNECTED)
            gateway_state.mesh_connected = true;
        else if (eventinfo->code == CODE_MESH_DISCONNECTED)
            gateway_state.mesh_connected = false;
        else
            return;
    }

    if (ur_mesh_get_device_state() == DEVICE_STATE_LEADER && gateway_state.yunio_connected == true)
        gateway_state.gateway_mode = true;

    if (gateway_state.mesh_connected == true)
        gateway_service_start();
    else
        gateway_service_stop();
}


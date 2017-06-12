#include <string.h>

#include <yos/framework.h>
#include <yos/log.h>
#include <yos/list.h>
#include <yos/network.h>
#include <tfs.h>
#include "umesh.h"
#include "cJSON.h"

#include "mqtt_sn.h"

#define ME "GW_SERVICE"

typedef struct {
    dlist_t next;
    char id2[TFS_ID2_LEN + 1];
    char idsu[16 + TFS_ID2_LEN + 1];
    struct sockaddr_in6 addr;
} client_t;

typedef struct {
    int sockfd;
    bool gateway_mode;
    bool yunio_connected;
    bool mqtt_connected;
    struct sockaddr_in6 gw_addr;
    struct sockaddr_in6 src_addr;
    dlist_t clients;
} gateway_state_t;

static gateway_state_t gateway_state;
int link_negotiate_protocol(void);
int link_device_register(void);
int link_push(cJSON *data);
int link_execute(const char *idsu, int sid, cJSON *payload);
typedef void (*yos_free_cb)(void *private_data);
typedef int (*yos_link_cb)(const char *idsu, const char *payload, void *private_data);

int link_subscribe(const char *idsu, yos_link_cb cb, yos_free_cb free_cb, void *private_data)
{
    return -1;
}

int link_publish(const char *idsu, const char *paylaod)
{
    return -1;
}

int yos_cloud_subscribe(const char *idsu, yos_link_cb cb, yos_free_cb free_cb, void *private_data)
{
    return link_subscribe(idsu, cb, free_cb, private_data);
}

int yos_cloud_publish(const char *idsu, const char *payload)
{
    gateway_state_t *pstate = &gateway_state;

    if (pstate->yunio_connected) {
        return link_publish(idsu, payload);
    }

    if (!pstate->mqtt_connected) {
        LOGE(ME, "mqtt not ready!");
        return -1;
    }

    void *buf;
    int len;
    struct sockaddr_in6 *paddr = &pstate->gw_addr;
    pub_body_t *pub_body = msn_alloc(PUBLISH, strlen(idsu)+1+strlen(payload)+1, &buf, &len);

    memcpy(pub_body->payload, idsu, strlen(idsu) + 1);
    memcpy(pub_body->payload + strlen(idsu) + 1, payload, strlen(payload) + 1);

    sendto(pstate->sockfd, buf, len, MSG_DONTWAIT,
           (struct sockaddr *)paddr, sizeof(*paddr));

    free(buf);
    return 0;
}

static int client_msg_link_cb(const char *idsu, const char *payload, void *private_data)
{
    void *buf;
    int len;
    client_t *client = private_data;
    gateway_state_t *pstate = &gateway_state;
    pub_body_t *pub_body = msn_alloc(PUBLISH, strlen(payload)+1, &buf, &len);

    memcpy(pub_body->payload, payload, strlen(payload) + 1);

    sendto(pstate->sockfd, buf, len, MSG_DONTWAIT,
           (struct sockaddr *)&client->addr, sizeof(client->addr));

    free(buf);
    return 0;
}

static void connect_to_gateway(gateway_state_t *pstate, struct sockaddr_in6 *paddr)
{
    void *buf;
    int len;
    conn_body_t *conn_body;

    LOGD(ME, "connect to new gateway");
    conn_body = msn_alloc(CONNECT, TFS_ID2_LEN + 1, &buf, &len);
    tfs_get_ID2((uint8_t *)conn_body->ClientId, NULL);
    conn_body->ClientId[TFS_ID2_LEN] = 0;

    sendto(pstate->sockfd, buf, len, MSG_DONTWAIT,
           (struct sockaddr *)paddr, sizeof(*paddr));

    free(buf);
}

static void handle_adv(gateway_state_t *pstate, void *pmsg, int len)
{
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
    if (pstate->gateway_mode) {
        const char *idsu = (const char *)(pub_msg+1);
        const char *payload = idsu + strlen(idsu) + 1;
        link_publish(idsu, payload);
        return;
    }

    const char *payload = (const char *)(pub_msg+1);
    LOGD(ME, "recv %s", payload);
    cJSON *json = cJSON_Parse(payload);
    cJSON *idsu = cJSON_GetObjectItem(json, "idsu");

    link_execute(idsu ? idsu->valuestring : NULL, 0, json);

    cJSON_Delete(json);
}

static client_t *new_client(gateway_state_t *pstate, const char *id2)
{
    client_t *client;
    dlist_for_each_entry(&pstate->clients, client, client_t, next) {
        if (strncmp(id2, client->id2, sizeof(client->id2)) == 0) {
            LOGD(ME, "existing client %s - %s", client->id2, client->idsu);
            return client;
        }
    }

    client = malloc(sizeof(*client));
    bzero(client, sizeof(*client));
    strncpy(client->id2, id2, sizeof(client->id2) - 1);
    snprintf(client->idsu, sizeof(client->idsu) - 1, "device.%s", client->id2);
    dlist_add_tail(&client->next, &pstate->clients);

    link_subscribe(client->idsu, client_msg_link_cb, NULL, client);

    LOGD(ME, "new client %s - %s", client->id2, client->idsu);

    return client;
}

static void handle_connect(gateway_state_t *pstate, void *pmsg, int len)
{
    if (!pstate->gateway_mode) {
        LOGE(ME, "error recv CONNECT cmd!");
        return;
    }

    conn_body_t *conn_body = pmsg;

    /* update ipaddr anyway */
    client_t *client = new_client(pstate, conn_body->ClientId);
    memcpy(&client->addr, &pstate->src_addr, sizeof(client->addr));

    void *buf;
    conn_ack_t *conn_ack = msn_alloc(CONNACK, 0, &buf, &len);
    conn_ack->ReturnCode = 0;
    sendto(pstate->sockfd, buf, len, MSG_DONTWAIT,
           (struct sockaddr *)&client->addr, sizeof(client->addr));
    free(buf);
}

static void handle_connack(gateway_state_t *pstate, void *pmsg, int len)
{
    if (pstate->gateway_mode) {
        LOGE(ME, "error recv CONNACK cmd!");
        return;
    }

    conn_ack_t *conn_ack = pmsg;
    if (conn_ack->ReturnCode != 0) {
        LOGE(ME, "connect fail %d!", conn_ack->ReturnCode);
        return;
    }

    pstate->mqtt_connected = true;

    LOGD(ME, "connack");
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
        LOGE(ME, "error read count %d", len);
        return;
    }

    len = msn_parse_header(buf, len, &pmsg);
    if (len < 0) {
        LOGE(ME, "error parsing header %x", buf[0]);
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

    free(buf);
}

int csp_get_args(const char ***pargv);
int gateway_service_init(void)
{
    int i, argc;
    const char **argv;
    gateway_state_t *pstate = &gateway_state;

    argc = csp_get_args(&argv);
    for (i=0;i<argc;i++) {
        if (strcmp(argv[i], "--cloud-gateway") == 0) {
            pstate->gateway_mode = true;
            break;
        }
    }

    pstate->sockfd = -1;
    dlist_init(&gateway_state.clients);
    return 0;
}

void gateway_service_deinit(void)
{
}

void gateway_service_clear(void)
{
}

static int init_socket(void)
{
    gateway_state_t *pstate = &gateway_state;
    struct sockaddr_in6 addr;
    int sockfd = pstate->sockfd;
    int ret;

    if (sockfd >= 0) {
        yos_cancel_poll_read_fd(sockfd, gateway_sock_read_cb, pstate);
        close(sockfd);
    }

    sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        LOGE(ME, "error open socket %d", errno);
        return -1;
    }

    int val = 1;
    setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &val, sizeof(val));

    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(MQTT_SN_PORT);
    ret = bind(sockfd, (const struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        LOGE(ME, "error bind socket %d", errno);
        close(sockfd);
        return -1;
    }

    yos_poll_read_fd(sockfd, gateway_sock_read_cb, pstate);
    pstate->sockfd = sockfd;

    return 0;
}

int gateway_service_start(void)
{
    return 0;
}

void gateway_service_stop(void) {
    close(gateway_state.sockfd);
}

void gateway_service_event(input_event_t *eventinfo) {
    if (eventinfo->type == EV_YUNIO) {
        if (eventinfo->code == CODE_YUNIO_ON_CONNECTED) {
            gateway_state.yunio_connected = true;
        }
    }

    if (eventinfo->type != EV_MESH)
        return;

    if (eventinfo->code != CODE_MESH_CONNECTED)
        return;

    init_socket();

    if (gateway_state.gateway_mode) {
        yos_cancel_delayed_action(-1, gateway_advertise, &gateway_state);
        yos_post_delayed_action(5 * 1000, gateway_advertise, &gateway_state);
    }
}


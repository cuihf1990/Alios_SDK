/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <aos/aos.h>

#include "umesh.h"
#include "umesh_hal.h"
#include "umesh_utils.h"
#include "core/mesh_mgmt.h"
#include "core/sid_allocator.h"
#include "core/link_mgmt.h"
#include "core/router_mgr.h"
#include "ipv6/ip6.h"
#include "hal/interfaces.h"
#include "tools/cli.h"
#include "tools/diags.h"
#include "lwip/sockets.h"

#include "lwip/inet_chksum.h"
#include "lwip/inet.h"

static void process_help(int argc, char *argv[]);
#ifdef CONFIG_AOS_MESH_DEBUG
static void process_autotest(int argc, char *argv[]);
static void process_channel(int argc, char *argv[]);
static void process_init(int argc, char *argv[]);
static void process_ipaddr(int argc, char *argv[]);
static void process_loglevel(int argc, char *argv[]);
static void process_macaddr(int argc, char *argv[]);
static void process_meshnetid(int argc, char *argv[]);
static void process_meshnetsize(int argc, char *argv[]);
static void process_mode(int argc, char *argv[]);
static void process_networks(int argc, char *argv[]);
static void process_prefix(int argc, char *argv[]);
static void process_router(int argc, char *argv[]);
static void process_seclevel(int argc, char *argv[]);
static void process_state(int argc, char *argv[]);
static void process_sids(int argc, char *argv[]);
static void process_testcmd(int argc, char *argv[]);
static void process_traceroute(int argc, char *argv[]);
static void process_whitelist(int argc, char *argv[]);
#endif

static void process_extnetid(int argc, char *argv[]);
static void process_nbrs(int argc, char *argv[]);
static void process_ping(int argc, char *argv[]);
static void process_start(int argc, char *argv[]);
static void process_stats(int argc, char *argv[]);
static void process_status(int argc, char *argv[]);
static void process_stop(int argc, char *argv[]);

void response_append(const char *format, ...);
static void show_router(uint8_t id);
static void show_ipaddr(network_context_t *network);

const cli_command_t g_commands[] = {
    { "help", &process_help },
#ifdef CONFIG_AOS_MESH_DEBUG
    { "autotest", &process_autotest },
    { "channel", &process_channel },
    { "init", &process_init },
    { "ipaddr", &process_ipaddr },
    { "loglevel", &process_loglevel },
    { "macaddr", &process_macaddr },
    { "meshnetid", &process_meshnetid },
    { "meshnetsize", &process_meshnetsize },
    { "mode", &process_mode },
    { "networks", &process_networks },
    { "prefix", &process_prefix },
    { "router", &process_router },
    { "seclevel", &process_seclevel },
    { "state", &process_state },
    { "sids", &process_sids },
    { "testcmd", &process_testcmd },
    { "traceroute", &process_traceroute },
    { "whitelist", &process_whitelist },
#endif
    { "extnetid", &process_extnetid },
    { "nbrs", &process_nbrs },
    { "ping", &process_ping },
    { "start", &process_start },
    { "stats", &process_stats },
    { "status", &process_status },
    { "stop", &process_stop },
};

enum {
    MAX_ARGS_NUM = 6,
    RESP_BUF_LEN = 256,
};

enum {
    DEF_ICMP6_PAYLOAD_SIZE = 8,
};

typedef struct cli_state_s {
    uint16_t      icmp_seq;
    uint16_t      icmp_acked;
    int           icmp_socket;
} cli_state_t;
static cli_state_t g_cl_state;

typedef void (*cmd_cb_t)(void *buf, int len, void *priv);
typedef struct input_cli_s {
    uint8_t  *data;
    uint16_t length;
    cmd_cb_t cb;
    void     *priv;
} input_cli_t;
static cmd_cb_t g_cur_cmd_cb;
static void *g_cur_cmd_priv;

static const char *state2str(mm_device_state_t state)
{
    switch (state) {
        case DEVICE_STATE_DISABLED:
            return "disabled";
        case DEVICE_STATE_DETACHED:
            return "detached";
        case DEVICE_STATE_ATTACHED:
            return "attached";
        case DEVICE_STATE_LEAF:
            return "leaf";
        case DEVICE_STATE_LEADER:
            return "leader";
        case DEVICE_STATE_SUPER_ROUTER:
            return "super_router";
        case DEVICE_STATE_ROUTER:
            return "router";
        default:
            return "unknown";
    }
}

static const char *attachstate2str(attach_state_t state)
{
    switch (state) {
        case ATTACH_IDLE:
            return "idle";
        case ATTACH_REQUEST:
            return "attaching";
        case ATTACH_SID_REQUEST:
            return "sid";
        case ATTACH_DONE:
            return "done";
    }
    return "unknown";
}

static const char *mediatype2str(media_type_t media)
{
    switch (media) {
        case MEDIA_TYPE_WIFI:
            return "wifi";
        case MEDIA_TYPE_BLE:
            return "ble";
        case MEDIA_TYPE_15_4:
            return "15.4";
    }
    return "unknown";
}

static const char *nbrstate2str(neighbor_state_t state)
{
    switch (state) {
        case STATE_CANDIDATE:
            return "candidate";
        case STATE_PARENT:
            return "parent";
        case STATE_CHILD:
            return "child";
        case STATE_NEIGHBOR:
            return "nbr";
        case STATE_INVALID:
            return "invalid";
    }
    return "unknown";
}

void process_help(int argc, char *argv[])
{
    uint16_t index;

    for (index = 0; index < sizeof(g_commands) / sizeof(g_commands[0]); ++index) {
        response_append("%s\r\n", g_commands[index].name);
    }
}

#ifdef CONFIG_AOS_MESH_DEBUG
static int hex2bin(const char *hex, uint8_t *bin, uint16_t bin_length);

typedef struct cli_autotest_s {
    int           udp_socket;
    ur_timer_t    timer;
    ur_timer_t    print_timer;
    uint16_t      times;
    uint16_t      seq;
    uint16_t      acked;
    uint16_t      length;
#if LWIP_IPV6
    ur_ip6_addr_t target;
#else
    mesh_ip4_addr_t target;
#endif
    slist_t       acked_list;
} cli_autotest_t;
cli_autotest_t g_cli_autotest;

enum {
    AUTOTEST_PRINT_WAIT_TIME = 15000,
    AUTOTEST_UDP_PORT        = 7335,
};

typedef struct autotest_acked_s {
    slist_t  next;
    uint16_t subnetid;
    uint16_t sid;
    uint16_t acked;
    uint16_t seq;
} autotest_acked_t;

#define for_each_acked(acked) \
    slist_for_each_entry(&g_cli_autotest.acked_list, acked, autotest_acked_t, next)

enum {
    AUTOTEST_REQUEST       = 1,
    AUTOTEST_REPLY         = 2,
    AUTOTEST_ECHO_INTERVAL = 1000,
};

typedef struct autotest_cmd_s {
    uint16_t seq;
    uint16_t type;
    ur_ip6_addr_t addr;
}  __attribute__((packed)) autotest_cmd_t;

static void handle_autotest_print_timer(void *args)
{
    autotest_acked_t *acked;
    uint8_t          num = 0;

    g_cli_autotest.print_timer = NULL;
    while (!slist_empty(&g_cli_autotest.acked_list)) {
        acked = slist_first_entry(&g_cli_autotest.acked_list, autotest_acked_t,
                                  next);
        response_append("%04x:%d\%, ",
                        ntohs(acked->sid), (acked->acked * 100) / g_cli_autotest.seq);
        slist_del(&acked->next, &g_cli_autotest.acked_list);
        ur_mem_free(acked, sizeof(autotest_acked_t));
        num++;
    }
    response_append("\r\nnum %d\r\n", num);
}

static void handle_autotest_timer(void *args)
{
#if LWIP_IPV6
    uint8_t                      *payload;
    const ur_netif_ip6_address_t *src;
    autotest_cmd_t               *cmd;

    g_cli_autotest.timer = NULL;
    payload = (uint8_t *)ur_mem_alloc(g_cli_autotest.length);
    if (payload) {
        src = umesh_get_ucast_addr();
        cmd = (autotest_cmd_t *)payload;
        cmd->type = AUTOTEST_REQUEST;
        cmd->seq = htons(g_cli_autotest.seq++);
        memcpy(&cmd->addr, &src->addr.ip6_addr, sizeof(ur_ip6_addr_t));
        ip6_sendto(g_cli_autotest.udp_socket, payload, g_cli_autotest.length,
                   &g_cli_autotest.target, AUTOTEST_UDP_PORT);
        ur_mem_free(payload, g_cli_autotest.length);
        g_cli_autotest.times--;
    }
    if (g_cli_autotest.times) {
        g_cli_autotest.timer = ur_start_timer(AUTOTEST_ECHO_INTERVAL,
                                              handle_autotest_timer, NULL);
    } else if (g_cli_autotest.print_timer == NULL) {
        g_cli_autotest.print_timer = ur_start_timer(AUTOTEST_PRINT_WAIT_TIME,
                                                    handle_autotest_print_timer, NULL);
    }
#else
    uint8_t *payload;
    const ur_netif_ip6_address_t *src;
    autotest_cmd_t *cmd;

    g_cli_autotest.timer = NULL;
    payload = (uint8_t *)ur_mem_alloc(g_cli_autotest.length);
    if (payload) {
        src = umesh_get_ucast_addr();
        cmd = (autotest_cmd_t *)payload;
        cmd->type = AUTOTEST_REQUEST;
        cmd->seq = htons(g_cli_autotest.seq++);
        memcpy(&cmd->addr, &src->addr.ip4_addr, sizeof(mesh_ip4_addr_t));
        ip6_sendto(g_cli_autotest.udp_socket, payload, g_cli_autotest.length,
                   &g_cli_autotest.target, AUTOTEST_UDP_PORT);
        ur_mem_free(payload, g_cli_autotest.length);
        g_cli_autotest.times--;
    }
    if (g_cli_autotest.times) {
        g_cli_autotest.timer = ur_start_timer(AUTOTEST_ECHO_INTERVAL,
                                              handle_autotest_timer, NULL);
    } else if (g_cli_autotest.print_timer == NULL) {
        g_cli_autotest.print_timer = ur_start_timer(AUTOTEST_PRINT_WAIT_TIME,
                                                    handle_autotest_print_timer, NULL);
    }
#endif
}

void process_autotest(int argc, char *argv[])
{
    char                         *end;
    autotest_acked_t             *acked = NULL;

    if (argc == 0) {
        for_each_acked(acked) {
            response_append("%04x: %d, %d\r\n", ntohs(acked->sid), acked->seq,
                            acked->acked);
        }
        return;
    }

    ur_stop_timer(&g_cli_autotest.timer, NULL);
    ur_stop_timer(&g_cli_autotest.print_timer, NULL);

    while (!slist_empty(&g_cli_autotest.acked_list)) {
        acked = slist_first_entry(&g_cli_autotest.acked_list, autotest_acked_t,
                                  next);
        slist_del(&acked->next, &g_cli_autotest.acked_list);
        ur_mem_free(acked, sizeof(autotest_acked_t));
    }

    g_cli_autotest.seq = 0;
    g_cli_autotest.acked = 0;
    g_cli_autotest.times = 1;
    if (argc > 1) {
        g_cli_autotest.times = (uint16_t)strtol(argv[1], &end, 0);
        if (g_cli_autotest.times == 0) {
            g_cli_autotest.times = 1;
        }
    }

    g_cli_autotest.length = sizeof(autotest_cmd_t);
    if (argc > 2) {
        g_cli_autotest.length = (uint16_t)strtol(argv[2], &end, 0);
        if (g_cli_autotest.length < (sizeof(autotest_cmd_t))) {
            g_cli_autotest.length = sizeof(autotest_cmd_t);
        }
    }
    if (g_cli_autotest.length > UR_IP6_MTU) {
        response_append("exceed mesh IP6 MTU %d\r\n", UR_IP6_MTU);
        return;
    }
#if LWIP_IPV6
    string_to_ip6_addr(argv[0], &g_cli_autotest.target);
#else
    g_cli_autotest.target.m32 = inet_addr(argv[0]);
#endif

    handle_autotest_timer(NULL);
}

void process_channel(int argc, char *argv[])
{
    channel_t channel;

    umesh_get_channel(&channel);
    response_append("%d\r\n", channel.channel);
    response_append("wifi %d\r\n", channel.wifi_channel);
    response_append("hal ucast %d\r\n", channel.hal_ucast_channel);
    response_append("hal bcast %d\r\n", channel.hal_bcast_channel);
    response_append("done\r\n");
}

void process_init(int argc, char *argv[])
{
    umesh_init(MODE_RX_ON);
    response_append("done\r\n");
}

void process_ipaddr(int argc, char *argv[])
{
    show_ipaddr(NULL);
    response_append("done\r\n");
}

void process_loglevel(int argc, char *argv[])
{
    if (argc > 0) {
        ur_log_set_level(str2lvl(argv[0]));
    }

    response_append("current: %s\r\n", lvl2str(ur_log_get_level()));
}

void process_networks(int argc, char *argv[])
{
    slist_t *networks;
    network_context_t *network;

    networks = umesh_get_networks();
    slist_for_each_entry(networks, network, network_context_t, next) {
        response_append("index %d\r\n", network->index);
        response_append("  hal %d\r\n", network->hal->module->type);
        response_append("  state %s\r\n", network->state == 0 ? "up" : "down");
        response_append("  meshnetid %x\r\n", network->meshnetid);
        response_append("  sid %x\r\n", network->sid);
        response_append("  sid type %d\r\n", network->router->sid_type);
        response_append("  route id %d\r\n", network->router->id);
        response_append("  netdata version %d\r\n", network->network_data.version);
        response_append("  size %d\r\n", network->network_data.size);
        response_append("  channel %d\r\n", network->channel);
    }
}

void process_macaddr(int argc, char *argv[])
{
    const mac_address_t *addr = umesh_get_mac_address();
    if (addr) {
        response_append("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
                        addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3],
                        addr->addr[4], addr->addr[5], addr->addr[6], addr->addr[7]);
    }
    response_append("done\r\n");
}

void process_meshnetid(int argc, char *argv[])
{
    response_append("0x%x\r\n", umesh_get_meshnetid());
}

void process_meshnetsize(int argc, char *argv[])
{
    response_append("%d\r\n", umesh_mm_get_meshnetsize());
}

void process_mode(int argc, char *argv[])
{
    uint8_t mode, index;
    ur_error_t error;

    mode = umesh_get_mode();

    for (index = 0; index < argc; index++) {
        if (strcmp(argv[index], "none") == 0) {
            mode = 0;
            break;
        }

        if (strcmp(argv[index], "LEADER") == 0) {
            mode |= MODE_LEADER;
            continue;
        }

#ifdef CONFIG_AOS_MESH_SUPER
        if (strcmp(argv[index], "SUPER") == 0) {
            mode |= MODE_SUPER;
            continue;
        }
#endif

        if (strcmp(argv[index], "ROUTER") == 0) {
            mode &= ~MODE_SUPER;
            continue;
        }

        if (strcmp(argv[index], "RX_ON") == 0) {
            mode |= MODE_RX_ON;
            continue;
        }

        if (strcmp(argv[index], "RX_OFF") == 0) {
            mode &= ~MODE_RX_ON;
            continue;
        }

        if (strcmp(argv[index], "MOBILE") == 0) {
            mode |= MODE_MOBILE;
            continue;
        }
        if (strcmp(argv[index], "FIXED") == 0) {
            mode &= ~MODE_MOBILE;
            continue;
        }

        /* or specify number */
        mode = atoi(argv[index]);
    }
    error = umesh_set_mode(mode);

    if (mode == 0 || error != UR_ERROR_NONE) {
        response_append("none\r\n");
        return;
    }

    if (mode & MODE_LEADER) {
        response_append("LEADER | ");
    }
    if (mode & MODE_SUPER) {
        response_append("SUPER");
    } else {
        response_append("NORMAL");
    }

    if (mode & MODE_RX_ON) {
        response_append(" | RX_ON");
    } else {
        response_append(" | RX_OFF");
    }

    if (mode & MODE_MOBILE) {
        response_append(" | MOBILE");
    } else {
        response_append(" | FIXED");
    }

    response_append("\r\n");
}

static bool update_autotest_acked_info(uint16_t subnetid, uint16_t sid,
                                       uint16_t seq)
{
    bool             exist = false;
    autotest_acked_t *acked = NULL;

    for_each_acked(acked) {
        if (acked->sid == sid && acked->subnetid == subnetid) {
            exist = true;
            break;
        }
    }

    if (exist == false) {
        acked = (autotest_acked_t *)ur_mem_alloc(sizeof(autotest_acked_t));
        if (acked == NULL) {
            return true;
        }
        acked->subnetid = subnetid;
        acked->sid = sid;
        acked->acked = 0;
        acked->seq = seq;
        slist_add_tail(&acked->next, &g_cli_autotest.acked_list);
    }

    if (seq > acked->seq || acked->acked == 0) {
        acked->acked++;
        acked->seq = seq;
        return true;
    }
    return false;
}

static void handle_udp_autotest(const uint8_t *payload, uint16_t length)
{
#if LWIP_IPV6
    autotest_cmd_t               *cmd;
    uint8_t                      *data;
    ur_ip6_addr_t                dest;
    const ur_netif_ip6_address_t *src;
    uint16_t                     seq;

    if (length == 0) {
        return;
    }

    cmd = (autotest_cmd_t *)payload;
    memcpy(&dest, &cmd->addr, sizeof(ur_ip6_addr_t));
    seq = ntohs(cmd->seq);
    if (cmd->type == AUTOTEST_REQUEST) {
        response_append("%d bytes autotest echo request from " IP6_ADDR_FMT
                        ", seq %d\r\n", length,
                        IP6_ADDR_DATA(dest), seq);
        data = (uint8_t *)ur_mem_alloc(length);
        if (data == NULL) {
            return;
        }
        memset(data, 0, length);
        cmd = (autotest_cmd_t *)data;
        cmd->type = AUTOTEST_REPLY;
        cmd->seq = htons(seq);
        src = umesh_get_ucast_addr();
        memcpy(&cmd->addr, &src->addr.ip6_addr, sizeof(ur_ip6_addr_t));
        ip6_sendto(g_cli_autotest.udp_socket, data, length, &dest,
                   AUTOTEST_UDP_PORT);
        ur_mem_free(data, length);
    } else if (cmd->type == AUTOTEST_REPLY) {
        g_cli_autotest.acked ++;
        if (update_autotest_acked_info(dest.m16[6], dest.m16[7], seq) == false) {
            return;
        }
        response_append("%d bytes autotest echo reply from " IP6_ADDR_FMT
                        ", seq %d\r\n", length,
                        IP6_ADDR_DATA(dest), seq);
    }
#else
    autotest_cmd_t *cmd;
    uint8_t *data;
    mesh_ip4_addr_t dest;
    const ur_netif_ip6_address_t *src;
    uint16_t seq;

    if (length == 0) {
        return;
    }

    cmd = (autotest_cmd_t *)payload;
    memcpy(&dest, &cmd->addr, sizeof(mesh_ip4_addr_t));
    seq = ntohs(cmd->seq);
    if (cmd->type == AUTOTEST_REQUEST) {
        response_append("%d bytes autotest echo request from " IP4_ADDR_FMT
                        ", seq %d\r\n", length,
                        IP4_ADDR_DATA(dest), seq);
        data = (uint8_t *)ur_mem_alloc(length);
        if (data == NULL) {
            return;
        }
        memset(data, 0, length);
        cmd = (autotest_cmd_t *)data;
        cmd->type = AUTOTEST_REPLY;
        cmd->seq = htons(seq);
        src = umesh_get_ucast_addr();
        memcpy(&cmd->addr, &src->addr.ip4_addr, sizeof(mesh_ip4_addr_t));
        ip6_sendto(g_cli_autotest.udp_socket, data, length, &dest,
                   AUTOTEST_UDP_PORT);
        ur_mem_free(data, length);
    } else if (cmd->type == AUTOTEST_REPLY) {
        g_cli_autotest.acked ++;
        if (update_autotest_acked_info(0, dest.m16[1], seq) == false) {
            return;
        }
        response_append("%d bytes autotest echo reply from " IP4_ADDR_FMT
                        ", seq %d\r\n", length,
                        IP4_ADDR_DATA(dest), seq);
    }
#endif
}

void process_prefix(int argc, char *argv[])
{
    response_append("fc00::/64\r\n");
}

void process_router(int argc, char *argv[])
{
    uint8_t id = 0, index;
    ur_error_t error = UR_ERROR_NONE;

    for (index = 0; index < argc; index++) {
        if (strcmp(argv[index], "SID_ROUTER") == 0) {
            id = SID_ROUTER;
            continue;
        }

        if (strcmp(argv[index], "VECTOR_ROUTER") == 0) {
            id = VECTOR_ROUTER;
            continue;
        }

        /* or specify number */
        id = atoi(argv[index]);
    }

    if (id == 0) {
        show_router(ur_router_get_default_router());
    } else {
        if (id != ur_router_get_default_router()) {
            error = ur_router_set_default_router(id);
        }

        response_append("switch to ");
        show_router(id);
        if (error == UR_ERROR_NONE) {
            response_append(" successfully");
        } else {
            response_append(" failed");
        }
    }
    response_append("\r\n");
}

void process_seclevel(int argc, char *argv[])
{
    uint8_t level;
    char    *end;

    if (argc == 0) {
        level = umesh_get_seclevel();
        response_append("%d\r\n", level);
    } else if (argc > 0) {
        level = (uint8_t)strtol(argv[0], &end, 0);
        umesh_set_seclevel(level);
    }
    response_append("done\r\n");
}

void process_state(int argc, char *argv[])
{
    mm_device_state_t state;

    state = umesh_get_device_state();

    response_append("%s\r\n", state2str(state));
}

void process_sids(int argc, char *argv[])
{
    sid_node_t    *node;
    slist_t       *nodes_list;
    slist_t       *networks;
    network_context_t *network;

    response_append("me=%04x\r\n", umesh_get_sid());
    networks = umesh_get_networks();
    slist_for_each_entry(networks, network, network_context_t, next) {
        nodes_list = get_ssid_nodes_list(network->sid_base);
        if (nodes_list == NULL) {
            continue;
        }
        slist_for_each_entry(nodes_list, node, sid_node_t, next) {
            response_append(EXT_ADDR_FMT ", %04x\r\n",
                            EXT_ADDR_DATA(node->node_id.ueid), node->node_id.sid);
        }
    }
}

/* speical cmd for test, output should be simple & well formated */
void process_testcmd(int argc, char *argv[])
{
    slist_t *hals;
    hal_context_t *hal;
    const char *cmd;

    if (argc < 1) {
        return;
    }

    cmd = argv[0];
    if (strcmp(cmd, "sid") == 0) {
        response_append("%04x", umesh_get_sid());
    } else if (strcmp(cmd, "state") == 0) {
        response_append("%s", state2str(umesh_get_device_state()));
    } else if (strcmp(cmd, "parent") == 0) {
        neighbor_t *nbr;
        hals = umesh_get_hals();
        slist_for_each_entry(hals, hal, hal_context_t, next) {
            slist_for_each_entry(&hal->neighbors_list, nbr, neighbor_t, next) {
                if (nbr->state != STATE_PARENT) {
                    continue;
                }
                response_append(EXT_ADDR_FMT, EXT_ADDR_DATA(nbr->mac));
                return;
            }
        }
    } else if (strcmp(cmd, "netsize") == 0) {
        response_append("%d", umesh_mm_get_meshnetsize());
    } else if (strcmp(cmd, "netid") == 0) {
        response_append("%04x", umesh_get_meshnetid());
    } else if (strcmp(cmd, "icmp_acked") == 0) {
        response_append("%d", g_cl_state.icmp_acked);
    } else if (strcmp(cmd, "autotest_acked") == 0) {
        response_append("%d", g_cli_autotest.acked);
    } else if (strcmp(cmd, "ipaddr") == 0) {
#if LWIP_IPV6
        response_append(IP6_ADDR_FMT, IP6_ADDR_DATA(umesh_get_ucast_addr()->addr.ip6_addr));
#else
        response_append(IP6_ADDR_FMT, IP6_ADDR_DATA(umesh_get_ucast_addr()->addr.ip4_addr));
#endif
    } else if (strcmp(cmd, "router") == 0) {
        show_router(ur_router_get_default_router());
    }
}

void process_traceroute(int argc, char *argv[])
{
#if LWIP_IPV6
    ur_ip6_addr_t dest;
    ur_addr_t dest_addr;
    network_context_t *network;

    if (argc == 0) {
        return;
    }

    string_to_ip6_addr(argv[0], &dest);
    if (umesh_resolve_dest(&dest, &dest_addr) != UR_ERROR_NONE) {
        return;
    }

    network = get_network_context_by_meshnetid(dest_addr.netid);
    if (network == NULL) {
        network = get_default_network_context();
    }
    send_trace_route_request(network, &dest_addr);
#endif
}

void process_whitelist(int argc, char *argv[])
{
    uint8_t       arg_index = 0;
    int           length;
    mac_address_t addr;
    int8_t        rssi;

    if (arg_index >= argc) {
        int i = 0;
        bool enabled = umesh_is_whitelist_enabled();
        const whitelist_entry_t *whitelist = umesh_get_whitelist_entries();
        response_append("whitelist is %s, entries:\r\n", enabled ? "enabled" : "disabled");
        for (i = 0; i < WHITELIST_ENTRY_NUM; i++) {
            if (whitelist[i].valid == false) {
                continue;
            }
            response_append("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
                            whitelist[i].address.addr[0], whitelist[i].address.addr[1],
                            whitelist[i].address.addr[2], whitelist[i].address.addr[3],
                            whitelist[i].address.addr[4], whitelist[i].address.addr[5],
                            whitelist[i].address.addr[6], whitelist[i].address.addr[7]);
        }
        return;
    }

    if (strcmp(argv[arg_index], "add") == 0) {
        if (++arg_index >= argc) {
            return;
        }
        addr.len = 8;
        length = hex2bin(argv[arg_index], addr.addr, sizeof(addr.addr));
        if (length != sizeof(addr.addr)) {
            return;
        }
        if (++arg_index < argc) {
            rssi = (int8_t)strtol(argv[arg_index], NULL, 0);
            umesh_add_whitelist_rssi(&addr, rssi);
        } else {
            umesh_add_whitelist(&addr);
        }
    } else if (strcmp(argv[arg_index], "clear") == 0) {
        umesh_clear_whitelist();
    } else if (strcmp(argv[arg_index], "disable") == 0) {
        umesh_disable_whitelist();
    } else if (strcmp(argv[arg_index], "enable") == 0) {
        umesh_enable_whitelist();
    } else if (strcmp(argv[arg_index], "remove") == 0) {
        if (++arg_index >= argc) {
            return;
        }
        addr.len = 8;
        length = hex2bin(argv[arg_index], addr.addr, sizeof(addr.addr));
        if (length != sizeof(addr.addr)) {
            return;
        }
        umesh_remove_whitelist(&addr);
    }
    response_append("done\r\n");
}

#endif

static int hex2bin(const char *hex, uint8_t *bin, uint16_t bin_length)
{
    uint16_t hex_length = strlen(hex);
    const char *hex_end = hex + hex_length;
    uint8_t *cur = bin;
    uint8_t num_chars = hex_length & 1;
    uint8_t byte = 0;

    if ((hex_length + 1) / 2 > bin_length) {
        return -1;
    }

    while (hex < hex_end) {
        if ('A' <= *hex && *hex <= 'F') {
            byte |= 10 + (*hex - 'A');
        } else if ('a' <= *hex && *hex <= 'f') {
            byte |= 10 + (*hex - 'a');
        } else if ('0' <= *hex && *hex <= '9') {
            byte |= *hex - '0';
        } else {
            return -1;
        }
        hex++;
        num_chars++;

        if (num_chars >= 2) {
            num_chars = 0;
            *cur++ = byte;
            byte = 0;
        } else {
            byte <<= 4;
        }
    }
    return cur - bin;
}

void response_append(const char *format, ...)
{
    va_list list;
    char res_buf[CMD_LINE_SIZE];
    uint16_t len;

    va_start(list, format);
    len = vsnprintf(res_buf, sizeof(res_buf) - 1, format, list);
    va_end(list);
    if (len >= sizeof(res_buf)) {
        len = sizeof(res_buf) - 1;
        res_buf[len] = 0;
    }

    if (g_cur_cmd_cb) {
        g_cur_cmd_cb(res_buf, len, g_cur_cmd_priv);
    } else {
#ifdef CONFIG_AOS_DDA
        extern int dda_cli_log(char *str);
        dda_cli_log((char *)res_buf);
#endif
        if (!g_cli_silent) {
            csp_printf("%s", res_buf);
        }
    }
}

void process_extnetid(int argc, char *argv[])
{
    umesh_extnetid_t extnetid;
    uint8_t length;

    if (argc > 0) {
        length = hex2bin(argv[0], extnetid.netid, 6);
        if (length != 6) {
            return;
        }
        extnetid.len = length;
        umesh_set_extnetid(&extnetid);
        aos_kv_set("extnetid", extnetid.netid, extnetid.len, 1);
    }

    memset(&extnetid, 0, sizeof(extnetid));
    umesh_get_extnetid(&extnetid);
    for (length = 0; length < extnetid.len; length++) {
        response_append("%02x:", extnetid.netid[length]);
    }
    if (extnetid.len > 0) {
        response_append("\r\n");
    }
    response_append("done\r\n");
}

static void show_ipaddr(network_context_t *network)
{
#if LWIP_IPV6
    const ur_netif_ip6_address_t *addr;
    addr = umesh_get_ucast_addr();
    while (addr) {
        response_append("\t" IP6_ADDR_FMT "\r\n",
                        IP6_ADDR_DATA(addr->addr.ip6_addr));
        addr = addr->next;
    }

    addr = umesh_get_mcast_addr();
    while (addr) {
        response_append("\t" IP6_ADDR_FMT "\r\n",
                        IP6_ADDR_DATA(addr->addr.ip6_addr));
        addr = addr->next;
    }
#else
    const ur_netif_ip6_address_t *addr;
    addr = umesh_get_ucast_addr();
    response_append("\t" IP4_ADDR_FMT "\r\n", IP4_ADDR_DATA(addr->addr.ip4_addr));
    addr = umesh_get_mcast_addr();
    response_append("\t" IP4_ADDR_FMT "\r\n", IP4_ADDR_DATA(addr->addr.ip4_addr));
#endif
}

void process_nbrs(int argc, char *argv[])
{
    neighbor_t *nbr;
    slist_t *hals;
    hal_context_t *hal;

    response_append("neighbors:\r\n");
    hals = umesh_get_hals();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        uint16_t   num = 0;
        response_append("\t<<hal type %s>>\r\n", mediatype2str(hal->module->type));
        slist_for_each_entry(&hal->neighbors_list, nbr, neighbor_t, next) {
            response_append("\t" EXT_ADDR_FMT, EXT_ADDR_DATA(nbr->mac));
            response_append(",%s,0x%04x,0x%04x,%d,%d,%d,%d,%d,%d\r\n", nbrstate2str(nbr->state), \
                            nbr->netid, nbr->sid, nbr->stats.link_cost, nbr->ssid_info.child_num, \
                            nbr->channel, nbr->stats.reverse_rssi, nbr->stats.forward_rssi, nbr->last_heard);
            num++;
        }
        response_append("\tnum=%d\r\n", num);
    }
}

void process_ping(int argc, char *argv[])
{
    ur_icmp6_header_t *header;
#if LWIP_IPV6
    ur_ip6_addr_t     target;
#else
    mesh_ip4_addr_t target;
#endif
    uint8_t           *payload;
    uint16_t          length;
    char              *end;

    if (argc == 0) {
        return;
    }

    length = DEF_ICMP6_PAYLOAD_SIZE + sizeof(ur_icmp6_header_t);
#if LWIP_IPV6
    string_to_ip6_addr(argv[0], &target);
#else
    target.m32 = inet_addr(argv[0]);
#endif

    if (argc > 1) {
        length = (uint16_t)strtol(argv[1], &end, 0);
        length += sizeof(ur_icmp6_header_t);
    }

    if (length > UR_IP6_MTU) {
        response_append("exceed mesh IP MTU %d\r\n", UR_IP6_MTU);
        return;
    }

    payload = (uint8_t *)ur_mem_alloc(length);
    if (payload == NULL) {
        return;
    }
    memset(payload, 0, length);

    header = (ur_icmp6_header_t *)(payload);
    header->type = UR_ICMP6_TYPE_EREQ;
    header->code = 7;
    header->data = g_cl_state.icmp_seq;
    ++g_cl_state.icmp_seq;
    header->chksum = inet_chksum(payload, length);

    ip6_sendto(g_cl_state.icmp_socket, payload, length, &target, 0);
    ur_mem_free(payload, length);
}

void cli_handle_echo_response(const uint8_t *payload, uint16_t length)
{
    ur_icmp6_header_t *icmp6_header;
#if LWIP_IPV6
    ur_ip6_header_t   *ip6_header;

    if (length) {
        ip6_header = (ur_ip6_header_t *)payload;
        icmp6_header = (ur_icmp6_header_t *)(payload + UR_IP6_HLEN);
        if (icmp6_header->type == UR_ICMP6_TYPE_EREP) {
            g_cl_state.icmp_acked ++;
            response_append("%d bytes from " IP6_ADDR_FMT " icmp_seq %d icmp_acked %d\r\n",
                            length - UR_IP6_HLEN - sizeof(ur_icmp6_header_t),
                            IP6_ADDR_DATA(ip6_header->src),
                            icmp6_header->data, g_cl_state.icmp_acked);
        }
    }
#else
    mesh_ip4_header_t *ip4_header;

    if (length) {
        ip4_header = (mesh_ip4_header_t *)payload;
        icmp6_header = (ur_icmp6_header_t *)(payload + MESH_IP4_HLEN);
        if (icmp6_header->type == UR_ICMP6_TYPE_EREP) {
            g_cl_state.icmp_acked ++;
            response_append("%d bytes from " IP4_ADDR_FMT " icmp_seq %d icmp_acked %d\r\n",
                            length - MESH_IP4_HLEN - sizeof(ur_icmp6_header_t),
                            IP4_ADDR_DATA(ip4_header->src),
                            icmp6_header->data, g_cl_state.icmp_acked);
        }
    }
#endif
}

static void show_router(uint8_t id)
{
    switch (id) {
        case SID_ROUTER:
            response_append("SID_ROUTER");
            break;
        case VECTOR_ROUTER:
            response_append("VECTOR_ROUTER");
            break;
        default:
            response_append("UNKNOWN_ROUTER");
            break;
    }
}

void process_start(int argc, char *argv[])
{
    umesh_start();
    response_append("done\r\n");
}

void process_stats(int argc, char *argv[])
{
    slist_t                  *hals;
    hal_context_t            *hal;
    const ur_message_stats_t *message_stats;
    const frame_stats_t      *hal_stats;
    const ur_mem_stats_t     *mem_stats;

    hals = umesh_get_hals();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
#ifdef CONFIG_AOS_MESH_DEBUG
        const ur_link_stats_t    *link_stats;
        link_stats = umesh_get_link_stats(hal->module->type);
        if (link_stats) {
            response_append("\t<<hal type %s>>\r\n", mediatype2str(hal->module->type));
            response_append("link stats\r\n");
            response_append("  in_frames %d\r\n", link_stats->in_frames);
            response_append("  in_command %d\r\n", link_stats->in_command);
            response_append("  in_data %d\r\n", link_stats->in_data);
            response_append("  in_filterings %d\r\n", link_stats->in_filterings);
            response_append("  in_drops %d\r\n", link_stats->in_drops);
            response_append("  out_frames %d\r\n", link_stats->out_frames);
            response_append("  out_command %d\r\n", link_stats->out_command);
            response_append("  out_data %d\r\n", link_stats->out_data);
            response_append("  out_errors %d\r\n", link_stats->out_errors);
            response_append("  send_queue_size %d\r\n", link_stats->send_queue_size);
            response_append("  recv_queue_size %d\r\n", link_stats->recv_queue_size);
            response_append("  sending %s\r\n", link_stats->sending ? "true" : "false");
            response_append("  sending_timeouts %d\r\n", link_stats->sending_timeouts);
        }
#endif

        hal_stats = umesh_get_hal_stats(hal->module->type);
        if (hal_stats) {
            response_append("\t<<hal type %s>>\r\n", mediatype2str(hal->module->type));
            response_append("hal stats\r\n");
            response_append("  in_frames %d\r\n", hal_stats->in_frames);
            response_append("  out_frames %d\r\n", hal_stats->out_frames);
        }
    }

    message_stats = umesh_get_message_stats();
    if (message_stats) {
        response_append("message stats\r\n");
        response_append("  nums %d\r\n", message_stats->num);
        response_append("  queue_fulls %d\r\n", message_stats->queue_fulls);
        response_append("  mem_fails %d\r\n", message_stats->mem_fails);
        response_append("  pbuf_fails %d\r\n", message_stats->pbuf_fails);
        response_append("  size %d\r\n", message_stats->size);

        uint8_t index;
        response_append("  msg debug info\r\n  ");
        for (index = 0; index < MSG_DEBUG_INFO_SIZE; index++) {
            response_append("%d:", message_stats->debug_info[index]);
        }
        response_append("\r\n");
    }

    mem_stats = umesh_get_mem_stats();
    if (mem_stats) {
        response_append("memory stats\r\n");
        response_append("  nums %d\r\n", mem_stats->num);
    }
}

static void process_status(int argc, char *argv[])
{
    slist_t *networks;
    network_context_t *network;
    channel_t channel;

    response_append("state\t%s\r\n", state2str(umesh_get_device_state()));
    networks = get_network_contexts();
    slist_for_each_entry(networks, network, network_context_t, next) {
        response_append("<<network %s %d>>\r\n",
                        mediatype2str(network->hal->module->type), network->index);
        response_append("\tnetid\t0x%x\r\n", umesh_mm_get_meshnetid(network));
        response_append("\tmac\t" EXT_ADDR_FMT "\r\n",
                        EXT_ADDR_DATA(network->hal->mac_addr.addr));
        response_append("\tattach\t%s\r\n", attachstate2str(network->attach_state));
        response_append("\tsid\t%04x\r\n", umesh_mm_get_local_sid());
        response_append("\tnetsize\t%d\r\n", umesh_mm_get_meshnetsize());
        response_append("\tipaddr");
        show_ipaddr(network);
        response_append("\trouter\t");
        show_router(network->router->id);
        response_append("\r\n");
        response_append("\tbcast_mtu %d\r\n",
                        hal_umesh_get_bcast_mtu(network->hal->module));
        response_append("\tucast_mtu %d\r\n",
                        hal_umesh_get_ucast_mtu(network->hal->module));
    }

    umesh_get_channel(&channel);
    response_append("\tchannel %d\r\n", channel.channel);
}

void process_stop(int argc, char *argv[])
{
    umesh_stop();
    response_append("done\r\n");
}

static void do_cli(void *arg)
{
    input_cli_t *buf = arg;
    char        *argv[MAX_ARGS_NUM];
    char        *cmd;
    int         argc;
    char        *last;
    uint16_t    index;

    cmd = strtok_r((char *)buf->data, " ", &last);
    for (argc = 0; argc < MAX_ARGS_NUM; ++argc) {
        if ((argv[argc] = strtok_r(NULL, " ", &last)) == NULL) {
            break;
        }
    }

    if (umesh_is_initialized() == false && strcmp(cmd, "init") != 0) {
        ur_mem_free(buf->data, buf->length);
        ur_mem_free(buf, sizeof(input_cli_t));
        return;
    }

    g_cur_cmd_cb = buf->cb;
    g_cur_cmd_priv = buf->priv;
    for (index = 0; index < sizeof(g_commands) / sizeof(g_commands[0]); index++) {
        if (strcmp(cmd, g_commands[index].name) == 0) {
            g_commands[index].function(argc, argv);
            break;
        }
    }
    if (g_cur_cmd_cb) {
        g_cur_cmd_cb(NULL, 0, buf->priv);
    }
    g_cur_cmd_cb = NULL;
    g_cur_cmd_priv = NULL;

    ur_mem_free(buf->data, buf->length);
    ur_mem_free(buf, sizeof(input_cli_t));
}

void ur_cli_cmd(char *buf, uint16_t length, cmd_cb_t cb, void *priv)
{
    input_cli_t *input_cli;

    input_cli = (input_cli_t *)ur_mem_alloc(sizeof(input_cli_t));
    if (input_cli == NULL) {
        return;
    }
    input_cli->data = (uint8_t *)ur_mem_alloc(length + 1);
    if (input_cli->data == NULL) {
        ur_mem_free(input_cli, sizeof(input_cli_t));
        return;
    }
    input_cli->length = length + 1;
    input_cli->cb = cb;
    input_cli->priv = priv;
    memcpy(input_cli->data, (uint8_t *)buf, length);
    input_cli->data[length] = '\0';
    umesh_task_schedule_call(do_cli, input_cli);
}

void ur_cli_input(char *buf, uint16_t length)
{
    ur_cli_cmd(buf, length, NULL, NULL);
}

void ur_cli_input_args(char **argv, uint16_t argc)
{
    uint8_t index;
    char **options = NULL;

    if (argc < 2) {
        return;
    }

    if (umesh_is_initialized() == false && strcmp(argv[1], "init") != 0) {
        return;
    }

    for (index = 0; index < sizeof(g_commands) / sizeof(g_commands[0]); index++) {
        if (strcmp(argv[1], g_commands[index].name) == 0) {
            if (argc > 2) {
                options = &argv[2];
            }
            argc -= 2;
            g_commands[index].function(argc, options);
            return;
        }
    }

    response_append("cmd no supported\r\n");
}

static void umesh_command(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                          char **argv)
{
    ur_cli_input_args(argv, argc);
}

static struct cli_command ncmd = {
    .name = "umesh",
    .help = "umesh [cmd]",
    .function = umesh_command,
};

#ifndef WITH_LWIP
struct cb_arg {
    raw_data_handler_t handler;
    uint8_t *buffer;
    int length;
};

static void do_read_cb(void *priv)
{
    struct cb_arg *arg = priv;
    arg->handler(arg->buffer, arg->length);

    ur_mem_free(arg->buffer, UR_IP6_MTU + UR_IP6_HLEN);
    ur_mem_free(arg, sizeof(*arg));
}

static void ur_read_sock(int fd, raw_data_handler_t handler)
{
    int     length = 0;
    uint8_t *buffer;

    buffer = ur_mem_alloc(UR_IP6_MTU + UR_IP6_HLEN);
    if (buffer == NULL) {
        return;
    }

    length = lwip_recv(fd, buffer, UR_IP6_MTU + UR_IP6_HLEN, 0);
    if (length > 0) {
        struct cb_arg *arg = ur_mem_alloc(sizeof(*arg));
        arg->handler = handler;
        arg->buffer = buffer;
        arg->length = length;
        aos_schedule_call(do_read_cb, arg);
        return;
    }

    ur_mem_free(buffer, UR_IP6_MTU + UR_IP6_HLEN);
}

static void mesh_worker(void *arg)
{
#ifdef CONFIG_AOS_MESH_DEBUG
    int maxfd = g_cli_autotest.udp_socket;
#else
    int maxfd = -1;
#endif

    if (g_cl_state.icmp_socket > maxfd) {
        maxfd = g_cl_state.icmp_socket;
    }

    while (1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(g_cl_state.icmp_socket, &rfds);
#ifdef CONFIG_AOS_MESH_DEBUG
        FD_SET(g_cli_autotest.udp_socket, &rfds);
#endif

        lwip_select(maxfd + 1, &rfds, NULL, NULL, NULL);

        if (FD_ISSET(g_cl_state.icmp_socket, &rfds)) {
            ur_read_sock(g_cl_state.icmp_socket, cli_handle_echo_response);
        }
#ifdef CONFIG_AOS_MESH_DEBUG
        if (FD_ISSET(g_cli_autotest.udp_socket, &rfds)) {
            ur_read_sock(g_cli_autotest.udp_socket, handle_udp_autotest);
        }
#endif
    }
}
#endif

int g_cli_silent;
ur_error_t mesh_cli_init(void)
{
    g_cl_state.icmp_socket = echo_socket(&cli_handle_echo_response);
#ifdef CONFIG_AOS_MESH_DEBUG
    slist_init(&g_cli_autotest.acked_list);
    g_cli_autotest.udp_socket = autotest_udp_socket(&handle_udp_autotest,
                                                    AUTOTEST_UDP_PORT);
#endif

#ifndef WITH_LWIP
    aos_task_new("meshworker", mesh_worker, NULL, 8192);
#endif
    cli_register_command(&ncmd);
    return UR_ERROR_NONE;
}

/* main.c - Application main entry point */

/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <aos/aos.h>
#include <aos/kernel.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh.h>

#include "mesh.h"
#include "net.h"
#include "transport.h"

#define CID_INTEL 0x0002

static struct bt_mesh_cfg_srv cfg_srv = {
	.relay = BT_MESH_RELAY_DISABLED,
	.beacon = BT_MESH_BEACON_ENABLED,
#if defined(CONFIG_BT_MESH_FRIEND)
	.frnd = BT_MESH_FRIEND_ENABLED,
#else
	.frnd = BT_MESH_FRIEND_NOT_SUPPORTED,
#endif
#if defined(CONFIG_BT_MESH_GATT_PROXY)
	.gatt_proxy = BT_MESH_GATT_PROXY_ENABLED,
#else
	.gatt_proxy = BT_MESH_GATT_PROXY_NOT_SUPPORTED,
#endif
	.default_ttl = 7,

	/* 3 transmissions with 20ms interval */
	.net_transmit = BT_MESH_TRANSMIT(2, 20),
	.relay_retransmit = BT_MESH_TRANSMIT(2, 20),
};

static struct bt_mesh_health_srv health_srv = {
};

static struct bt_mesh_model_pub health_pub = {
	.msg  = BT_MESH_HEALTH_FAULT_MSG(0),
};

static struct bt_mesh_model_pub gen_level_pub;
static struct bt_mesh_model_pub gen_onoff_pub;

static void gen_onoff_get(struct bt_mesh_model *model,
			  struct bt_mesh_msg_ctx *ctx,
			  struct net_buf_simple *buf)
{
}

static void gen_onoff_set(struct bt_mesh_model *model,
			  struct bt_mesh_msg_ctx *ctx,
			  struct net_buf_simple *buf)
{
}

static void gen_onoff_set_unack(struct bt_mesh_model *model,
				struct bt_mesh_msg_ctx *ctx,
				struct net_buf_simple *buf)
{
}

static const struct bt_mesh_model_op gen_onoff_op[] = {
	{ BT_MESH_MODEL_OP_2(0x82, 0x01), 0, gen_onoff_get },
	{ BT_MESH_MODEL_OP_2(0x82, 0x02), 2, gen_onoff_set },
	{ BT_MESH_MODEL_OP_2(0x82, 0x03), 2, gen_onoff_set_unack },
	BT_MESH_MODEL_OP_END,
};

static void gen_level_get(struct bt_mesh_model *model,
			  struct bt_mesh_msg_ctx *ctx,
			  struct net_buf_simple *buf)
{
}

static void gen_level_set(struct bt_mesh_model *model,
			  struct bt_mesh_msg_ctx *ctx,
			  struct net_buf_simple *buf)
{
}

static void gen_level_set_unack(struct bt_mesh_model *model,
				struct bt_mesh_msg_ctx *ctx,
				struct net_buf_simple *buf)
{
}

static void gen_delta_set(struct bt_mesh_model *model,
			  struct bt_mesh_msg_ctx *ctx,
			  struct net_buf_simple *buf)
{
}

static void gen_delta_set_unack(struct bt_mesh_model *model,
				struct bt_mesh_msg_ctx *ctx,
				struct net_buf_simple *buf)
{
}

static void gen_move_set(struct bt_mesh_model *model,
			 struct bt_mesh_msg_ctx *ctx,
			 struct net_buf_simple *buf)
{
}

static void gen_move_set_unack(struct bt_mesh_model *model,
			       struct bt_mesh_msg_ctx *ctx,
			       struct net_buf_simple *buf)
{
}

static const struct bt_mesh_model_op gen_level_op[] = {
	{ BT_MESH_MODEL_OP_2(0x82, 0x05), 0, gen_level_get },
	{ BT_MESH_MODEL_OP_2(0x82, 0x06), 3, gen_level_set },
	{ BT_MESH_MODEL_OP_2(0x82, 0x07), 3, gen_level_set_unack },
	{ BT_MESH_MODEL_OP_2(0x82, 0x09), 5, gen_delta_set },
	{ BT_MESH_MODEL_OP_2(0x82, 0x0a), 5, gen_delta_set_unack },
	{ BT_MESH_MODEL_OP_2(0x82, 0x0b), 3, gen_move_set },
	{ BT_MESH_MODEL_OP_2(0x82, 0x0c), 3, gen_move_set_unack },
	BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_model root_models[] = {
	BT_MESH_MODEL_CFG_SRV(&cfg_srv),
	BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
	BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, gen_onoff_op,
		      &gen_onoff_pub, NULL),
	BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_LEVEL_SRV, gen_level_op,
		      &gen_level_pub, NULL),
};

static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(0, root_models, BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
	.cid = CID_INTEL,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

static const u8_t default_key[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};

static struct {
        u16_t local;
        u16_t dst;
        u16_t net_idx;
        u16_t app_idx;
} net = {
        .local = BT_MESH_ADDR_UNASSIGNED,
        .dst = BT_MESH_ADDR_UNASSIGNED,
        .app_idx = BT_MESH_KEY_DEV,
};

static u8_t hex2val(char c)
{
        if (c >= '0' && c <= '9') {
                return c - '0';
        } else if (c >= 'a' && c <= 'f') {
                return c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
                return c - 'A' + 10;
        } else {
                return 0;
        }
}

static size_t hex2bin(const char *hex, u8_t *bin, size_t bin_len)
{
        size_t len = 0;

        while (*hex && len < bin_len) {
                bin[len] = hex2val(*hex++) << 4;

                if (!*hex) {
                        len++;
                        break;
                }

                bin[len++] |= hex2val(*hex++);
        }

        return len;
}

static int output_number(bt_mesh_output_action_t action, uint32_t number)
{
	printf("OOB Number: %u\n", number);
	return 0;
}

static void cmd_net_send(void *timer, void *args)
{
        struct net_buf_simple *msg = NET_BUF_SIMPLE(32);
        struct bt_mesh_msg_ctx ctx = {
                .send_ttl = BT_MESH_TTL_DEFAULT,
                .net_idx = net.net_idx,
                .addr = net.dst,
                .app_idx = net.app_idx,

        };
        struct bt_mesh_net_tx tx = {
                .ctx = &ctx,
                .src = net.local,
                .xmit = bt_mesh_net_transmit_get(),
                .sub = bt_mesh_subnet_get(net.net_idx),
        };
        size_t len;
        int err;

        if (!tx.sub) {
                printf("No matching subnet for NetKey Index 0x%04x\n",
                       net.net_idx);
                return;
        }

        net_buf_simple_init(msg, 0);
        len = hex2bin("2", msg->data, net_buf_simple_tailroom(msg) - 4);
        net_buf_simple_add(msg, len);

        err = bt_mesh_trans_send(&tx, msg, NULL, NULL);
        if (err) {
                printf("Failed to send (err %d)\n", err);
        }
}

static void prov_complete(u16_t net_idx, u16_t addr)
{
        printf("Local node provisioned, net_idx 0x%04x address 0x%04x\n",
               net_idx, addr);
        net.net_idx = net_idx,
        net.local = addr;
        net.dst = addr + 1;
        cmd_net_send(NULL, NULL);
}

static void prov_reset(void)
{
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
}

static const uint8_t dev_uuid[16] = { 0xda, 0xdd };

static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
	.output_size = 4,
	.output_actions = BT_MESH_DISPLAY_NUMBER,
	.output_number = output_number,
	.complete = prov_complete,
	.reset = prov_reset,
};

static void bt_ready(int err)
{
        u16_t net_idx = 1;
        u16_t addr = CID_INTEL;
        u32_t iv_index = 1;

	if (err) {
		printf("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printf("Bluetooth initialized\n");

	err = bt_mesh_init(&prov, &comp);
	if (err) {
		printf("Initializing mesh failed (err %d)\n", err);
		return;
	}

	//bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

        bt_mesh_provision(default_key, net_idx, 0, iv_index, 0, addr,
                          default_key);

	printf("Mesh initialized\n");
}

extern int hci_driver_init();
void blemesh_sample(void)
{
	int err;

	printf("Initializing...\n");

	/* Initialize the Bluetooth Subsystem */
        hci_driver_init();
	err = bt_enable(bt_ready);
	if (err) {
		printf("Bluetooth init failed (err %d)\n", err);
	}

}

static void app_delayed_action(void *arg)
{
    blemesh_sample();
}

int application_start(int argc, char **argv)
{
    aos_post_delayed_action(1000, app_delayed_action, NULL);
    aos_loop_run();
    return 0;
}

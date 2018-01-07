/*
 * copyright (c) 2015-2016 intel corporation
 *
 * licensed under the apache license, version 2.0 (the "license");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/license-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "as is" basis,
 * without warranties or conditions of any kind, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <aos/aos.h>
#include <aos/kernel.h>

#include <misc/byteorder.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#define DEVICE_NAME	"AOS-BLE-PERIPHERAL"
#define DEVICE_NAME_LEN	(sizeof(DEVICE_NAME) - 1)
#define APPEARANCE	0x0000

extern int hci_driver_init();

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (err) {
		printf("Failed to connect to %s (%u)\n", addr, err);
		return;
	}

	printf("Connected %s\n", addr);

	if (bt_conn_security(conn, BT_SECURITY_FIPS)) {
		printf("Failed to set security\n");
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printf("Disconnected from %s (reason %u)\n", addr, reason);
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printf("Passkey for %s: %u\n", addr, passkey);
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printf("Pairing cancelled: %s\n", addr);
}

static struct bt_conn_auth_cb auth_cb_display = {
	.passkey_display = auth_passkey_display,
	.passkey_entry = NULL,
	.cancel = auth_cancel,
};

static const char *gap_name;
static uint16_t gap_appearance;

static ssize_t read_name(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                         void *buf, uint16_t len, uint16_t offset)
{
        return bt_gatt_attr_read(conn, attr, buf, len, offset, gap_name,
                                 strlen(gap_name));
}

static ssize_t read_appearance(struct bt_conn *conn,
                               const struct bt_gatt_attr *attr, void *buf,
                               uint16_t len, uint16_t offset)
{
        uint16_t appearance = sys_cpu_to_le16(gap_appearance);

        return bt_gatt_attr_read(conn, attr, buf, len, offset, &appearance,
                                 sizeof(appearance));
}

static struct bt_gatt_attr attrs[] = {
        BT_GATT_PRIMARY_SERVICE(BT_UUID_GAP),
        BT_GATT_CHARACTERISTIC(BT_UUID_GAP_DEVICE_NAME, BT_GATT_CHRC_READ),
        BT_GATT_DESCRIPTOR(BT_UUID_GAP_DEVICE_NAME, BT_GATT_PERM_READ,
                           read_name, NULL, NULL),
        BT_GATT_CHARACTERISTIC(BT_UUID_GAP_APPEARANCE, BT_GATT_CHRC_READ),
        BT_GATT_DESCRIPTOR(BT_UUID_GAP_APPEARANCE, BT_GATT_PERM_READ,
                           read_appearance, NULL, NULL),
};

static void gap_init(const char *name, uint16_t appearance)
{
        gap_name = name;
        gap_appearance = appearance;

        bt_gatt_register(attrs, ARRAY_SIZE(attrs));
}

extern int hci_driver_init();
void ble_sample(void)
{
	int err;

        hci_driver_init();
	err = bt_enable(NULL);
	if (err) {
		printf("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printf("Bluetooth initialized\n");

	gap_init(DEVICE_NAME, APPEARANCE);

	bt_conn_auth_cb_register(&auth_cb_display);
	bt_conn_cb_register(&conn_callbacks);

	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (err) {
		printf("Advertising failed to start (err %d)\n", err);
		return;
	}

	printf("Advertising successfully started\n");
}

static void app_delayed_action(void *arg)
{
    ble_sample();
}

int application_start(int argc, char **argv)
{
    aos_post_delayed_action(1000, app_delayed_action, NULL);
    aos_loop_run();
    return 0;
}

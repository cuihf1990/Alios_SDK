#include <aos/aos.h>
#include <ble_app_framework_def.h>
#include <ble_app_framework.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>
#include <bluetooth/hci.h>
#include <bluetooth/gatt.h>

#ifndef __packed
#define __packed __attribute__((__packed__))
#endif

struct attr_info {
    /* hold the user-handle related to attr */
    uint16_t handle;
    /* user_data length info related to bt_gatt_attr.user_data*/
    uint16_t user_data_len; /* used by value attr */
};

#define BLE_PERI_HDL_MAX 1 /* support only 1 peripheral */
#define DEV_NAME_MAX_LEN 32
#define MANUFACTURE_NAME_MAX_LEN 32
typedef struct ble_peri_s {
    /* gatt db from user app */
    uint8_t *db;
    /* gatt db length */
    int db_len;
    /* attr array built from user db */
    struct bt_gatt_attr *attr;
    /* total number of attrs */
    int attr_num;
    /* hold the attr info, 1 entry for each attr */
    struct attr_info *itbl;
    /**
     * value attributes added by ble_attr_add(),
     * 1 entry for each characteristic value attr.
     */
    ble_gatt_attr_t *vattr;
    /* service structure built from attr */
    struct bt_gatt_service svc;
    char dev_name[DEV_NAME_MAX_LEN];
    char manu_name[MANUFACTURE_NAME_MAX_LEN];
} ble_peri_t;

static uint32_t g_ble_hdl = 0;
static ble_peri_t g_peri[BLE_PERI_HDL_MAX];

#define MOD "ble_framework"

int hci_driver_init();

peripheral_hdl_t
ble_peripheral_init
(
    peripheral_init_t *p,
    ble_peripheral_conn_cb_t c,
    ble_peripheral_disconn_cb_t disc,
    const uint8_t *gatt_db,
    int db_len
)
{
    int err;
    peripheral_hdl_t hdl;

    if (!p || !gatt_db) {
        LOGE(MOD, "%s: invalid arguments", __func__);
        return -1;
    }

    if (g_ble_hdl >= BLE_PERI_HDL_MAX) {
        LOGW(MOD, "No more peripheral is allowed.");
        return -1;
    }

    LOG("Initializing BLE Peripheral");

    hci_driver_init();
    err = bt_enable(NULL);
    if (err) {
        LOGE(MOD, "BLE Peripheral initialization failed (err %d)", err);
        return -1;
    }

    hdl = g_ble_hdl++;
    g_peri[hdl].db = (uint8_t *)gatt_db;
    g_peri[hdl].db_len = db_len;
    strncpy(g_peri[hdl].dev_name, p->dev_name, sizeof(g_peri[hdl].dev_name) - 1);

    LOG("BLE Peripheral initialization completed");

    return hdl;
}

static void free_val_attrs_list(peripheral_hdl_t hdl)
{

}

static void free_handle_to_attr_tbl(peripheral_hdl_t hdl)
{

}

void ble_peripheral_deinit
(
    peripheral_hdl_t hdl
)
{
    int i;

    /* Clear peripheral device structure here <TODO> */
    if (g_peri[hdl].attr)
        aos_free(g_peri[hdl].attr);

    for (i = 0; i < g_peri[hdl].attr_num; i++) {
        if (g_peri[hdl].attr[i].user_data)
            aos_free(g_peri[hdl].attr[i].user_data);
    }

    free_val_attrs_list(hdl);
    free_handle_to_attr_tbl(hdl);

    /* not supported in this version yet. */
    //bt_gatt_service_unregister(&g_peri[hdl].svc);

    return;
}

//#pragma pack(1)
struct db_genreral_hdr {
    uint8_t handle[2];
    uint8_t perm[1];
    uint8_t len[1];
    uint8_t type[2];
} __packed;

struct db_char_hdr {
    uint8_t val_handle[2];
    uint8_t perm[1];
    uint8_t len[1];
} __packed;
//#pragma pack()

#define G_HEADER_SIZE sizeof(struct db_genreral_hdr)
#define C_HEADER_SIZE sizeof(struct db_char_hdr)
#define TYPE_CMP(typer, typee) (((typer)[0] == ((typee) & 0xff)) && \
                                ((typer)[1] == (((typee) >> 8) & 0xff)))

static int make_attr_and_svc(peripheral_hdl_t hdl)
{
    uint8_t *db = g_peri[hdl].db, *p;
    int db_len = g_peri[hdl].db_len, len = 0, attr_cnt = 0, attr_idx = 0;
    struct db_genreral_hdr *iter = (struct db_genreral_hdr *)db;
    struct db_char_hdr *c;
    struct bt_gatt_attr *a, *a2;

    if (hdl >= BLE_PERI_HDL_MAX) {
        LOGE(MOD, "hdl number is not within valid range.");
        return -1;
    }

    if (!db || !db_len) {
        LOGE(MOD, "Invalid GATT database.");
        return -1;
    }

    LOGD(MOD, "db addr: %p, iter addr: %p", db, iter);

    /* get attr count */
    while ((uint32_t)iter < (uint32_t)(db + db_len)) {
        if (TYPE_CMP(iter->type, GATT_UUID_PRI_SERVICE) || /* primary service */
            TYPE_CMP(iter->type, GATT_UUID_SEC_SERVICE)) { /* secondary service */
            LOGD(MOD, "A primary or secondary service item found.");
            attr_cnt++;
            p = (uint8_t *)iter;
            p += G_HEADER_SIZE + iter->len[0] - 2;
            iter = (struct db_genreral_hdr *)p;
        } else if (TYPE_CMP(iter->type, GATT_UUID_INCLUDE_SERVICE)) { /* included service */
            LOGW(MOD, "Included service is not supported yet!!");
            return -1;
        } else if (TYPE_CMP(iter->type, GATT_UUID_CHAR_DECLARE)) { /* characteristic declare */
            LOGD(MOD, "A characteristic item found.");

            attr_cnt += 2; /* 1 for c, one for cdesr, do we need ccc? <TODO> */

            p = (uint8_t *)iter;
            p += G_HEADER_SIZE + iter->len[0] - 2; /* eat until uuid */
            c = (struct db_char_hdr *)p;
            p += C_HEADER_SIZE + c->len[0];
            /* writable charateristic? */
            if (c->perm[0] & (uint8_t)LEGATTDB_PERM_WRITABLE)
                p++;

            c = (struct db_char_hdr *)p;

            /* Is there a desciptor? skip it for now, need an attr for it. <TODO> */
            if (c->len[0] == (uint8_t)LEGATTDB_UUID16_SIZE) {
                /* attr_cnt++; */
                p += C_HEADER_SIZE + c->len[0];
                if (c->perm[0] & (uint8_t)LEGATTDB_PERM_WRITABLE)
                    p++;
            }

            /* More descriptor followed? <TODO> */

            iter = (struct db_genreral_hdr *)p;
        } else {
            LOGE(MOD, "Invalid attribute type in gatt database "
                 "(type: 0x%04x)!!!", iter->type);
            return -1;
        }
        LOGD(MOD, "After a new round, iter addr is %p", iter);
    }

    LOGI(MOD, "%d attributes added.", attr_cnt);

    /* Allocate memory for attribute array*/
    g_peri[hdl].attr_num = attr_cnt;
    g_peri[hdl].attr = (struct bt_gatt_attr *)aos_malloc(
        sizeof(struct bt_gatt_attr) * attr_cnt);
    g_peri[hdl].itbl = (struct attr_info *)aos_malloc(
        sizeof(struct attr_info) * attr_cnt);
    if (g_peri[hdl].attr == NULL || g_peri[hdl].itbl == NULL) {
        LOGE(MOD, "Failed to allocate memory for peripheral (%d)", hdl);
        return -1;
    }

    /* Make attr array */
    len = 0;
    iter = (struct db_genreral_hdr *)db;
    attr_idx = 0;
    while ((uint32_t)iter < (uint32_t)(db + db_len)) {
        if (TYPE_CMP(iter->type, GATT_UUID_PRI_SERVICE) ||
            TYPE_CMP(iter->type, GATT_UUID_SEC_SERVICE)) { /* service */
            a = &(g_peri[hdl].attr)[attr_idx];

            if (TYPE_CMP(iter->type, GATT_UUID_PRI_SERVICE)) {
                LOGD(MOD, "Adding a primary service attribute.");
                a->uuid = BT_UUID_GATT_PRIMARY;
            } else {
                LOGD(MOD, "Adding a secondary service attribute.");
                a->uuid = BT_UUID_GATT_SECONDARY;
            }

            /* Fill the attribute array */
            a->perm = BT_GATT_PERM_READ;
            a->read = bt_gatt_attr_read_service;

            /* service handle into attr info table */
            //memcpy(g_peri[hdl].itbl[attr_idx].handle, iter->handle, 2);
            g_peri[hdl].itbl[attr_idx].handle = ((uint16_t)iter->handle[0] |\
                 ((uint16_t)iter->handle[1] << 8));

            LOGD(MOD, "New handle (0x%04x, idx: %d) added in attr info table.",
                 g_peri[hdl].itbl[attr_idx].handle, attr_idx);

            /* determine the user data */
            if (iter->len[0] == 4) { /* uuid16 */
                struct bt_uuid_16 *tmp;
                tmp = (struct bt_uuid_16 *)aos_malloc(sizeof(struct bt_uuid_16));
                tmp->uuid.type = BT_UUID_TYPE_16;
                p = (uint8_t *)iter;
                p += G_HEADER_SIZE;
                memcpy(&tmp->val, p, 2);
                a->user_data = tmp;
            } else if (iter->len[0] == 18) { /* uui128 */
                struct bt_uuid_128 *tmp;
                tmp = (struct bt_uuid_128 *)aos_malloc(sizeof(struct bt_uuid_128));
                tmp->uuid.type = BT_UUID_TYPE_128;
                p = (uint8_t *)iter;
                p += G_HEADER_SIZE;
                memcpy(&tmp->val, p, 16);
                a->user_data = tmp;
            } else {
                LOGE(MOD, "Not supported service uuid for peripheral "
                     "(%d, attr_idx: %d).", hdl, attr_idx);
                return -1;
            }

            /* update pointer*/
            attr_idx++;
            p = (uint8_t *)iter;
            p += G_HEADER_SIZE + iter->len[0] - 2;
            iter = (struct db_genreral_hdr *)p;
        } else if (TYPE_CMP(iter->type, GATT_UUID_INCLUDE_SERVICE)) {
            LOGW(MOD, "Included service is not supported yet!!");
            return -1;
        } else if (TYPE_CMP(iter->type, GATT_UUID_CHAR_DECLARE)) {
            LOGD(MOD, "Adding a characteristic and a value attribute.");

            /* Fill the charateristic attribute */
            a = &(g_peri[hdl].attr)[attr_idx];
            a->uuid = BT_UUID_GATT_CHRC;
            a->perm = BT_GATT_PERM_READ;
            a->read = bt_gatt_attr_read_chrc;

            /* characteristic handle into attr info table */
            //memcpy(g_peri[hdl].itbl[attr_idx].handle, iter->handle, 2);
            g_peri[hdl].itbl[attr_idx].handle = ((uint16_t)iter->handle[0] |\
                 ((uint16_t)iter->handle[1] << 8));

            LOGD(MOD, "New handle (0x%04x, idx: %d) added in attr info table.",
                 g_peri[hdl].itbl[attr_idx].handle, attr_idx);

            /* Fill the charateristic value attribute */
            a2 = &(g_peri[hdl].attr)[attr_idx+1];
            a2->read = NULL;
            a2->write = NULL;
            a2->user_data = NULL;
            p = (uint8_t *)iter;
            p += G_HEADER_SIZE + iter->len[0] -2;
            c = (struct db_char_hdr *)p;
            a2->perm = c->perm[0];

            /* characteristic value handle into attr info table */
            //memcpy(g_peri[hdl].itbl[attr_idx+1].handle, c->val_handle, 2);
            g_peri[hdl].itbl[attr_idx+1].handle = ((uint16_t)c->val_handle[0] |\
                 ((uint16_t)c->val_handle[1] << 8));

            LOGD(MOD, "New handle (0x%04x, idx: %d) added in attr info table.",
                 g_peri[hdl].itbl[attr_idx+1].handle, attr_idx+1);

            /* user data */
            if (iter->len[0] == 7) { /* uui16 */
                struct bt_gatt_chrc *tmp = (struct bt_gatt_chrc *)aos_malloc(
                    sizeof(struct bt_gatt_chrc) + sizeof(struct bt_uuid_16));
                struct bt_uuid_16 *tmp_uuid = (struct bt_uuid_16 *)( \
                    (uint8_t *)tmp + sizeof(struct bt_gatt_chrc));
                if (!tmp) {
                    LOGE(MOD, "Failed to allocate memory %f %d",
                         __FILE__, __LINE__);
                    return -1;
                }
                tmp_uuid->uuid.type = BT_UUID_TYPE_16;
                p = (uint8_t *)iter;
                p += G_HEADER_SIZE;
                memcpy(&tmp_uuid->val, p, 2);
                tmp->uuid = (struct bt_uuid *)tmp_uuid;
                tmp->properties = *p;
                a->user_data = tmp;
                a2->uuid = (struct bt_uuid *)tmp_uuid;
            } else if (iter->len[0] == 21) { /* uuid128 */
                struct bt_gatt_chrc *tmp = (struct bt_gatt_chrc *)aos_malloc(
                    sizeof(struct bt_gatt_chrc) + sizeof(struct bt_uuid_128));
                struct bt_uuid_128 *tmp_uuid = (struct bt_uuid_128 *)( \
                    (uint8_t *)tmp + sizeof(struct bt_gatt_chrc));
                if (!tmp) {
                    LOGE(MOD, "Failed to allocate memory %f %d",
                         __FILE__, __LINE__);
                    return -1;
                }
                tmp_uuid->uuid.type = BT_UUID_TYPE_128;
                p = (uint8_t *)iter;
                p += G_HEADER_SIZE;
                memcpy(&tmp_uuid->val, p, 16);
                tmp->uuid = (struct bt_uuid *)tmp_uuid;
                tmp->properties = *p;
                a->user_data = tmp;
                a2->uuid = (struct bt_uuid *)tmp_uuid;
            } else {
                LOGE(MOD, "Not supported charateristic uuid for peripheral "
                     "(%d, attr_idx: %d).", hdl, attr_idx);
                return -1;
            }

            attr_idx += 2;

            /* update pointer */
            p = (uint8_t *)iter;
            p += G_HEADER_SIZE + iter->len[0] - 2 + C_HEADER_SIZE + c->len[0];
            if (c->perm[0] & (uint8_t)LEGATTDB_PERM_WRITABLE)
                p++;

            /* Is there a descriptor? If yes, skip it for now. <TODO> */
            c = (struct db_char_hdr *)p;
            if (c->len[0] == (uint8_t)LEGATTDB_UUID16_SIZE)
                p += C_HEADER_SIZE + c->len[0];

            /* More descriptor followed? <TODO> */

            /* update the iteration pointer */
            iter = (struct db_genreral_hdr *)p;
        } else {
            LOGE(MOD, "Invalid attribute type in gatt database!!!");
            return -1;
        }
    }

    if (attr_idx != g_peri[hdl].attr_num) {
        LOGE(MOD, "Error accured (attr_idx (%d) not maitch attr_num (%d))",
             attr_idx, g_peri[hdl].attr_num);
        return -1;
    }

    return 0;
}

void ble_adv_start
(
    ble_adv_complete_cb_t adv_handler,
    const char *manufacture,
    peripheral_hdl_t hdl
)
{
    int err;

    /* Add adv data dynamically <TODO> */
    struct bt_data ad[] = {
        BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
        BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x0a, 0x18),
    };

    struct bt_data sd[] = {
        BT_DATA(BT_DATA_NAME_COMPLETE, g_peri[hdl].dev_name,
                strlen(g_peri[hdl].dev_name)),
    };

    strncpy(g_peri[hdl].manu_name, manufacture,
            sizeof(g_peri[hdl].manu_name) - 1);

    if (make_attr_and_svc(hdl) != 0) return;
    //bt_gatt_service_register(&(g_peri[hdl].svc));
    bt_gatt_register(g_peri[hdl].attr, g_peri[hdl].attr_num);

    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad),
                          sd, ARRAY_SIZE(sd));
    if (err) {
        LOGE(MOD, "Advertising failed to start (err %d).", err);
        return;
    }

    LOG("Advertising successfully started.");
}

void ble_adv_stop()
{
    bt_le_adv_stop();
}

ble_gatt_attr_t *
ble_attr_add
(
    uint16_t handle, /* Note: this handle is not peripheral_hdl_t */
    uint16_t val_len,
    const uint8_t *val
)
{
    peripheral_hdl_t hdl;
    ble_gatt_attr_t *vattr, *p;

    vattr = (ble_gatt_attr_t *)aos_malloc(sizeof(ble_gatt_attr_t));
    if (!vattr) {
        LOGE(MOD, "Failed to alloca memory %s %d", __FILE__, __LINE__);
        return NULL;
    }

    vattr->handle = handle;
    vattr->this_node.next = NULL;
    vattr->this_node.prev = NULL;
    vattr->value_buffer_length = val_len;
    /* Do not care other field at this moment. May add later. */

    /**
     * Assume the case that only 1 peripheral supported,
     * need fix if multiple peripherals. <TODO>
     */
    hdl = g_ble_hdl - 1;

    /* vattr node into the end of vattr list */
    p = g_peri[hdl].vattr;
    if (g_peri[hdl].vattr == NULL) {
        g_peri[hdl].vattr = vattr;
    } else {
        /* quickly goes to end */
        dlist_node_t *n = &p->this_node;
        while (n->next != NULL) n = n->next;
        p = CONTAINER_OF(n, ble_gatt_attr_t, this_node);
        vattr->this_node.prev = &p->this_node;
        p->this_node.next = &vattr->this_node;
    }

    /* Find the associated attr in DB */
    int i;
    struct attr_info *t;
    LOGD(MOD, "Attr handle to find: 0x%04x, in %s", handle, __func__);
    for (i = 0; i < g_peri[hdl].attr_num; i++) {
        t = &g_peri[hdl].itbl[i];
        LOGD(MOD, "Checking t->handle: 0x%04x", t->handle);
        if (t->handle == handle) break;
    }

    if (i >= g_peri[hdl].attr_num) {
        LOGE(MOD, "No corresponding handle (0x%04x) found in DB.", handle);
        return NULL;
    }

    LOGD(MOD, "Attr found on idx: %d", i);

    /* associated attr found, fill the user_data field. */
    struct bt_gatt_attr *a = &(g_peri[hdl].attr[i]);
    a->user_data = (void *)val;
    g_peri[hdl].itbl[i].user_data_len = val_len;

    return vattr;
}

static struct bt_gatt_attr* find_attr_entry
(
    ble_gatt_attr_t *attr,
    peripheral_hdl_t hdl
)
{
    ble_gatt_attr_t *v = g_peri[hdl].vattr;
    dlist_node_t *n;
    uint16_t handle;
    int idx;

    if (!v) return NULL;

    n = &v->this_node;

    /* Find the vattr entry */
    while (n) {
        v = CONTAINER_OF(n, ble_gatt_attr_t, this_node);
        if (v == attr) break;
        else {n = n->next; continue;}
    }

    if (!n) {
        LOGE(MOD, "No vattr entry found for peripheral (%d)", hdl);
        return NULL;
    }

    /* vattr found, use the handle to find attr entry*/
    handle = v->handle;
    LOGD(MOD, "Attr handle to find: 0x%04x, in %s", handle, __func__);
    for (idx = 0; idx < g_peri[hdl].attr_num; idx++) {
        LOGD(MOD, "Checking handle: 0x%04x", g_peri[hdl].itbl[idx].handle);
        if (g_peri[hdl].itbl[idx].handle == handle) break;
    }

    if (idx >= g_peri[hdl].attr_num) {
        LOGE(MOD, "No attr entry found for peripheral (%d)", hdl);
        return NULL;
    }

    /* attr found */
    LOGD(MOD, "Attr found on idx: %d", idx);
    return &g_peri[hdl].attr[idx];
}

static void indicate_cb
(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    uint8_t err
)
{
    if (err != 0) {
        LOGE(MOD, "Indication fail (err: %d, %p).", err, attr);
    } else {
        LOGD(MOD, "Indication success (%p).", attr);
    }
}

void ble_attr_indicate
(
    ble_gatt_attr_t *attr,
    peripheral_hdl_t hdl,
    uint16_t len,
    const uint8_t *data
)
{
    struct bt_gatt_attr *a;
    struct bt_gatt_indicate_params params;

    a = find_attr_entry(attr, hdl);
    if (!a) return;

    params.attr = a;
    params.func = indicate_cb;
    params.data = (const void *)data;
    params.len = len;

    bt_gatt_indicate(NULL, &params);
}

void ble_attr_notify
(
    ble_gatt_attr_t *attr,
    peripheral_hdl_t hdl,
    uint16_t len,
    const uint8_t *data
)
{
    struct bt_gatt_attr *a;

    a = find_attr_entry(attr, hdl);
    if (!a) return;

    /* attr found, notify it */
    bt_gatt_notify(NULL, a, (const void *)data, len);
}

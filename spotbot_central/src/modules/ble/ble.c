#include "../../common/messages.h"
#include "../../common/nvm/nv_handler.h"
#include "zephyr/sys/util.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/printk.h>
#include <zephyr/types.h>
#include <zephyr/zbus/zbus.h>

#define STACKSIZE 2048
#define BLE_INT_THREAD_PRIORITY 6

// Define constants for workqueue
#define WORQ_THREAD_STACK_SIZE 512
#define WORKQ_PRIORITY 4 // Lowest priority workqueue

// Define parameters for the frame
#define MAX_FRAME_SIZE 25

LOG_MODULE_REGISTER(Ble_module, LOG_LEVEL_DBG);
ZBUS_CHAN_DECLARE(TX_CHANNEL);
ZBUS_CHAN_DECLARE(CONFIG_UPDATE_CHANNEL);

// Define queue structure
static K_THREAD_STACK_DEFINE(work_q_stack, WORQ_THREAD_STACK_SIZE);
static struct k_work_q tx_work_q = {0};

// Creatt Transmission_frame structure and offload function
struct Transmission_frame {
    struct k_work work;
    bt_addr_t addr;
    uint8_t ad_data[MAX_FRAME_SIZE];
    uint16_t len;
};

static struct Transmission_frame tx_frame = {0};

static void device_found(const bt_addr_le_t* addr, int8_t rsi, uint8_t type,
                         struct net_buf_simple* ad)
{
    char addr_str[BT_ADDR_LE_STR_LEN];

    /* We're only interested in connectable events */
    if (type != BT_GAP_ADV_TYPE_ADV_SCAN_IND) {
        return;
    }

    /* Can only connect to nordic devices */
    if ((ad->len < 4) || (ad->len > MAX_FRAME_SIZE)) {
        return;
    }

    if (!(ad->data[2] == 0x59 && ad->data[3] == 0x00)) {
        return;
    }

    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));

    // Copy recived frame into buffer for transmission.
    tx_frame.len = ad->len;
    memcpy(tx_frame.addr.val, addr->a.val, BT_ADDR_SIZE);
    memcpy(tx_frame.ad_data, ad->data, tx_frame.len);

    k_work_submit(&tx_frame.work);
    LOG_DBG("Device found: %s (RSSI %d) \n", addr_str, rsi);
    return;
}

static void start_scan(void)
{
    int err;
    struct bt_le_scan_param scan_param = {
        .type = BT_LE_SCAN_TYPE_ACTIVE,
        .options = BT_LE_SCAN_OPT_CODED,
        .interval = BT_GAP_SCAN_FAST_INTERVAL,
        .window = BT_GAP_SCAN_FAST_INTERVAL,
        .timeout = 0,
        .interval_coded = 0,
        .window_coded = 0,
    };

    /* This demo doesn't require active scan */
    err = bt_le_scan_start(&scan_param, device_found);
    if (err) {
        LOG_ERR("Scanning failed to start (err %d)\n", err);
        return;
    }

    LOG_INF("Scanning successfully started\n");
}

void dl_tx_frame_handler(struct k_work* work_tem)
{
    struct Transmission_frame* _tx_frame = CONTAINER_OF(work_tem, struct Transmission_frame, work);

    // TODO: process raw data and send on bus
    node_data* node = (node_data*)malloc(sizeof(node_data));

    int addr = ((int16_t)_tx_frame->ad_data[_tx_frame->len - 11] << 8) |
               _tx_frame->ad_data[_tx_frame->len - 10];
    node->x = ((int16_t)_tx_frame->ad_data[_tx_frame->len - 9] << 8) |
              _tx_frame->ad_data[_tx_frame->len - 8];
    node->y = ((int16_t)_tx_frame->ad_data[_tx_frame->len - 7] << 8) |
              _tx_frame->ad_data[_tx_frame->len - 6];
    node->z = ((int16_t)_tx_frame->ad_data[_tx_frame->len - 5] << 8) |
              _tx_frame->ad_data[_tx_frame->len - 4];
    node->temp = ((uint16_t)_tx_frame->ad_data[_tx_frame->len - 2] << 2) |
                 _tx_frame->ad_data[_tx_frame->len - 1];
    node->status = _tx_frame->ad_data[_tx_frame->len - 3];

    if (addr == 0x0003) {
        memcpy(node->node_id.val, _tx_frame->addr.val, BT_ADDR_SIZE);
    }
    else if (addr == 0x0203) {
        node->node_id.val[5] = _tx_frame->ad_data[_tx_frame->len - 17];
        node->node_id.val[4] = _tx_frame->ad_data[_tx_frame->len - 16];
        node->node_id.val[3] = _tx_frame->ad_data[_tx_frame->len - 15];
        node->node_id.val[2] = _tx_frame->ad_data[_tx_frame->len - 14];
        node->node_id.val[1] = _tx_frame->ad_data[_tx_frame->len - 13];
        node->node_id.val[0] = _tx_frame->ad_data[_tx_frame->len - 12];
    }
    else if (addr == 0x0007) {
        uint8_t ip[4] = {0, 0, 0, 0};
        // Extract new IP from the BLE frame
        ip[3] = _tx_frame->ad_data[_tx_frame->len - 9];
        ip[2] = _tx_frame->ad_data[_tx_frame->len - 8];
        ip[1] = _tx_frame->ad_data[_tx_frame->len - 7];
        ip[0] = _tx_frame->ad_data[_tx_frame->len - 6];

        // Check validity of recived ip and update stored ip
        nvError err = write_ip(ip);
        if (err == E_ALREADY_PRESENT) {
            LOG_INF("Ip already present");
            return;
        }

        // Notify of change ip to all listeners
        zbus_chan_notify(&CONFIG_UPDATE_CHANNEL, K_NO_WAIT);
        return;
    }

    // Put the data on channel
    zbus_chan_pub(&TX_CHANNEL, node, K_FOREVER);
}

static void init_ble(int thread_id, void* unused, void* unused2)
{
    int err;
    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return;
    }

    err = nv_init();
    if (err != 0) {
        LOG_ERR("NV is not initialized\n");
        // TODO: implement fallback options
    }

    // Start the workqueue
    k_work_queue_init(&tx_work_q);
    k_work_queue_start(&tx_work_q, work_q_stack, K_THREAD_STACK_SIZEOF(work_q_stack),
                       WORKQ_PRIORITY, NULL);

    // Initialize work item and connect it to its handler function
    k_work_init(&tx_frame.work, dl_tx_frame_handler);

    LOG_INF("Bluetooth initialized\n");
    start_scan();
}

K_THREAD_DEFINE(ble_init_thread, STACKSIZE, init_ble, 1, NULL, NULL, BLE_INT_THREAD_PRIORITY, 0, 0);

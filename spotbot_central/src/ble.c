#include "zephyr/sys/util.h"

#include <stddef.h>
#include <stdint.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/printk.h>
#include <zephyr/types.h>


LOG_MODULE_REGISTER(Ble_module, LOG_LEVEL_DBG);

static void start_scan(void);

static struct bt_conn* default_conn;

#define STACKSIZE 2048
#define BLE_INT_THREAD_PRIORITY 6

// Define constants for workqueue
#define WORQ_THREAD_STACK_SIZE 512
#define WORKQ_PRIORITY 4 // Lowest priority workqueue

// Define queue structure
static K_THREAD_STACK_DEFINE(work_q_stack, WORQ_THREAD_STACK_SIZE);
static struct k_work_q tx_work_q = {0};

// Creatt Transmission_frame structure and offload function
struct Transmission_frame {
    struct k_work work;
    uint8_t data[25];
    uint8_t size;
};

static struct Transmission_frame tx_frame = {0};

static void device_found(const bt_addr_le_t* addr, int8_t rsi, uint8_t type,
                         struct net_buf_simple* ad)
{
    char addr_str[BT_ADDR_LE_STR_LEN];

    if (default_conn) {
        return;
    }

    /* We're only interested in connectable events */
    if (type != BT_GAP_ADV_TYPE_ADV_SCAN_IND) {
        return;
    }

    /* Can only connect to nordic devices */
    if (ad->len < 4) {
        return;
    }

    if (!(ad->data[2] == 0x59 && ad->data[3] == 0x00)) {
        return;
    }

    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));

    // Copy recived frame into buffer for transmission.
    tx_frame.size = ad->len;
    memcpy(tx_frame.data, ad->data, tx_frame.size);

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

void offload_function(struct k_work* work_tem)
{
    struct Transmission_frame* _tx_frame = CONTAINER_OF(work_tem, struct Transmission_frame, work);
    LOG_DBG("%d", _tx_frame->size);
    LOG_HEXDUMP_DBG(_tx_frame->data, _tx_frame->size, "Data: ");

    // TODO: Put the data on channel
}

static void init_ble(int thread_id, void* unused, void* unused2)
{
    int err;
    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return;
    }

    // Start the workqueue
    k_work_queue_init(&tx_work_q);
    k_work_queue_start(&tx_work_q, work_q_stack, K_THREAD_STACK_SIZEOF(work_q_stack),
                       WORKQ_PRIORITY, NULL);

    // Initialize work item and connect it to its handler function
    k_work_init(&tx_frame.work, offload_function);

    LOG_INF("Bluetooth initialized\n");
    start_scan();
}

K_THREAD_DEFINE(consumer, STACKSIZE, init_ble, 1, NULL, NULL, BLE_INT_THREAD_PRIORITY, 0, 0);

#include "common/messages.h"
#include "modules/nvm/nv_handler.h"
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

#define TEST_MODE false
#if TEST_MODE

#define STACKSIZE 1048
#define TEST_APP_THREAD_PRIORITY 2

LOG_MODULE_REGISTER(test_module, LOG_LEVEL_DBG);

ZBUS_CHAN_DECLARE(TX_CHANNEL);
ZBUS_CHAN_DECLARE(CONFIG_UPDATE_CHANNEL);

const uint8_t DEFAULT_IP[4] = {10, 20, 30, 40};

// This is Highest priority thread which blocks all other threads from starting.
// Implement small block which you want to test here.
static void test_app(int thread_id, void* unused, void* unused2)
{
    nvError err = nv_init();
    if (err != E_OK) {
        LOG_INF("Init Error code %d", err);
    }

    uint8_t ip[4] = {1, 2, 3, 4};
    err = read_ip(ip, DEFAULT_IP);
    if (err != E_OK) {
        LOG_INF("Read Error code %d", err);
    }

    uint8_t ip_2[4] = {100, 120, 130, 140};
    err = write_ip(ip_2);
    if (err != E_OK) {
        LOG_INF("Read Error code %d", err);
    }

    while (1) {
    }
}


K_THREAD_DEFINE(test_app_thread, STACKSIZE, test_app, 1, NULL, NULL, TEST_APP_THREAD_PRIORITY, 0,
                0);
#endif
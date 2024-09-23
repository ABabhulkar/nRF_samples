#include "../../common/messages.h"

#include <stddef.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/types.h>
#include <zephyr/zbus/zbus.h>

LOG_MODULE_REGISTER(mqtt_handler_module, LOG_LEVEL_DBG);

ZBUS_CHAN_DECLARE(TX_CHANNEL);

#define STACKSIZE 1024
#define MQTT_INT_THREAD_PRIORITY 6
#define MQTT_MONITOR_THREAD_PRIORITY 7

// TODO: Define topic list

// Define listener for tx_ch channel
static void tx_frame_recived_cb(const struct zbus_channel* chan)
{
    const struct tx_msg* msg = zbus_chan_const_msg(chan);

    // TODO: Process the msg here and send it to lower layer for transmission
}

ZBUS_LISTENER_DEFINE(tx_channel, tx_frame_recived_cb);
ZBUS_CHAN_ADD_OBS(TX_CHANNEL, tx_channel, 1);

static void init_mqtt_handler(int thread_id, void* unused, void* unused2)
{
    // Step1: Register modem(wifi/4G)
    // Step2: connect to mqtt
    // Step3: subscribe to topics
    // Step4: send started ack
}

static void mqtt_monitor_conn(int thread_id, void* unused, void* unused2)
{
    // step1: wait for initialization
    // step2: start timer
    // step3: check connection
    // step4: if not connected try reconnection
}

K_THREAD_DEFINE(mqtt_monitor_thread, STACKSIZE, mqtt_monitor_conn, 1, NULL, NULL,
                MQTT_MONITOR_THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(mqtt_init_thread, STACKSIZE, init_mqtt_handler, 1, NULL, NULL,
                MQTT_INT_THREAD_PRIORITY, 0, 0);

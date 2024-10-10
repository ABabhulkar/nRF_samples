#include "../../common/messages.h"
#include "../../common/nvm/nv_handler.h"
#include "network_interface.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/types.h>
#include <zephyr/zbus/zbus.h>

LOG_MODULE_REGISTER(mqtt_handler_module, LOG_LEVEL_DBG);

ZBUS_CHAN_DECLARE(TX_CHANNEL);
ZBUS_CHAN_DECLARE(CONFIG_UPDATE_CHANNEL);

K_SEM_DEFINE(sem_init_complete, 0, 1);

#define STACKSIZE 1024
#define MQTT_INT_THREAD_PRIORITY 6
#define MQTT_MONITOR_THREAD_PRIORITY 7
#define CONFIG_UPDATE_THREAD_PRIORITY 5

#define DEFAULT_PORT 1883

#define TOPIC "Default"
#define CON_ACK "Connected to server"

static const uint8_t DEFAULT_IP[4] = {192, 168, 2, 1};

// Define network
static Connection* con;

// Define topic list
static inline const char** get_topics()
{
    // TODO: check if some list impl can be used here
    static const char* topics[] = {"Hello", "world", "from", "C"};

    return topics;
}

// Define listener for tx_ch channel
static void tx_frame_recived_cb(const struct zbus_channel* chan)
{
    const node_data* node = zbus_chan_const_msg(chan);
    if (con->isConnected()) {
        // Process the node here and send node_data to lower layer for transmission
        char topic[13]; // hex representation of nodeID;
        snprintf(topic, sizeof(topic), "%02X%02X%02X%02X%02X%02X", node->node_id.val[5],
                 node->node_id.val[4], node->node_id.val[3], node->node_id.val[2],
                 node->node_id.val[1], node->node_id.val[0]);

        char msg[30]; // char byte array needed for transmission.
        snprintf(msg, sizeof(msg), "%d,%d,%d,%d", node->x, node->y, node->z, node->status);

        con->sendFrame(topic, msg);

        LOG_DBG("%s;%s;  %d", topic, msg, node->status);
    }
    else {
        con->reconnect();
    }
}

ZBUS_LISTENER_DEFINE(tx_channel, tx_frame_recived_cb);
ZBUS_CHAN_ADD_OBS(TX_CHANNEL, tx_channel, 1);

ZBUS_SUBSCRIBER_DEFINE(config_update_sub, 2);
ZBUS_CHAN_ADD_OBS(CONFIG_UPDATE_CHANNEL, config_update_sub, 1);

static void subscriber_config_update(void* sub)
{
    const struct zbus_channel* chan;
    const struct zbus_observer* subscriber = sub;

    while (!zbus_sub_wait(subscriber, &chan, K_FOREVER)) {
        if (&CONFIG_UPDATE_CHANNEL != chan) {
            LOG_ERR("Wrong channel %p!", chan);
            continue;
        }

        LOG_INF("New IP recived");
        uint8_t temp_ip[4];
        if (read_ip(temp_ip, DEFAULT_IP) == E_OK) {
            memcpy(con->con_param.ip, temp_ip, sizeof(temp_ip));
            con->reconnect();
        }
        else {
            LOG_ERR("Error reading new IP");
        }
    }
}

static void init_mqtt_handler(int thread_id, void* unused, void* unused2)
{
    // Init nv storage
    if (nv_init() != 0) {
        LOG_ERR("NV is not initialized\n");
        // TODO: implement fallback options
    }

    // Define connection parameters
    Connection_param con_param = {.port = DEFAULT_PORT, .timeout = 0};

    // Read ip from nvm and pass it on to connect
    if (read_ip(con_param.ip, DEFAULT_IP) != E_OK) {
        LOG_DBG("Loading default ip due to error");
    }

    // Register modem(wifi/4G)
    con = init_connection(WIFI_ESP, con_param);
    if (con == NULL) {
        // TODO: perform error handling based on type of error
        return;
    }

    // Initialize device
    con->init();

    // connect to mqtt
    con->connect(&con->con_param);

    // Subscribe to topics
    con->subscribe(get_topics(), 4);

    // Get gateway mac and send as msg
    char* topic = con->getMAC();

    // Send started ack (topic= mac of this device)
    con->sendFrame(topic, CON_ACK);

    k_sem_give(&sem_init_complete);
}

static void mqtt_monitor_conn(int thread_id, void* unused, void* unused2)
{
    // wait for initialization
    k_sem_take(&sem_init_complete, K_FOREVER);

    while (1) {
        // if not connected try reconnection
        if (!con->isConnected()) {
            // TODO: configure a timeout
            con->reconnect();
        }
        k_sleep(K_SECONDS(5));
    }
}

K_THREAD_DEFINE(mqtt_monitor_thread, STACKSIZE, mqtt_monitor_conn, 1, NULL, NULL,
                MQTT_MONITOR_THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(mqtt_init_thread, STACKSIZE, init_mqtt_handler, 1, NULL, NULL,
                MQTT_INT_THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(config_update_task, STACKSIZE, subscriber_config_update, &config_update_sub, NULL,
                NULL, CONFIG_UPDATE_THREAD_PRIORITY, 0, 0);
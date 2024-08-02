/** @file
 *  @brief Nordic UART Bridge Service (NUS) sample with dynamic thread for sensor
 */

#include "zephyr/kernel/thread.h"

#include <bluetooth/services/nus.h>
#include <dk_buttons_and_leds.h>
#include <soc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>
#include <zephyr/settings/settings.h>
#include <zephyr/types.h>


#define LOG_MODULE_NAME ble_node
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define STACKSIZE CONFIG_BT_NUS_THREAD_STACK_SIZE
#define PRIORITY_BLE_TRANSMIT 7
#define PRIORITY_SERVER 8
#define PRIORITY_CLIENT 8

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define RUN_STATUS_LED DK_LED1
#define CON_STATUS_LED DK_LED2
#define SW0_NODE DK_BTN1_MSK

#define RUN_LED_BLINK_INTERVAL 1000
#define RESPONSE_TIMEOUT K_MSEC(5000)

#define START_COMMAND "start"
#define ACK_OK "ok"
#define ACK_N_OK "n_ok"

enum state_e { SERVER, CLIENT, IDLE };

static K_SEM_DEFINE(ble_init_ok, 0, 1);
static K_SEM_DEFINE(button_state_pressed, 0, 1);
static K_SEM_DEFINE(start_command, 0, 1);

static struct bt_conn* current_conn;
static struct bt_conn* auth_conn;

struct uart_data_t {
    void* fifo_reserved;
    uint8_t data[CONFIG_BT_NUS_UART_BUFFER_SIZE];
    uint16_t len;
};

// static K_FIFO_DEFINE(fifo_uart_tx_data);
static K_FIFO_DEFINE(fifo_tx_data);
static K_FIFO_DEFINE(fifo_server_response);
static K_FIFO_DEFINE(fifo_client_response);

static enum state_e state = IDLE;

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

static void connected(struct bt_conn* conn, uint8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    if (err) {
        LOG_ERR("Connection failed (err %u)", err);
        return;
    }

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    LOG_INF("Connected %s", addr);

    current_conn = bt_conn_ref(conn);

    dk_set_led_on(CON_STATUS_LED);
}

static void disconnected(struct bt_conn* conn, uint8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("Disconnected: %s (reason %u)", addr, reason);

    if (auth_conn) {
        bt_conn_unref(auth_conn);
        auth_conn = NULL;
    }

    if (current_conn) {
        bt_conn_unref(current_conn);
        current_conn = NULL;
        dk_set_led_off(CON_STATUS_LED);
    }
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

static void bt_receive_cb(struct bt_conn* conn, const uint8_t* const data, uint16_t len)
{
    char addr[BT_ADDR_LE_STR_LEN] = {0};

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, ARRAY_SIZE(addr));

    LOG_INF("Received data from: %s", addr);

    for (uint16_t pos = 0; pos != len;) {
        struct uart_data_t* tx = k_malloc(sizeof(*tx));

        if (!tx) {
            LOG_WRN("Not able to allocate UART send data buffer");
            return;
        }

        /* Keep the last byte of TX buffer for potential LF char. */
        size_t tx_data_size = sizeof(tx->data) - 1;

        if ((len - pos) > tx_data_size) {
            tx->len = tx_data_size;
        }
        else {
            tx->len = (len - pos);
        }

        memcpy(tx->data, &data[pos], tx->len);

        pos += tx->len;

        /* Append the LF character when the CR character triggered
         * transmission from the peer.
         */
        if ((pos == len) && (data[len - 1] == '\r')) {
            tx->data[tx->len] = '\n';
            tx->len++;
        }

        // Logic to identify and route msg to threads
        if (state == SERVER) {
            k_fifo_put(&fifo_server_response, tx);
        }
        else if (state == CLIENT) {
            k_fifo_put(&fifo_client_response, tx);
        }
        else {
            if (strncmp(tx->data, "start", 5) == 0) {
                k_sem_give(&start_command);
            }
        }
    }
}

static struct bt_nus_cb nus_cb = {
    .received = bt_receive_cb,
};

void error(void)
{
    dk_set_leds_state(DK_ALL_LEDS_MSK, DK_NO_LEDS_MSK);

    while (true) {
        /* Spin for ever */
        k_sleep(K_MSEC(1000));
    }
}

void button_pressed(uint32_t button_state, uint32_t has_changed)
{
    if ((button_state == 1) && (state == IDLE)) {
        LOG_ERR("Button 1");
        k_sem_give(&button_state_pressed);
    }
}

static void configure_gpio(void)
{
    int err;

    err = dk_leds_init();
    if (err) {
        LOG_ERR("Cannot init LEDs (err: %d)", err);
    }

    err = dk_buttons_init(&button_pressed);
    if (err) {
        LOG_ERR("Cannot init LEDs (err: %d)", err);
    }
}

int main(void)
{
    int blink_status = 0;
    int err = 0;

    configure_gpio();

    err = bt_enable(NULL);
    if (err) {
        error();
    }

    LOG_INF("Bluetooth initialized");

    k_sem_give(&ble_init_ok);

    if (IS_ENABLED(CONFIG_SETTINGS)) {
        settings_load();
    }

    err = bt_nus_init(&nus_cb);
    if (err) {
        LOG_ERR("Failed to initialize UART service (err: %d)", err);
        return 0;
    }

    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
        return 0;
    }

    for (;;) {
        dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
        k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
    }
}

// TODO: Move code of server side to one c file and client side to other

// Runnable for BLE thread which will take care of ble transmission.
void ble_write_thread(void)
{
    /* Don't go any further until BLE is initialized */
    k_sem_take(&ble_init_ok, K_FOREVER);

    for (;;) {
        /* Wait indefinitely for data to be sent over bluetooth */
        struct uart_data_t* buf = k_fifo_get(&fifo_tx_data, K_FOREVER);

        if (bt_nus_send(NULL, buf->data, buf->len)) {
            LOG_WRN("Failed to send data over BLE connection");
        }

        k_free(buf);
    }
}

void reset_state()
{
    state = IDLE;
}

void button_handler(void)
{
    while (1) {
        // wait for button press
        k_sem_take(&button_state_pressed, K_FOREVER);
        state = SERVER;

        // Allocate memory for the UART data structure
        struct uart_data_t* request = k_malloc(sizeof(*request));
        if (request) {
            // Convert sensor reading to string and store in buf->data
            memcpy(request->data, START_COMMAND, sizeof(START_COMMAND));
            request->len = sizeof(START_COMMAND);

            // Send to BLE via FIFO
            k_fifo_put(&fifo_tx_data, request);
        }
        else {
            LOG_ERR("Failed to allocate memory for buf");
            reset_state();
            continue;
        }

        // wait for response with timeout
        struct uart_data_t* response = k_fifo_get(&fifo_server_response, RESPONSE_TIMEOUT); // temp
        if (response) {
            LOG_HEXDUMP_INF(response->data, response->len, "random Numbers");
            // TODO: add crypto logic here
        }
        else {
            LOG_ERR("Timeout happened");
            reset_state();
            continue;
        }

        // Wait for ACK with timeout
        response = k_fifo_get(&fifo_server_response, RESPONSE_TIMEOUT); // temp
        if (response) {
            LOG_INF("Response %c", response->data[0]);
        }

        reset_state();
        //  Sleep for 500 ms
        k_msleep(500);
    }
}

bool validate(struct uart_data_t* data)
{
    // TODO: validate the date and generate ack
    return true;
}

// Runnable for sensor thread which take care of sensor reading
void data_generator(void* arg0, void* arg1, void* agr2)
{
    char sensor_reading[5];
    bool is_valid = false;

    while (1) {
        k_sem_take(&start_command, K_FOREVER);
        state = CLIENT;

        sys_rand_get(sensor_reading, 5);
        LOG_HEXDUMP_INF(sensor_reading, sizeof(sensor_reading), "Sensor_reading :");

        //  Allocate memory for the UART data structure
        struct uart_data_t* buf = k_malloc(sizeof(*buf));
        if (buf) {
            // store sensor readings into buffer
            memcpy(buf->data, sensor_reading, sizeof(sensor_reading));
            buf->len = sizeof(sensor_reading);

            // Send to BLE via FIFO
            k_fifo_put(&fifo_tx_data, buf);
        }
        else {
            LOG_ERR("Failed to allocate memory for buf");
            reset_state();
            continue;
        }

        // wait for response with timeout
        struct uart_data_t* response = k_fifo_get(&fifo_client_response, RESPONSE_TIMEOUT); // temp
        if (response) {
            is_valid = validate(response);
        }
        else {
            LOG_ERR("Timeout happened");
            reset_state();
            continue;
        }

        //  Generate Ack for data received data
        struct uart_data_t* ack = k_malloc(sizeof(*ack));
        if (buf) {
            if (is_valid) {
                memcpy(ack->data, ACK_OK, sizeof(ACK_OK));
                ack->len = sizeof(ACK_OK);
            }
            else {
                memcpy(ack->data, ACK_N_OK, sizeof(ACK_N_OK));
                ack->len = sizeof(ACK_N_OK);
            }

            // Send to BLE via FIFO
            k_fifo_put(&fifo_tx_data, ack);
        }
        else {
            LOG_ERR("Failed to allocate memory for buf");
        }

        reset_state();
    }
}

K_THREAD_DEFINE(ble_write_thread_id, STACKSIZE, ble_write_thread, NULL, NULL, NULL,
                PRIORITY_BLE_TRANSMIT, 0, 0);
K_THREAD_DEFINE(button_handler_thread, STACKSIZE, button_handler, NULL, NULL, NULL, PRIORITY_SERVER,
                0, 0);
K_THREAD_DEFINE(data_generator_thread, STACKSIZE, data_generator, NULL, NULL, NULL, PRIORITY_CLIENT,
                0, 0);
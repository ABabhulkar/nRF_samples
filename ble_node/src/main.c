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
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>
#include <zephyr/settings/settings.h>
#include <zephyr/types.h>
#include <zephyr/usb/usb_device.h>


#define LOG_MODULE_NAME peripheral_uart
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define STACKSIZE CONFIG_BT_NUS_THREAD_STACK_SIZE
#define PRIORITY 7

// Stacksize for sensor thread.
#define SENSOR_STACKSIZE 256

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000

#define CON_STATUS_LED DK_LED2

#define KEY_PASSKEY_ACCEPT DK_BTN1_MSK
#define KEY_PASSKEY_REJECT DK_BTN2_MSK

#define UART_BUF_SIZE CONFIG_BT_NUS_UART_BUFFER_SIZE
#define UART_WAIT_FOR_BUF_DELAY K_MSEC(50)
#define UART_WAIT_FOR_RX CONFIG_BT_NUS_UART_RX_WAIT_TIME

static K_SEM_DEFINE(ble_init_ok, 0, 1);

static struct bt_conn* current_conn;
static struct bt_conn* auth_conn;

static const struct device* uart = DEVICE_DT_GET(DT_CHOSEN(nordic_nus_uart));
static struct k_work_delayable uart_work;

struct uart_data_t {
    void* fifo_reserved;
    uint8_t data[UART_BUF_SIZE];
    uint16_t len;
};

static K_FIFO_DEFINE(fifo_uart_tx_data);
static K_FIFO_DEFINE(fifo_uart_rx_data);

// Defining stack for dynamic thread
K_THREAD_STACK_DEFINE(sensor_stack_area, STACKSIZE);
static bool is_start_sensor = false;

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

void sensor_thread(void* arg0, void* arg1, void* agr2);

static void uart_cb(const struct device* dev, struct uart_event* evt, void* user_data)
{
    ARG_UNUSED(dev);

    static size_t aborted_len;
    struct uart_data_t* buf;
    static uint8_t* aborted_buf;
    static bool disable_req;

    switch (evt->type) {
    case UART_TX_DONE:
        LOG_INF("UART_TX_DONE");
        if ((evt->data.tx.len == 0) || (!evt->data.tx.buf)) {
            return;
        }

        if (aborted_buf) {
            buf = CONTAINER_OF(aborted_buf, struct uart_data_t, data[0]);
            aborted_buf = NULL;
            aborted_len = 0;
        }
        else {
            buf = CONTAINER_OF(evt->data.tx.buf, struct uart_data_t, data[0]);
        }

        k_free(buf);

        buf = k_fifo_get(&fifo_uart_tx_data, K_NO_WAIT);
        if (!buf) {
            return;
        }

        if (uart_tx(uart, buf->data, buf->len, SYS_FOREVER_MS)) {
            LOG_WRN("Failed to send data over UART");
        }
        break;

    case UART_RX_RDY:
        LOG_INF("UART_RX_RDY");
        buf = CONTAINER_OF(evt->data.rx.buf, struct uart_data_t, data[0]);
        buf->len += evt->data.rx.len;

        if (disable_req) {
            return;
        }

        if ((evt->data.rx.buf[buf->len - 1] == '\n') || (evt->data.rx.buf[buf->len - 1] == '\r')) {
            disable_req = true;
            uart_rx_disable(uart);
        }
        break;

    case UART_RX_DISABLED:
        LOG_INF("UART_RX_DISABLED");
        disable_req = false;

        buf = k_malloc(sizeof(*buf));
        if (buf) {
            buf->len = 0;
        }
        else {
            LOG_WRN("Not able to allocate UART receive buffer");
            k_work_reschedule(&uart_work, UART_WAIT_FOR_BUF_DELAY);
            return;
        }

        uart_rx_enable(uart, buf->data, sizeof(buf->data), UART_WAIT_FOR_RX);
        break;

    case UART_RX_BUF_REQUEST:
        LOG_INF("UART_RX_BUF_REQUEST");
        buf = k_malloc(sizeof(*buf));
        if (buf) {
            buf->len = 0;
            uart_rx_buf_rsp(uart, buf->data, sizeof(buf->data));
        }
        else {
            LOG_WRN("Not able to allocate UART receive buffer");
        }
        break;

    case UART_RX_BUF_RELEASED:
        LOG_INF("UART_RX_BUF_RELEASED");
        buf = CONTAINER_OF(evt->data.rx_buf.buf, struct uart_data_t, data[0]);

        if (buf->len > 0) {
            k_fifo_put(&fifo_uart_rx_data, buf);
        }
        else {
            k_free(buf);
        }
        break;

    case UART_TX_ABORTED:
        LOG_INF("UART_TX_ABORTED");
        if (!aborted_buf) {
            aborted_buf = (uint8_t*)evt->data.tx.buf;
        }

        aborted_len += evt->data.tx.len;
        buf = CONTAINER_OF((void*)aborted_buf, struct uart_data_t, data);

        uart_tx(uart, &buf->data[aborted_len], buf->len - aborted_len, SYS_FOREVER_MS);
        break;

    default:
        break;
    }
}

static void uart_work_handler(struct k_work* item)
{
    struct uart_data_t* buf;

    buf = k_malloc(sizeof(*buf));
    if (buf) {
        buf->len = 0;
    }
    else {
        LOG_WRN("Not able to allocate UART receive buffer");
        k_work_reschedule(&uart_work, UART_WAIT_FOR_BUF_DELAY);
        return;
    }
    // TODO: check usage
    uart_rx_enable(uart, buf->data, sizeof(buf->data), UART_WAIT_FOR_RX);
}

static int uart_init(void)
{
    int err;
    int pos;
    struct uart_data_t* rx;
    struct uart_data_t* tx;

    if (!device_is_ready(uart)) {
        return -ENODEV;
    }

    rx = k_malloc(sizeof(*rx));
    if (rx) {
        rx->len = 0;
    }
    else {
        return -ENOMEM;
    }

    k_work_init_delayable(&uart_work, uart_work_handler);

    err = uart_callback_set(uart, uart_cb, NULL);
    if (err) {
        k_free(rx);
        LOG_ERR("Cannot initialize UART callback");
        return err;
    }

    // TODO: maybe not useful
    if (IS_ENABLED(CONFIG_UART_LINE_CTRL)) {
        LOG_INF("Wait for DTR");
        while (true) {
            uint32_t dtr = 0;

            uart_line_ctrl_get(uart, UART_LINE_CTRL_DTR, &dtr);
            if (dtr) {
                break;
            }
            /* Give CPU resources to low priority threads. */
            k_sleep(K_MSEC(100));
        }
        LOG_INF("DTR set");
        err = uart_line_ctrl_set(uart, UART_LINE_CTRL_DCD, 1);
        if (err) {
            LOG_WRN("Failed to set DCD, ret code %d", err);
        }
        err = uart_line_ctrl_set(uart, UART_LINE_CTRL_DSR, 1);
        if (err) {
            LOG_WRN("Failed to set DSR, ret code %d", err);
        }
    }

    tx = k_malloc(sizeof(*tx));

    if (tx) {
        pos = snprintf(tx->data, sizeof(tx->data), "Starting Nordic UART service example\r\n");

        if ((pos < 0) || (pos >= sizeof(tx->data))) {
            k_free(rx);
            k_free(tx);
            LOG_ERR("snprintf returned %d", pos);
            return -ENOMEM;
        }

        tx->len = pos;
    }
    else {
        k_free(rx);
        return -ENOMEM;
    }

    err = uart_tx(uart, tx->data, tx->len, SYS_FOREVER_MS);
    if (err) {
        k_free(rx);
        k_free(tx);
        LOG_ERR("Cannot display welcome message (err: %d)", err);
        return err;
    }

    err = uart_rx_enable(uart, rx->data, sizeof(rx->data), UART_WAIT_FOR_RX);
    if (err) {
        LOG_ERR("Cannot enable uart reception (err: %d)", err);
        /* Free the rx buffer only because the tx buffer will be handled in the callback */
        k_free(rx);
    }

    return err;
}

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
    int err;
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

        if (strncmp(tx->data, "start", 5) == 0) {
            is_start_sensor = true;
        }

        // TODO: Identify if this is intended data or not and display over uart.
        err = uart_tx(uart, tx->data, tx->len, SYS_FOREVER_MS);
        if (err) {
            k_fifo_put(&fifo_uart_tx_data, tx);
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

static void configure_gpio(void)
{
    int err;

    err = dk_leds_init();
    if (err) {
        LOG_ERR("Cannot init LEDs (err: %d)", err);
    }
}

int main(void)
{
    int blink_status = 0;
    int err = 0;

    configure_gpio();

    err = uart_init();
    if (err) {
        error();
    }

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

    // TODO: create new project and strip everything related to uart as it is not required

    for (;;) {
        if (is_start_sensor) {
            is_start_sensor = false;
            struct k_thread sensor_thread_data;
            k_thread_create(&sensor_thread_data, sensor_stack_area,
                            K_THREAD_STACK_SIZEOF(sensor_stack_area), sensor_thread, NULL, NULL,
                            NULL, PRIORITY, 0, K_NO_WAIT);
        }

        dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
        k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
    }
}

// Runnable for BLE thread which will take care of ble transmission.
void ble_write_thread(void)
{
    /* Don't go any further until BLE is initialized */
    k_sem_take(&ble_init_ok, K_FOREVER);

    for (;;) {
        /* Wait indefinitely for data to be sent over bluetooth */
        struct uart_data_t* buf = k_fifo_get(&fifo_uart_rx_data, K_FOREVER);

        if (bt_nus_send(NULL, buf->data, buf->len)) {
            LOG_WRN("Failed to send data over BLE connection");
        }

        k_free(buf);
    }
}

// Runnable for sensor thread which take care of sensor reading
void sensor_thread(void* arg0, void* arg1, void* agr2)
{
    char sensor_reading[5];

    LOG_HEXDUMP_INF(sensor_reading, sizeof(sensor_reading), "Sensor_reading :");

    //  Allocate memory for the UART data structure
    struct uart_data_t* buf = k_malloc(sizeof(*buf));
    if (buf) {
        // Convert sensor reading to string and store in buf->data
        memcpy(buf->data, sensor_reading, sizeof(sensor_reading));
        buf->len = sizeof(sensor_reading);

        // Ensure the string is null-terminated
        // if (buf->len < sizeof(buf->data)) {
        //     buf->data[buf->len] = '\0';
        // }
        // else {
        //     buf->data[sizeof(buf->data) - 1] = '\0';
        //     buf->len = sizeof(buf->data) - 1; // Adjust length to maximum size
        // }

        // Send to BLE via FIFO
        k_fifo_put(&fifo_uart_rx_data, buf);
    }
    else {
        LOG_ERR("Failed to allocate memory for buf");
    }
}

K_THREAD_DEFINE(ble_write_thread_id, STACKSIZE, ble_write_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
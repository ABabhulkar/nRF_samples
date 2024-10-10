#include "esp.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_core.h>

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
#define MSG_SIZE 32

#define STACKSIZE 512
#define ESP_THREAD_PRIORITY 6
#define UART_MONITOR_DELAY 5

LOG_MODULE_REGISTER(esp, LOG_LEVEL_INF);

static const struct device* const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

static int rx_buf_pos;
static char rx_buf[MSG_SIZE];
static bool connected;

/* queue to store up to 10 messages (aligned to 4-byte boundary) */
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void serial_cb(const struct device* dev, void* user_data)
{
    uint8_t c;

    if (!uart_irq_update(uart_dev)) {
        return;
    }

    if (!uart_irq_rx_ready(uart_dev)) {
        return;
    }

    /* read until FIFO empty */
    while (uart_fifo_read(uart_dev, &c, 1) == 1) {
        if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
            /* terminate string */
            rx_buf[rx_buf_pos] = '\0';

            /* if queue is full, message is silently dropped */
            k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

            /* reset the buffer (it was copied to the msgq) */
            rx_buf_pos = 0;
        }
        else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
            rx_buf[rx_buf_pos++] = c;
        }
        /* else: characters beyond buffer size are dropped */
    }
}

/*
 * Print a null-terminated string character by character to the UART interface
 */
void print_uart(char* buf)
{
    int msg_len = strlen(buf);

    for (int i = 0; i < msg_len; i++) {
        uart_poll_out(uart_dev, buf[i]);
    }
}

void esp_connect(Connection_param* con_param)
{
    // send ip address over uart and ask for connection
}

void esp_subscribe(const char* topic[], uint16_t num_topics)
{
    // send list of topics to be subscribed
}

bool esp_isConnected()
{
    return connected;
}

void esp_sendFrame(const char* topic, const char* msg)
{
    // send frame over uart
    uint8_t size = strlen(topic) + strlen(msg) + 8;
    char tx_buf[size];
    snprintf(tx_buf, size, "ID;%s;%s;\r\n", topic, msg);
    print_uart(tx_buf);
}

void esp_reconnect()
{
    // send reconnection request
    LOG_DBG("Reconnect called");
}

// return: mack of the device in string form
char* esp_getMAC()
{
    return "dummy";
}

void esp_init()
{
    if (!device_is_ready(uart_dev)) {
        LOG_ERR("UART device not found!");
        return;
    }

    /* configure interrupt and callback to receive data */
    int ret = uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
    if (ret < 0) {
        if (ret == -ENOTSUP) {
            LOG_ERR("Interrupt-driven UART API support not enabled\n");
        }
        else if (ret == -ENOSYS) {
            LOG_ERR("UART device does not support interrupt-driven API\n");
        }
        else {
            LOG_ERR("Error setting UART callback: %d\n", ret);
        }
        return;
    }
    uart_irq_rx_enable(uart_dev);
    connected = false;
    LOG_INF("Init done");
}

static void esp_main(int thread_id, void* unused, void* unused2)
{
    char tx_buf[MSG_SIZE];

    // TODO: remove after testing
    connected = true;
    LOG_INF("Main started");
    while (k_msgq_get(&uart_msgq, &tx_buf, K_FOREVER) == 0) {
        // Process recived msg here and create proper actions
        LOG_INF("%s", tx_buf);
    }
}

K_THREAD_DEFINE(esp_main_thread, STACKSIZE, esp_main, 1, NULL, NULL, ESP_THREAD_PRIORITY, 0,
                UART_MONITOR_DELAY);
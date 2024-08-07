#include "server.h"

#include "common.h"

LOG_MODULE_DECLARE(LOG_MODULE_NAME);
static K_SEM_DEFINE(button_state_pressed, 0, 1);
K_FIFO_DEFINE(fifo_server_response);

void button_pressed(uint32_t button_state, uint32_t has_changed)
{
    if ((button_state == 1) && (get_state() == IDLE)) {
        LOG_ERR("Button 1");
        k_sem_give(&button_state_pressed);
    }
}

void button_handler(void)
{
    while (1) {
        // wait for button press
        k_sem_take(&button_state_pressed, K_FOREVER);
        request_state(SERVER);

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

K_THREAD_DEFINE(button_handler_thread, SERVER_THREAD_STACKSIZE, button_handler, NULL, NULL, NULL,
                PRIORITY_SERVER, 0, 0);
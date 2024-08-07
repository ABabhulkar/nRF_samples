#include "server.h"

#include "common.h"
#include "psa/crypto.h"
#include "psa/crypto_types.h"
#include "psa/crypto_values.h"
#include "zephyr/kernel.h"
#include "zephyr/logging/log.h"

#include <stdint.h>


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

void hash_response(struct uart_data_t* response)
{
    uint8_t hash[SHA256_SIZE];
    uint32_t olen;
    psa_status_t status;
    status =
        psa_hash_compute(PSA_ALG_SHA_256, response->data, response->len, hash, sizeof(hash), &olen);
    if (status != PSA_SUCCESS) {
        LOG_ERR("psa hash failed: %d", status);
        return;
    }
    LOG_HEXDUMP_INF(hash, sizeof(hash), "Hash");

    // Allocate memory to send hash
    struct uart_data_t* hash_response = k_malloc(sizeof(*hash_response));
    if (hash_response) {
        // Convert sensor reading to string and store in buf->data
        memcpy(hash_response->data, hash, sizeof(hash));
        hash_response->len = sizeof(hash);

        // Send to BLE via FIFO
        k_fifo_put(&fifo_tx_data, hash_response);
    }
    else {
        LOG_ERR("Failed to allocate memory for buf");
        reset_state();
        return;
    }

    k_free(response);
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
            hash_response(response);
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
        else {
            LOG_ERR("Timeout happened");
        }

        reset_state();
        //  Sleep for 500 ms
        k_msleep(500);
    }
}

K_THREAD_DEFINE(button_handler_thread, SERVER_THREAD_STACKSIZE, button_handler, NULL, NULL, NULL,
                PRIORITY_SERVER, 0, 0);
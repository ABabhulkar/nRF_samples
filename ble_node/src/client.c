#include "client.h"

#include "common.h"
#include "zephyr/kernel.h"

LOG_MODULE_DECLARE(LOG_MODULE_NAME);
static K_SEM_DEFINE(start_command, 0, 1);
K_FIFO_DEFINE(fifo_client_response);

bool validate(struct uart_data_t* data)
{
    // TODO: validate the date and generate ack
    return true;
}

void start_generator()
{
    k_sem_give(&start_command);
}

// Runnable for sensor thread which take care of sensor reading
void data_generator(void* arg0, void* arg1, void* agr2)
{
    uint8_t sensor_reading[5];
    bool is_valid = false;

    while (1) {
        k_sem_take(&start_command, K_FOREVER);
        request_state(CLIENT);

        sys_rand_get(sensor_reading, 5);
        psa_status_t status = psa_generate_random(sensor_reading, sizeof(sensor_reading));
        if (status != PSA_SUCCESS) {
            LOG_INF("psa_generate_random failed! (Error: %d)", status);
            reset_state();
            continue;
        }

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

K_THREAD_DEFINE(data_generator_thread, GENERATOR_THREAD_STACKSIZE, data_generator, NULL, NULL, NULL,
                PRIORITY_CLIENT, 0, 0);
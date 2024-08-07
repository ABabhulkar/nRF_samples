#ifndef COMMON_THREAD_H_
#define COMMON_THREAD_H_

#include "zephyr/toolchain.h"

#include <psa/crypto.h>
#include <psa/crypto_extra.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>


#define LOG_MODULE_NAME ble_node

#define RESPONSE_TIMEOUT K_MSEC(5000)
#define START_COMMAND "start"

EXTERN_C struct k_fifo fifo_tx_data;
struct uart_data_t {
    void* fifo_reserved;
    uint8_t data[CONFIG_BT_NUS_UART_BUFFER_SIZE];
    uint16_t len;
};

enum state_e { SERVER, CLIENT, IDLE };

void reset_state();
void request_state(enum state_e);
enum state_e get_state();

#endif // COMMON_THREAD_H_
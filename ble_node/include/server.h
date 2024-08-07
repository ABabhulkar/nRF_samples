#ifndef SERVER_THREAD_H_
#define SERVER_THREAD_H_

#include "server.h"
#include "zephyr/toolchain.h"

#include <zephyr/kernel.h>


#define SW0_NODE DK_BTN1_MSK

#define SERVER_THREAD_STACKSIZE 1024
#define PRIORITY_SERVER 8
#define SHA256_SIZE (32)

EXTERN_C struct k_fifo fifo_server_response;

void button_pressed(uint32_t button_state, uint32_t has_changed);

#endif // SERVER_THREAD_H_
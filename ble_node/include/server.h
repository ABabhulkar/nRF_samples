#ifndef SERVER_THREAD_H_
#define SERVER_THREAD_H_

#include <zephyr/kernel.h>

#define SW0_NODE DK_BTN1_MSK

#define SERVER_THREAD_STACKSIZE 256
#define PRIORITY_SERVER 8
static K_SEM_DEFINE(button_state_pressed, 0, 1);
static K_FIFO_DEFINE(fifo_server_response);

void button_pressed(uint32_t button_state, uint32_t has_changed);

#endif // SERVER_THREAD_H_
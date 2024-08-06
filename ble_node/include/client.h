#ifndef CLIENT_THREAD_H_
#define CLIENT_THREAD_H_

#include <soc.h>
#include <stdbool.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/types.h>

#define PRIORITY_CLIENT 8
#define GENERATOR_THREAD_STACKSIZE 256

#define ACK_OK "ok"
#define ACK_N_OK "n_ok"

static K_FIFO_DEFINE(fifo_client_response);
static K_SEM_DEFINE(start_command, 0, 1);

void data_generator(void* arg0, void* arg1, void* agr2);

#endif // CLIENT_THREAD_H_
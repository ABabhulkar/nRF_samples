#ifndef CLIENT_THREAD_H_
#define CLIENT_THREAD_H_

#include "zephyr/toolchain.h"

#include <soc.h>
#include <stdbool.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/types.h>


#define PRIORITY_CLIENT 8
#define GENERATOR_THREAD_STACKSIZE 1024

#define ACK_OK "ok"
#define ACK_N_OK "n_ok"

EXTERN_C struct k_fifo fifo_client_response;

void data_generator(void* arg0, void* arg1, void* agr2);
void start_generator();

#endif // CLIENT_THREAD_H_
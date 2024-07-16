The message queue is a kernel object. A message queue is a thread-safe data container that holds a fixed amount of data (messages), which can be accessed safely by multiple threads at the same time. The queue can store different types of data.
The kernel takes care of all the necessary operations and safeguards for adding new data to the queue and removing data from it.

In addition, the queue also supports timeouts. You can use a timeout to:
- Make a thread that puts data to the queue go to sleep if the queue is full.
- Make a thread that gets data from the queue go to sleep if the queue is empty.
![[Pasted image 20240715161157.png]]The data size (message size) must be a multiple of the data alignment. If you have an odd data size, you can either pad the data or use a compiler attribute (such as `__aligned(4)`) to specify minimum alignment.

#### How to use
1. **Decide on what data you want to put in a message.**
   - The data type of a message is set statically, so it can’t change it dynamically later.
   - The message can be defined as a simple integer, a string, a stuct, or a union within a struct. By using a union within a struct, we can save memory, especially when those data types are not required simultaneously.
   ```c
struct MyStruct {
  int dataType; // Indicates the active data type
  union {
    int intValue;
    float floatValue;
    char stringValue[24];
  } data;
};
```

2. **Define the message queue and initialize it.**
   Using  [K_MSGQ_DEFINE()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/data_passing/message_queues.html#c.K_MSGQ_DEFINE) massage queue can be defined. The below simple snippet shows how to define and initialize a message queue `(device_message_queue`) with 16 messages. Each element is 4 bytes in size (`uint32_t`). For the alignment, the snippet specifies `sizeof(uint32_t)`, but you could simply pass 4.
   ```c
K_MSGQ_DEFINE(device_message_queue, sizeof(uint32_t), 16, 4);
```

3. **Write a message to the message queue.**
   Write the message using the [k_msgq_put()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/data_passing/message_queues.html#c.k_msgq_put) function, which expects three parameters:
   1. the message queue defined in step 2
   2. a pointer to a message of the type defined in step 1
   3. timeout: `K_FOREVER`,`K_MSEC()`,`K_NO_WAIT`

4. **Read a message from the message queue.**
   To read a message, use the [k_msgq_get()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/data_passing/message_queues.html#c.k_msgq_get) function. Note that calling this function will remove the message from the message queue (known as popping the message).
   The alternative is use the [k_msgq_peek()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/data_passing/message_queues.html#c.k_msgq_peek) function, which reads a message without removing it from the message queue.

> [!Important]
> Use a message queue to transfer data items of known size and number between threads in an asynchronous manner. 
> It can also be used with interrupts, but should be done with care as it may block ISR.

#### Example
```c
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

// 2200 msec = 2.2 sec
#define PRODUCER_SLEEP_TIME_MS 2200

LOG_MODULE_REGISTER(Less1_Exer1, LOG_LEVEL_DBG);

// Stack size for both the producer and consumer threads
#define STACKSIZE 2048
#define PRODUCER_THREAD_PRIORITY 6
#define CONSUMER_THREAD_PRIORITY 7
  
// Define the data type of the message
typedef struct {
    uint32_t x_reading;
    uint32_t y_reading;
    uint32_t z_reading;
} SensorReading;

// Define the message queue
K_MSGQ_DEFINE(device_message_queue, sizeof(SensorReading), 16, 4);

int main(void)
{
    int ret;

    if (!gpio_is_ready_dt(&led0)) {
        return 0;
    }

    ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 0;
    }
    
    ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 0;
    }

    return 0;
}

static void producer_func(void* unused1, void* unused2, void* unused3)
{
    ARG_UNUSED(unused1);
    ARG_UNUSED(unused2);
    ARG_UNUSED(unused3);

    while (1) {
        static SensorReading acc_val = {100, 100, 100};
        int ret;
        // Write messages to the message queue
        ret = k_msgq_put(&device_message_queue, &acc_val, K_FOREVER);

        if (ret) {
            LOG_ERR("Return value from k_msgq_put = %d", ret);
        }

        acc_val.x_reading += 1;
        acc_val.y_reading += 1;
        acc_val.z_reading += 1;
        k_msleep(PRODUCER_SLEEP_TIME_MS);
    }
}

static void consumer_func(void* unused1, void* unused2, void* unused3)
{
    ARG_UNUSED(unused1);
    ARG_UNUSED(unused2);
    ARG_UNUSED(unused3);

    while (1) {
        SensorReading temp;
        int ret;
        // Read messages from the message queue
        ret = k_msgq_get(&device_message_queue, &temp, K_FOREVER);

        if (ret) {
            LOG_ERR("Return value from k_msgq_get = %d", ret);
        }

        LOG_INF("Values got from the queue: %d.%d.%d\r\n", temp.x_reading, temp.y_reading, temp.z_reading);
    }
}

K_THREAD_DEFINE(producer, STACKSIZE, producer_func, NULL, NULL, NULL, PRODUCER_THREAD_PRIORITY, 0,0);

K_THREAD_DEFINE(consumer, STACKSIZE, consumer_func, NULL, NULL, NULL, CONSUMER_THREAD_PRIORITY, 0, 0);
```

Debug view for the example
![[Pasted image 20240716105953.png]]
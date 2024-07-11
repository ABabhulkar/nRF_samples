
There are two ways to create a thread in Zephyr:
- dynamically (at run-time) through [`k_thread_create()`](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.k_thread_create)
- statically (at compile time) by using the [K_THREAD_DEFINE()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.K_THREAD_DEFINE) macro

Source is [[_Index#^57fbd5]]
> [!Info]
> 
> K_THREAD_DEFINE(thread_id, stack_size, thread_functions, arg1, arg2, arg3, priority, options, delay)

- **thread_id:** name of the thread
- **stack_size:** size of the stack
- **thread_function:** function to be executed by the thread
- **arg1, arg2, arg3**: arguments to be passed to the thread function
- **priority:** priority of the thread
- **options:** [thread options](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#thread-options) can be found here
- **delay:** delay before the thread starts

> [!Important]
> Stack sizes should always be a power of two (512, 1024, 2048, etc.).
> Stack size should be chosen more carefully, to avoid unnecessarily using memory. We do not need that here for a simple application.

**Thread switching**
**`k_yield()`** will change the thread state from “Running” to “Runnable”, which means that at the next rescheduling point, the thread that just yielded is still a candidate in the [[Scheduler]] algorithm for making a thread active (“Running”). The overall result is that after the thread yields, there will be at least one item in the runnable thread queue for the scheduler to choose from at the next rescheduling point.
>[!Note]
> To give lower priority threads a chance to run, the current thread needs to be put to “Non-runnable”. This can be done using `k_sleep()`, which we will see further on in this exercise.

**`k_sleep()`** will change the thread state from “Running” to “Non-runnable” until the timeout has passed, and then change it to “Runnable”. This means that the thread will not be candidate in the scheduler’s algorithm until the timeout amount of time has passed. Hence thread sleeping is better choice for **adding delays** and not for yielding.

[[Time sequences for scheduling.canvas|Time sequences for scheduling]] shows comparative time sequences.

> [!More on this]
> [`k_sleep()`](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.k_sleep) expect a parameter of type `k_timeout_t` which can be constructed using macros like [`K_MSEC()`](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/timing/clocks.html#c.K_MSEC). For simplicity, `k_sleep()` has simpler-to-use derivatives like [k_msleep()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.k_msleep) and [`k_usleep()`](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.k_usleep) that you can pass the time unit directly. The latter will be used extensively.

##### Thread synchronization
Two mechanisms you can utilize to achieve thread synchronization are [[Semaphores]] or [[Mutexes]]. They have some differing properties, but in essence they are both variables that are changed before and after the critical section by a thread to make sure that no other threads can execute the segment before that thread has completed it.
The main differences are that semaphores have a maximum value that is set at initialization, while mutexes have ownership property, i.e only the thread incrementing its value can decrement it, until zero when it is relinquished.

##### Examples
1. Static and dynamic thread creation
```c
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

// Thread stack size for simplicity it is same for all.
#define STACKSIZE 1024

// Thread priority
#define THREAD0_PRIORITY 6
#define THREAD1_PRIORITY 7
#define THREAD2_PRIORITY 7

// Runnable for thread 0: This thread will never stop
void thread0(void)
{
    while (1) {
        printk("Hello, I am thread0\n");
        k_msleep(1000);
    }
}

// Runnable for thread 1: This thread will never stop
void thread1(void)
{
    while (1) {
        printk("Hello, I am thread1\n");
        k_msleep(1000);
    }
}
  
// Runnable for thread 2: This thread will end once task is done
void thread2(void* arg0, void* arg1, void* agr2)
{
    for (int32_t i = 0; i < 10; i++) {
        printk("Hello, I am thread2 : %d\n", i);
        k_busy_wait(1000000);
    }
}

// Defining stack for dynamic thread
K_THREAD_STACK_DEFINE(my_stack_area, STACKSIZE);
  
int main(void)
{
    while (1) {
        // Dynamic thread creation
        struct k_thread my_thread_data;
        k_tid_t my_tid =
            k_thread_create(&my_thread_data, my_stack_area, K_THREAD_STACK_SIZEOF(my_stack_area),
                            thread2, NULL, NULL, NULL, THREAD2_PRIORITY, 0, K_NO_WAIT);
 
        //----- This is wait and abort thread ------//
        k_msleep(5000);
        k_thread_abort(my_tid);
        //------------------------------------------//

        //- Wait till thread execution is complete -//
        // k_thread_join(my_tid, K_NO_WAIT);         // Comment this line or k_thread_abort only one
        // can work at a time
        //------------------------------------------//

        k_msleep(10000);
    }
}

// This thread will start immediately after creation
K_THREAD_DEFINE(thread0_id, STACKSIZE, thread0, NULL, NULL, NULL, THREAD0_PRIORITY, 0, 0);
// This thread will start after 5s delay
K_THREAD_DEFINE(thread1_id, STACKSIZE, thread1, NULL, NULL, NULL, THREAD1_PRIORITY, 0, 5);
```


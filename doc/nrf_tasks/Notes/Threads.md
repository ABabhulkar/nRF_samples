
A thread is the basic unit of runnable code. 
- user-defined thread
	- dynamically (at run-time) through [`k_thread_create()`](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.k_thread_create)
	- statically (at compile time) by using the [K_THREAD_DEFINE()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.K_THREAD_DEFINE) macro
- thread created by the RTOS
	- System thread
	- Idle thread
	- workqueue thread
	- Thread created by a RTOS subsystem

A thread has the following items:
- **Thread control block**: For each thread, there will be an instance of a thread control block within the RTOS that keeps track of a thread’s information, specifically its metadata.
- **Stack**: Each thread will have its own stack. The stack area’s size must be set to align with the specific processing requirements of the thread.
- **Entry point function**: This is the body of the thread or, in other words runnable. It usually contains an infinite loop, as exiting the entry point will terminate the thread. The entry point function can have three optional argument values that can be passed to it on start.
- **Thread priority**: It instructs the [[Scheduler]] how to allocate CPU time to the thread.
- **Optional [thread options](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#thread-options)** :  By using this optional field, you can make the thread receive special treatment under specific circumstances.
- **Optional starting delay**: By passing K_NO_WAIT which is simply a start delay of 0. Or we can specify an optional start delay.

> [!Important]
> Dynamic threads are not supported in Zephyr RTOS as of V3.4.0
> if you use the [`k_thread_create()`](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.k_thread_create) function, you must allocate a stack using the [`K_THREAD_STACK_DEFINE()`](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.K_THREAD_STACK_DEFINE) macro in advance.


> [!Note]
> There is also the option to create a thread with the delay set to K_FOREVER, which effectively makes the thread inactive. To activate, call [`k_thread_start()`](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.k_thread_start), which will add the thread to the queue of ready threads (ready queue).
#### Thread life cycle
![[Pasted image 20240715122421.png]]

#### Statically defined thread
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


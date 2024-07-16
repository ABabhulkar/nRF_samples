- [[#Introduction|Introduction]]
- [[#System Threads|System Threads]]
- [[#User defined threads|User defined threads]]
- [[#Thread synchronization|Thread synchronization]]
- [[#Examples|Examples]]
### Introduction
A  [Threads](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/threads/index.html) is the basic unit of runnable code. 
- [[#User defined threads]]
	- dynamically (at run-time) through [`k_thread_create()`](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.k_thread_create)
	- [[Statically defined thread]] (at compile time) by using the [K_THREAD_DEFINE()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.K_THREAD_DEFINE) macro
- thread created by the RTOS
	- [[#System Threads]]
	- Idle thread
	- [[Workqueue threads]]
	- Thread created by a RTOS subsystem like logger, BT

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

In nRF Connect SDK, there are two main types of threads: 
- [[cooperative threads]] (negative priority value) : limited usage
- [[preemptible threads]] (non-negative priority

A thread can be in one of the following states at any given time.
- **Running:** The running thread is the one that is currently being executed by the CPU. 
- **Runnable:** A thread is marked as “Runnable” when it has no other dependencies with other threads or other system resources to proceed further in execution. The only resource this thread is waiting for is the CPU time. This is also known as *“Ready”* state.
- **Non-runnable:** A thread that has one or more factors that prevent its execution is deemed to be unready, and cannot be selected as the current thread. The scheduler does not include these threads in the scheduling algorithm to select the next thread to run. This is also known as *“Unready”* state.

> [!Note]
> There is also the option to create a thread with the delay set to K_FOREVER, which effectively makes the thread inactive. To activate, call [`k_thread_start()`](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.k_thread_start), which will add the thread to the queue of ready threads (ready queue).
### System Threads
A system thread is a type of thread that is spawned automatically by Zephyr RTOS during initialization.
1. **Main Thread:** 
   Executes the necessary RTOS initializations and calls the application’s `main()` function, if it exists. If no user-defined main() is supplied, the main thread will exit normally, though the system would be fully functional.
   This is entry point in the system.
2. **Idle Thread:** It runs when there is no other work to do, either running an empty loop or, if supported, will activate power management to save power **(This is the case for Nordic devices).**
> [!Info]
> 
> Having the `main()` function is optional in Zephyr RTOS-based applications. This is because the main thread automatically spawned by the RTOS will do the necessary RTOS initialization, including scheduler/kernel setup, and core drivers setup.
> 
> After that, it will try to call the user-defined `main()`, if one exists. If no `main()` function exists, the main thread will exit. The system will still be functional and other threads can be scheduled normally.

### User defined threads
In addition to system threads, a user can define their own threads to assign tasks to. For example, a user can create a thread to delegate reading sensor data, another thread to process data, and so on. Threads are assigned a priority, which instructs the scheduler how to allocate CPU time to the thread. 
nRF connect SDK provides macro to create [[Threads]].

### Thread synchronization
Two mechanisms you can utilize to achieve thread synchronization are [[Semaphores]] or [[Mutexes]]. They have some differing properties, but in essence they are both variables that are changed before and after the critical section by a thread to make sure that no other threads can execute the segment before that thread has completed it.
The main differences are that semaphores have a maximum value that is set at initialization, while mutexes have ownership property, i.e only the thread incrementing its value can decrement it, until zero when it is relinquished.

### Examples
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


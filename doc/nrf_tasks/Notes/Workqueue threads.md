##### Workqueue threads
In nRF Connect SDK work item is also a common unit of execution which is nothing more than a user-defined function that gets called by a dedicated thread called a [workqueue thread](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/workqueue.html).
The main use of this is to offload non-urgent work from an ISR or a high-priority thread to a lower priority thread. A system can have multiple workqueue threads, the default one is known as the system workqueue, available to any application or kernel code.

![[Pasted image 20240708162724.png]]
As you can see in the image above, the ISR or high priority thread submits work into a workqueue, and the dedicated workqueue thread pulls out a work item in a first in, first out (FIFO) order. The thread that pulls work items from the queue always yields after it has processed one work item, so that other equal priority threads are not blocked for a long time.

> [!Note]
> The advantage of delegating work as a work item instead of a dedicated thread is that since work items are all sharing one stack, the workqueue stack, a work item is lighter than a thread because no stack is allocated.


#### Example

```c
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

// Define stack size used by each thread
#define THREAD0_STACKSIZE 512
#define THREAD1_STACKSIZE 512

#define THREAD0_PRIORITY 2     //Highest priority task
#define THREAD1_PRIORITY 3     //Low priority task

//Function to emulate non-urgent work
static inline void emulate_work() 
{
	for(volatile int count_out = 0; count_out < 150000; count_out ++);
}

// Runnable of high priority thread
void thread0(void)
{
    uint64_t time_stamp;         //Init time
    int64_t delta_time;          //Init delta
    
    while (1) {
        time_stamp = k_uptime_get();                //Start time of work
        
        emulate_work();                             //non-urgent work
        delta_time = k_uptime_delta(&time_stamp);   //End time of work
        printk("thread0 yielding this round in %lld ms\n", delta_time);
        k_msleep(20);
   }
}

// Runnable of low priority thread
void thread1(void)
{
    uint64_t time_stamp;         //Init time
    int64_t delta_time;          //Init delta
    
    while (1) {
        time_stamp = k_uptime_get();                //Start time of work
        
        emulate_work();                             //non-urgent work
        delta_time = k_uptime_delta(&time_stamp);   //End time of work
        printk("thread0 yielding this round in %lld ms\n", delta_time);
        k_msleep(20);
   }
}
/* STEP 4 - Define entry function for thread1 */
K_THREAD_DEFINE(thread0_id, THREAD0_STACKSIZE, thread0, NULL, NULL, NULL, THREAD0_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread1_id, THREAD1_STACKSIZE, thread1, NULL, NULL, NULL, THREAD1_PRIORITY, 0, 0);
```
You can see that the higher priority `thread0` completes the task `emulate_work` in about 25-26 ms but `thread1` takes more than double that time. This is because `thread0` keeps blocking `thread1`.

The timeline of threads should look something like below: ![[Pasted image 20240709110830.png]]

We need to associate our work (`emulate_work()`) as a work item and push it to a specific workqueue. This is done by creating a `work_info` structure and a function, `offload_function()` that should only run `emulate_work()`.

```c
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

// Define stack size used by each thread
#define THREAD0_STACKSIZE 512
#define THREAD1_STACKSIZE 512

#define THREAD0_PRIORITY 2 // Highest priority task
#define THREAD1_PRIORITY 3 // Low priority task

// STEP 1 - Define constants for workqueue
#define WORQ_THREAD_STACK_SIZE 512
#define WORKQ_PRIORITY 4 // Lowest priority workqueue

// STEP 2 - Define stack area used by workqueue thread
static K_THREAD_STACK_DEFINE(my_stack_area, WORQ_THREAD_STACK_SIZE);

// STEP 3 - Define queue structure
static struct k_work_q offload_work_q = {0};

// Function to emulate non-urgent work
static inline void emulate_work()
{
    for (volatile int count_out = 0; count_out < 150000; count_out++);
}

// STEP 4 - Create work_info structure and offload function
struct work_info {
    struct k_work work;
    char name[25];
} my_work;

void offload_function(struct k_work* work_tem)
{
    emulate_work();
}  

// Runnable of high priority thread
void thread0(void)
{
    uint64_t time_stamp; // Init time
    int64_t delta_time;  // Init delta

    // STEP 5 - Start the workqueue
    k_work_queue_start(&offload_work_q, my_stack_area, K_THREAD_STACK_SIZEOF(my_stack_area), WORKQ_PRIORITY, NULL);

    // STEP 5 - Initialize work item and connect it to its handler function
    strcpy(my_work.name, "Thread0 emulate_work()");
    k_work_init(&my_work.work, offload_function);
    
    while (1) {
        time_stamp = k_uptime_get(); // Start time of work

        // STEP 6 - Submit the work item to the workqueue
        k_work_submit_to_queue(&offload_work_q, &my_work.work);
        
        delta_time = k_uptime_delta(&time_stamp); // End time of work
        printk("thread0 yielding this round in %lld ms\n", delta_time);
        k_msleep(20);
    }
}

// Runnable of low priority thread
void thread1(void)
{
    uint64_t time_stamp; // Init time
    int64_t delta_time;  // Init delta

    while (1) {
        time_stamp = k_uptime_get();              // Start time of work
        emulate_work();                           // non-urgent work
        delta_time = k_uptime_delta(&time_stamp); // End time of work
        printk("thread0 yielding this round in %lld ms\n", delta_time);
        k_msleep(20);
    }
}

/* STEP 4 - Define entry function for thread1 */
K_THREAD_DEFINE(thread0_id, THREAD0_STACKSIZE, thread0, NULL, NULL, NULL, THREAD0_PRIORITY, 0, 0);

K_THREAD_DEFINE(thread1_id, THREAD1_STACKSIZE, thread1, NULL, NULL, NULL, THREAD1_PRIORITY, 0, 0);
```

Now the timing looks something like below:![[Pasted image 20240709113118.png]]
> [!Example]
> This is an example of good architecture as we only keep urgent work to be processed in higher priorities and non-urgent work is offloaded to the appropriate lower priority. As an application designer on the RTOS, you should be aware of the kernel services provided to the application and make best use of it so as to avoid unnecessary latencies
https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/synchronization/mutexes.html
Mutexes have the following properties:

- [Locking](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/synchronization/mutexes.html#locking-a-mutex) a mutex will increment the lock count. Recursive locking (reentrant locking) will not make the locking thread block since it already owns the mutex. The thread should make sure that it unlocks the mutex the same number of times that it locked it to release the mutex so that other threads can attempt to own it.
- [Unlocking](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/synchronization/mutexes.html#unlocking-a-mutex) a mutex will decrement the lock count. When the lock count is zero, that means that the mutex is in an unlocked state. Threads can attempt to own the mutex only when the mutex is in an unlocked state.
- Only the thread that locked the mutex can unlock it.
- Mutexes locking and unlocking can only be done in threads and not in ISRs. This is because ISRs cannot participate in the ownership and priority inheritance mechanism of the scheduler.
- The thread locking the mutex is eligible for [priority inheritance](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/synchronization/mutexes.html#priority-inheritance) since only that thread can unlock the mutex.
![[Pasted image 20240709130427.png]]
> [!Note]
> 
> ```c-like
> CONFIG_MULTITHREADING=y
> ```
> 
> _**This configuration defaults to yes in all nRF Connect SDK applications and isn’t strictly necessary to enable manually.**_

##### Example
```c
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/random/random.h>
#include <string.h>

#define THREAD0_STACKSIZE       512
#define THREAD1_STACKSIZE       512

// Set the priority of the two threads to have equal priority
#define THREAD0_PRIORITY        4 
#define THREAD1_PRIORITY        4

// Define the two counters with a constant combined total
#define COMBINED_TOTAL   40
int32_t increment_count = 0; 
int32_t decrement_count = COMBINED_TOTAL; 

// STEP 1 - Define mutex to protect access to shared code section
K_MUTEX_DEFINE(test_mutex);

// Shared code run by both threads
void shared_code_section(void)
{
	// STEP 2 - Lock the mutex
    k_mutex_lock(&test_mutex, K_FOREVER);

	// Increment count and decrement count changed
	// according to logic defined
	increment_count += 1;
	increment_count = increment_count % COMBINED_TOTAL; 
	decrement_count -= 1;
	if (decrement_count == 0) 
	{
		decrement_count = COMBINED_TOTAL;
	}
	
	// STEP 3 - Unlock the mutex
    k_mutex_unlock(&test_mutex);
    
	// Print counter values if they do not add up to COMBINED_TOTAL
	if(increment_count + decrement_count != COMBINED_TOTAL )
	{
		printk("Race condition happend!\n");
		printk("Increment_count (%d) + Decrement_count (%d) = %d \n",
	                increment_count, decrement_count, (increment_count + decrement_count));
		k_msleep(400 + sys_rand32_get() % 10);
	}
}

// Functions for thread0 and thread1 with a shared code section
void thread0(void)
{
	printk("Thread 0 started\n");
	while (1) {
		shared_code_section(); 
	}
}

void thread1(void)
{
	printk("Thread 1 started\n");
	while (1) {
		shared_code_section(); 
	}
}

// Define and initialize threads
K_THREAD_DEFINE(thread0_id, THREAD0_STACKSIZE, thread0, NULL, NULL, NULL,
		THREAD0_PRIORITY, 0, 5000);

K_THREAD_DEFINE(thread1_id, THREAD1_STACKSIZE, thread1, NULL, NULL, NULL,
		THREAD1_PRIORITY, 0, 5000);

```
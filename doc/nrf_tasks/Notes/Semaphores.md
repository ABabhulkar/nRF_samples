https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/synchronization/semaphores.html
Semaphores have the following properties:
- At initialization, you set an initial count (greater than 0) and a maximum limit.
- “Give” will increment the semaphore count unless the count is already at the maximum limit, in which case the signal will not increment. “Give” can be done from any thread or ISR.
- “Take” will decrement the semaphore count unless the semaphore is unavailable (count at zero). Any thread that is trying to take a semaphore that is unavailable needs to wait until some other thread makes it available (by giving the semaphore). “Take” can be done only in threads and not in ISR (since ISRs should not block on anything).
- There is no ownership of semaphores.
- The thread taking the semaphore is NOT eligible for priority inheritance since the taking thread does not own the semaphore and any other thread can give the semaphore.
![[Pasted image 20240709125939.png]]

##### Example
```c
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/random/random.h>
#include <string.h>

#define PRODUCER_STACKSIZE       512
#define CONSUMER_STACKSIZE       512

// Set the priority of the producer and consumer thread
#define PRODUCER_PRIORITY        5 
#define CONSUMER_PRIORITY        4        //Set higher to be able to demonstrate its ability to request access

// Define semaphore to monitor instances of available resource
K_SEM_DEFINE(instance_monitor_sem, 10, 10);

// Initialize the available instances of this resource
volatile uint32_t available_instance_count = 10;

// Function for getting access of resource
void get_access(void)
{
	// Get semaphore before access to the resource
	k_sem_take(&instance_monitor_sem, K_FOREVER);

	// Decrement available resource
    available_instance_count--;
    printk("Resource taken and available_instance_count = %d\n", available_instance_count);
}

// Function for releasing access of resource
void release_access(void)
{
	// Increment available resource
    available_instance_count++;
    printk("Resource given and available_instance_count = %d\n", available_instance_count);
	
	// Give semaphore after finishing access to resource
	k_sem_give(&instance_monitor_sem); 
}

//Producer thread relinquishing access to instance
void producer(void)
{
	printk("Producer thread started\n");
	while (1) {
		release_access();
		// Assume the resource instance access is released at this point
		k_msleep(500 + sys_rand32_get() % 10);
	}
}

// Consumer thread obtaining access to instance
void consumer(void)
{
    printk("Consumer thread started\n");
	while (1) {
		get_access();
		// Assume the resource instance access is released at this point
		k_msleep(sys_rand32_get() % 10);
	}
}

// Define and initialize threads
K_THREAD_DEFINE(producer_id, PRODUCER_STACKSIZE, producer, NULL, NULL, NULL, PRODUCER_PRIORITY, 0, 0);

K_THREAD_DEFINE(consumer_id, CONSUMER_STACKSIZE, consumer, NULL, NULL, NULL, CONSUMER_PRIORITY, 0, 0);
```
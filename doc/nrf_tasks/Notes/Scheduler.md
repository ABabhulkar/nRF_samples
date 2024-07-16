The [scheduler](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/scheduling/index.html) is the part of the RTOS responsible for scheduling which tasks are running, i.e using CPU time, at any given time. It does this using a [scheduling algorithm](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/scheduling/index.html#scheduling-algorithm) to determine which task should be the next to run.

> [!Note]
> 
> The number of running threads possible is equal to the number of application cores. For example on the nRF52840, there is one application core, allowing for one running thread at a time.

Zephyr RTOS is by default a tickless RTOS. A tickless RTOS is completely event-driven, which means that instead of having periodic timer interrupts to wake up the scheduler, it is woken based on events known as rescheduling points.

#### Context switch
^da7450
The combined resources, including processor registers and the stack, form the thread’s context. This process of saving the context when preempting a thread and restoring it when resuming is known as context switching.
![[Pasted image 20240715125645.png]]
Notice that context switching does consume a bit of time as it involves copying data. Also, keep in mind that context switching happens with interrupts as well.

#### Rescheduling point
A rescheduling point is an instant in time when the scheduler gets called to select the thread to run next. Some examples of rescheduling points are:
- When a thread calls `k_yield()`, the thread’s state is changed from “Running” to “Ready”.
- Unblocking a thread by giving/sending a kernel synchronization object like a semaphore, mutex or alert, causes the thread’s state to be changed from “Unready” to “Ready”.
- When a receiving thread gets new data from other threads using [data passing](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/index.html#data-passing) kernel objects, the data receiving thread’s state is changed from “Waiting” to “Ready”.
- When [time slicing](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/kconfig/index.html#CONFIG_TIMESLICING) is enabled (covered in Exercise 2) and the thread has run continuously for the maximum time slice time allowed, the thread’s state is changed from “Running” to “Ready”.

#### Threads with equal priority
##### Default behavior
The scheduler will run the thread that was the first to have been made Ready in the ready queue.
##### Time Slicing
If you don’t want to worry about creating perfect logic for yielding between equal priority threads, you can enable _time slicing_.
Enable time slicing in our project, by adding the following lines to the `prj.conf` file.
```c-like
CONFIG_TIMESLICING=y
CONFIG_TIMESLICE_SIZE=10
CONFIG_TIMESLICE_PRIORITY=0
```
- `TIMESLICING` enables the time slicing feature.
- `TIMESLICE_SIZE` is the maximum time (in ms) that the current running thread has before it is forcefully preempted by the scheduler to allow the other equal priority threads to run.
- `TIMESLICE_PRIORITY` is the priority threshold for time slicing, meaning threads with higher priority than this threshold are not subject to time slicing. We set this value to `0`. This means priorities (0 to 15) will be affected by time-slicing only when two or more threads exist in one level. It’s always important to remember that time slicing only affect threads with the same priority level.
- [[Time sequences for scheduling.canvas|Time sequences for scheduling]] can show comparison.

##### Earliest deadline first
The firmware developer must provide an estimated deadline for each thread by calling [`k_thread_deadline_set()`](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.k_thread_deadline_set). When multiple threads exist with the same priority level, the scheduler will pick the thread with the earliest deadline (shortest period).
The Kconfig symbol to enable this option is [CONFIG_SCHED_DEADLINE](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/kconfig/index.html#CONFIG_SCHED_DEADLINE).

##### Scheduler locking and disabling interrupts
Scheduler locking is a mechanism in RTOS where the scheduler is temporarily locked or disabled to prevent [[Scheduler#^da7450|context switching]] between different threads or processes.
- For cooperative threads, it is done automatically. A cooperative thread has a scheduler-locking mechanism built into it.
- For preemptable threads, there are two functions related to scheduler locking: [k_sched_lock()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.k_sched_lock) to lock a thread, [k_sched_unlock()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.k_sched_unlock) to unlock a thread. The `k_sched_lock()` function effectively elevates the current thread to a cooperative priority, even when there are no cooperative priorities configured.
 
> [!Important]
> Scheduler locking does not prevent interrupts from interrupting your critical region. To protect a critical section of code from being preempted by the scheduler **and** from being interrupted by an ISR, you can use the [irq_lock()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/interrupts.html#c.irq_lock) and [irq_unlock()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/interrupts.html#c.irq_unlock) functions.


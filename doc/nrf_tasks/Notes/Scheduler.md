This is where the [scheduler](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/scheduling/index.html) comes in. The scheduler is the part of the RTOS responsible for scheduling which tasks are running, i.e using CPU time, at any given time. It does this using a [scheduling algorithm](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/scheduling/index.html#scheduling-algorithm) to determine which task should be the next to run.

> [!Note]
> 
> The number of running threads possible is equal to the number of application cores. For example on the nRF52840, there is one application core, allowing for one running thread at a time.

Zephyr RTOS is by default a tickless RTOS. A tickless RTOS is completely event-driven, which means that instead of having periodic timer interrupts to wake up the scheduler, it is woken based on events known as rescheduling points.

##### Rescheduling point
A rescheduling point is an instant in time when the scheduler gets called to select the thread to run next. Some examples of rescheduling points are:
- When a thread calls `k_yield()`, the thread’s state is changed from “Running” to “Ready”.
- Unblocking a thread by giving/sending a kernel synchronization object like a semaphore, mutex or alert, causes the thread’s state to be changed from “Unready” to “Ready”.
- When a receiving thread gets new data from other threads using [data passing](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/index.html#data-passing) kernel objects, the data receiving thread’s state is changed from “Waiting” to “Ready”.
- When [time slicing](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/kconfig/index.html#CONFIG_TIMESLICING) is enabled (covered in Exercise 2) and the thread has run continuously for the maximum time slice time allowed, the thread’s state is changed from “Running” to “Ready”.

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
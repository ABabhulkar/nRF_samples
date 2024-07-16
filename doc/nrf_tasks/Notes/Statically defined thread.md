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

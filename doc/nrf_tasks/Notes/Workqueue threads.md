##### Workqueue threads
In nRF Connect SDK work item is also a common unit of execution which is nothing more than a user-defined function that gets called by a dedicated thread called a [workqueue thread](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/workqueue.html).
The main use of this is to offload non-urgent work from an ISR or a high-priority thread to a lower priority thread. A system can have multiple workqueue threads, the default one is known as the system workqueue, available to any application or kernel code.
![[Pasted image 20240708162724.png]]
As you can see in the image above, the ISR or high priority thread submits work into a workqueue, and the dedicated workqueue thread pulls out a work item in a first in, first out (FIFO) order. The thread that pulls work items from the queue always yields after it has processed one work item, so that other equal priority threads are not blocked for a long time.

> [!Note]
> The advantage of delegating work as a work item instead of a dedicated thread is that since work items are all sharing one stack, the workqueue stack, a work item is lighter than a thread because no stack is allocated.

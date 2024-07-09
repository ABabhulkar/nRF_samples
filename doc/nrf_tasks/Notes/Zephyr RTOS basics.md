- [[#Introduction|Introduction]]
- [[#System Threads|System Threads]]
- [[#User defined threads|User defined threads]]
- [[#Workqueue threads|Workqueue threads]]
- [[#Threads Priority|Threads Priority]]
- [[Scheduler]]

##### Introduction
In nRF Connect SDK, there are two main types of threads: 
- [[cooperative threads]] (negative priority value) : limited usage
- [[preemptible threads]] (non-negative priority

A thread can be in one of the following states at any given time.
- **Running:** The running thread is the one that is currently being executed by the CPU. 
- **Runnable:** A thread is marked as “Runnable” when it has no other dependencies with other threads or other system resources to proceed further in execution. The only resource this thread is waiting for is the CPU time. This is also known as *“Ready”* state.
- **Non-runnable:** A thread that has one or more factors that prevent its execution is deemed to be unready, and cannot be selected as the current thread. The scheduler does not include these threads in the scheduling algorithm to select the next thread to run. This is also known as *“Unready”* state.

##### System Threads
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

##### User defined threads
In addition to system threads, a user can define their own threads to assign tasks to. For example, a user can create a thread to delegate reading sensor data, another thread to process data, and so on. Threads are assigned a priority, which instructs the scheduler how to allocate CPU time to the thread. 
nRF connect SDK provides macro to create [[User defined threads]].

##### Workqueue threads
In nRF Connect SDK work item is also a common unit of execution which is nothing more than a user-defined function that gets called by a dedicated thread called a [[Workqueue threads]].
The main use of this is to offload non-urgent work from an ISR or a high-priority thread to a lower priority thread.

##### Threads Priority
Threads are assigned an integer value to indicate their priority, which can be either negative or non-negative. **Lower numerical values take precedence over higher values,** meaning a thread with priority 4 will be given higher priority than a thread with priority 7.
**maximum value for a priority is 15**. The **main thread** has a priority of **0**, while the **idle thread** has a priority of **15** by default.
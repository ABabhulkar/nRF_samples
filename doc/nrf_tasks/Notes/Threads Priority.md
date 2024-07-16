
Threads are assigned an integer value to indicate their priority, which can be either negative or non-negative. **Lower numerical values take precedence over higher values,** meaning a thread with priority 4 will be given higher priority than a thread with priority 7.
**maximum value for a priority is 15**. 
- The **main thread** has a priority of **0**, 
- The **idle thread** has a priority of **15** by default. 
- If the logger module is used in deferred mode, the logger thread will have a priority of 14. 
- System work queue thread implemented as a cooperate thread with a priority of -1.

You can change the values of `CONFIG_NUM_PREEMPT_PRIORITIES` and `CONFIG_NUM_COOP_PRIORITIES` if you have a strong need for more or fewer levels. However, the default values set by the nRF Connect SDK are 15 and -16, respectively. 14. 
![[Pasted image 20240715141457.png]]

> [!Note]
>  You can change the values of `CONFIG_NUM_PREEMPT_PRIORITIES` and `CONFIG_NUM_COOP_PRIORITIES` if you have a strong need for more or fewer levels. However, the default values set by the nRF Connect SDK are 15 and -16, respectively.


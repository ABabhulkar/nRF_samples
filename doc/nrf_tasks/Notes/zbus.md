The _Zephyr bus - zbus_ is a lightweight and flexible software bus enabling a simple way for threads to talk to one another in a many-to-many way.
![[Pasted image 20240828103532.png]]
- Set of channels that consists of the control metadata information and message itself.
- [[#^3b9a99|Virtual Distributed Event Dispatcher (VDED)]], the bus logic responsible for sending notifications/messages to the observers. The VDED logic runs inside the publishing action in the same thread context, giving the bus an idea of a distributed execution. When a thread publishes to a channel, it also propagates the notifications to the observers;
- Threads (subscribers and message subscribers) and callbacks (listeners) publishing, reading, and receiving notifications from the bus.
- Publishing, reading, claiming, and finishing are available in all RTOS thread contexts, including ISRs.
- There are three types of observers:
	- **Listener**: A callback that the event dispatcher executes every time an observer channel is published or notified
	- **Message subscriber**: A thread-based observer that relies internally on a FIFO where the event dispatcher puts a copy of the message every time an observed channel is published or notified.
	- **Subscriber**: A thread-based observer that relies internally on a message queue where the event dispatcher puts a changed channel's reference every time an observed channel is published or notified.
	  Subscriber is responsible to read the message from the channel after receiving the notification.
> [!note]
> - Channel observation structure defines the relationship between a channel and its observers.
> - Static or runtime observations can be created. 
> - It is possible to disable the observer entirely or observation individually. The event dispatcher will ignore disabled observations.
> - Static allocation of observers can be done using [`ZBUS_CHAN_DEFINE`](https://docs.zephyrproject.org/latest/doxygen/html/group__zbus__apis.html#ga29a3a39e5c78a34b2d8491615d1f0687) or [`ZBUS_CHAN_ADD_OBS`](https://docs.zephyrproject.org/latest/doxygen/html/group__zbus__apis.html#gaf63215f3f53741edf52b4d0d7b2b97df)
 
> [!Important]
> - Zbus decouples the software modules from the HW interfaces. So they are reusable as long as interfaces(set of channels) are available. 
> - Messages can be dynamic or static allocated; notifications can be synchronous or asynchronous.
> - Custom metadata information can be added to a channel by using the user-data field.

> [!Important] **Message subscriber** 
> ZBus uses [include/zephyr/net/buf.h](https://github.com/zephyrproject-rtos/zephyr/blob/main/include/zephyr/net/buf.h) (network buffers) to exchange data with message subscribers. Thus, choose carefully the configurations `CONFIG_NET_BUF` [`CONFIG_ZBUS_MSG_SUBSCRIBER_NET_BUF_POOL_SIZE`](https://docs.zephyrproject.org/latest/kconfig.html#CONFIG_ZBUS_MSG_SUBSCRIBER_NET_BUF_POOL_SIZE "CONFIG_ZBUS_MSG_SUBSCRIBER_NET_BUF_POOL_SIZE") and [`CONFIG_HEAP_MEM_POOL_SIZE`](https://docs.zephyrproject.org/latest/kconfig.html#CONFIG_HEAP_MEM_POOL_SIZE "CONFIG_HEAP_MEM_POOL_SIZE").
#### Virtual Distributed Event Dispatcher
^3b9a99
VDED execution layer happens in the context of publisher. It can be a thread or an ISR. Scheduler won't preempt the VDED event in case of an ISR. Following steps are performed as a part of VDED.
1. The channel lock is acquired
2. The channel receives the new message via direct copy 
3. The event dispatcher logic executes the listeners, sends a copy of the message to the message subscribers, and pushes the channel's reference to the subscribers notification message queue in the same sequence they appear on the channel observers list. The listeners can perform non-copy quick assess to the constant message reference directly since the channel is still locked.
4. At last, the publishing function unlocks the channel.

**Example:**
Lets consider example were we have T1 as a publisher, S1 is subscriber, L1&L2 are the listeners, MS1 & MS2 are the message subscribers.
1. Publishing thread has the highest priority
![[Pasted image 20240828115329.png]]

2. The scenario considers the following priorities: T1 < MS1 < MS2 < S1.
![[Pasted image 20240828120041.png]]

3. HLP priority boost
   The VDED will elevate the publisher’s priority based on the HOP to ensure small latency and as few preemptions as possible.
![[Pasted image 20240828125815.png]]
> [!Note]
> The priority boost is enabled by default. To deactivate it, you must set the [`CONFIG_ZBUS_PRIORITY_BOOST`](https://docs.zephyrproject.org/latest/kconfig.html#CONFIG_ZBUS_PRIORITY_BOOST "CONFIG_ZBUS_PRIORITY_BOOST") configuration.

> [!Warning]
>  ZBus priority boost does not consider runtime observers on the HOP calculations.




Ref: 
- https://www.youtube.com/watch?v=EBnFqARqyeQ
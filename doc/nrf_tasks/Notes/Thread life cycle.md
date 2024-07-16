#### Thread life cycle
![[Pasted image 20240715122421.png]]

> [!Note]
> There is also the option to create a thread with the delay set to K_FOREVER, which effectively makes the thread inactive. To activate, call [`k_thread_start()`](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#c.k_thread_start), which will add the thread to the queue of ready threads (ready queue).

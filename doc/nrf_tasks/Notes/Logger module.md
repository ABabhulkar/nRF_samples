The logger module is the recommended method for sending messages to a console. The logger module supports both in-place and deferred logging among many other advanced features such as:
- Multiple backends
- Compile time filtering on module level
- Run time filtering independent for each backend
- Timestamping with user-provided function
- Dedicated API for dumping data
- Coloring of logs
- `printk()` support – printk message can be redirected to the logger
You can read the full list of features in the [Logging documentation](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/services/logging/index.html).

> [!More on this]
> When the logging module is used, it will create a low priority thread `(log_process_thread_func)` by default. The task of this thread is to take the deferred “queued” logs and push them out to a console.

The logger module is designed to be **thread-safe** and minimizes the time needed to log the message. **Time-consuming operations** like string formatting or obtaining access to the transport (i.e UART, RTT or whatever backend you are using) are **not performed immediately** when the logger API is called.

##### Severity levels
There are four _severity levels_ available in the system, see the table below.

| 1 (most severe)  | Error   | Severe error conditions                       | `LOG_LEVEL_ERR` |
| ---------------- | ------- | --------------------------------------------- | --------------- |
| 2                | Warning | Conditions that should be taken care of       | `LOG_LEVEL_WRN` |
| 3                | Info    | Informational messages that require no action | `LOG_LEVEL_INF` |
| 4 (least severe) | Debug   | Debugging messages                            | `LOG_LEVEL_DBG` |
|                  |         |                                               |                 |
> [!Example]
> As another example, the following lines:
> ```c
 > LOG_INF("Exercise %d",2);   
 > LOG_DBG("A log message in debug level");
 > LOG_WRN("A log message in warning level!");
> LOG_ERR("A log message in Error level!");
> ```
> We also have the `LOG_HEXDUMP_X`  macros for dumping data where `X` can be `DBG`, `INF`, `WRN`, or `ERR`.

There are two configuration categories for the logger module: configurations per module and global configuration. When logging is enabled globally, it works for all modules. However, modules can disable logging locally. Every module can specify its own logging level (`LOG_LEVEL_[level]`) or use `LOG_LEVEL_NONE`, which will disable the logging for that module.

#### Configuration
1. Enable the logger module. This is done by adding the configuration line below to the application configuration file `prj.conf`: 
   ``CONFIG_LOG=y``
2. Include the header file of the logger module.
   ``#include <zephyr/logging/log.h>``
3. Register your code with the logger module.
   ``LOG_MODULE_REGISTER(Less4_Exer2,LOG_LEVEL_DBG);``

##### [Logging in a module](https://docs.zephyrproject.org/latest/services/logging/index.html#logging-in-a-module)
If we have a module with more than one file and what to use same logger then following configuration is applied. Example is present in [[[NRF-5] 1C - 2P Communication bridge]] node 
- If the module consists of multiple files, then `LOG_MODULE_REGISTER()` should appear in exactly one of them.
- Each other file should use [`LOG_MODULE_DECLARE`](https://docs.zephyrproject.org/latest/services/logging/index.html#c.LOG_MODULE_DECLARE "LOG_MODULE_DECLARE") to declare its membership in the module.
- In order to use logging API in a function implemented in a header file [`LOG_MODULE_DECLARE`](https://docs.zephyrproject.org/latest/services/logging/index.html#c.LOG_MODULE_DECLARE "LOG_MODULE_DECLARE") macro must be used in the function body before logging API is called.
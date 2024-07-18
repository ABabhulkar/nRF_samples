### Monitor mode debugging
In monitor mode, the debugger lets the CPU debug parts of an application while crucial functions continue. Monitor mode is useful for timing-critical applications like Bluetooth Low Energy or PWM, where halting the entire application will affect the timing-critical aspects.

To enable monitor mode for the application, do the following:
1. In the application configuration file, enable the Kconfig options `[CONFIG_CORTEX_M_DEBUG_MONITOR_HOOK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/kconfig/index.html#CONFIG_CORTEX_M_DEBUG_MONITOR_HOOK)` and `[CONFIG_SEGGER_DEBUGMON](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/kconfig/index.html#CONFIG_SEGGER_DEBUGMON)`.
2. Enable monitor mode for the debugger you are using. For nRF Debug in nRF Connect for VS Code extension, enter `-exec monitor exec SetMonModeDebug=1` in the debug console to enable monitor mode.
![[Pasted image 20240717100017.png]]

### Thread Viewer
The Thread Viewer shows you information about the specific threads in the application. Their state uses Zephyr’s [thread states](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html) and is only updated when the debugger is stopped. While the device is running, the View is frozen. Any state in the table is stale until execution is stopped again.
![[Pasted image 20240717101226.png]]
- **Name** – Name of the thread.
- **Priority** – Priority of the thread configured by the user, based on Zephyr’s [thread priorities](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#thread-priorities).
- **Entry** – Name of the thread entry point function in the code.
- **Stack Usage** – Number of bytes used by the thread and the maximum number of bytes that can be allocated to the thread.
- **User Option** – Binary value associated with the thread. The numbers in this column represent bit fields which are based on Zephyr’s [thread options](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/threads/index.html#thread-options). Colors are for readability only.

### Addr2line
[Addr2line](https://man7.org/linux/man-pages/man1/addr2line.1.html) is a command line tool that uses an address in an executable and corresponding debugging information to figure out which file name and line number are associated with the address. This can be used to figure out where a fatal error occurs. addr2line is a part of the GCC package.
The addr2line command follows a general syntax

```c-like
addr2line [options] [addr...]
```

To use the tool, you need to provide two pieces of information:
- one or more addresses to convert.
- binary file that contains the symbols for those addresses.

Some other useful flag options are:
- `-a`: shows the address before each translated location.
- `-f`: shows the name of the function containing each location.
- `-p`: makes the output easier for humans to read.

The syntax `printk()` is similar to the standard [`printf()`](https://en.wikipedia.org/wiki/Printf_format_string) in C, you can provide either a string literal or a format string followed by one or more variables to be printed.
A basic set of specifiers are supported:
- Signed decimal: `%d`, `%i` and its subcategories
- Unsigned decimal: `%u` and its subcategories
- Unsigned hexadecimal: `%x` (`%X` is treated as `%x`)
- Pointer: `%p`
- String: `%s`
- Character: `%c`
- Percent: `%%`
- New line: `\n`
- Carriage return: `\r`
Field width (with or without leading zeroes) is supported. Length attributes `h`, `hh`, `l`, `ll` and `z` are supported.

1. Include the console drivers.
   This is done by enabling the configuration option `CONFIG_CONSOLE` in the application configuration file. This step is not necessary if it is set in the board configuration file.
2. Select the console.
   There are a few options available, such as the UART console (`CONFIG_UART_CONSOLE`) and RTT console (`CONFIG_RTT_CONSOLE`). **The default console set in the board configuration file is the UART console**
3. Include the header file `<zephyr/sys/printk.h>` in your application source code.

> [!Important]
> 
> The output of the `printk()` is not deferred, meaning the output is sent immediately to the console without any mutual exclusion or buffering. This is also known as synchronous logging.
> Logs are sent immediately as they are issued, and `printk()` will not return until all bytes of the message are sent. This **limits** the use of this function in **time-critical applications**.
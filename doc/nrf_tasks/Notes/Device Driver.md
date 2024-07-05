> [!Note]
>  Use `DEVICE_DT_GET()` the benefit of this is fail at build time if the device was not allocated by the driver, for instance, if it does not exist in the [[Devicetree]] or has the status disabled.
>  
>  In addition, unlike `device_get_binding()`, it does not perform a run-time string comparison, which could impact performance in some situations.

![[Pasted image 20240705112255.png]]

The macro [DEVICE_DT_GET()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/drivers/index.html?highlight=device_dt_get#c.DEVICE_DT_GET) has the signature shown below:
![[Pasted image 20240705112649.png]]Two common ways are by the node label through the macro [`DT_NODELABEL()`](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/build/dts/api/api.html#c.DT_NODELABEL) and by an alias through the macro [`DT_ALIAS()`](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/build/dts/api/api.html#c.DT_ALIAS). Before using the device pointer, it should be checked using [device_is_ready()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/drivers/index.html?#c.device_is_ready).
```c
const struct device *dev;
dev = DEVICE_DT_GET(DT_NODELABEL(uart0));

if (!device_is_ready(dev)) {
	//use device API here to use the device
    return;
}
```

For example, if you have two UART peripherals (`&uart0` and `&uart1`) and you want to use them both, you must have two separate pointers of type `const struct device`.

> [!Important]
> Most [peripheral APIs](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/hardware/peripherals/index.html) will have an equivalent to `DEVICE_DT_GET()` and `device_is_ready()` that are specific to the peripheral. For example for the GPIO peripheral, there is `GPIO_DT_SPEC_GET()` and `gpio_is_ready_dt()`.
> 
> Use the peripheral specific macro as they collect more information about the peripheral from the devicetree structure and reduce the need to add peripheral configurations in the application code.

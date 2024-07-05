- Using the board ID, choose the board you want to flash your application to. See the table below for an overview of board ID’s for Nordic devices.

| **Device**                                                                              | **Board ID**                |
| --------------------------------------------------------------------------------------- | --------------------------- |
| [nRF5340 DK](https://www.nordicsemi.com/Products/Development-hardware/nrf5340-dk)       | nrf5340dk_nrf5340_cpuapp_ns |
| [nRF52840 DK](https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk)     | nrf52840dk_nrf52840         |
| [nRF52833 DK](https://www.nordicsemi.com/Products/Development-hardware/nrf52833-dk)     | nrf52833dk_nrf52833         |
| [nRF52 DK](https://www.nordicsemi.com/Products/Development-hardware/nrf52-dk)           | nrf52dk_nrf52832            |
| [nRF9160 DK](https://www.nordicsemi.com/Products/Development-hardware/nrf9160-dk)       | nrf9160dk_nrf9160_ns        |
| [nRF9161 DK](https://www.nordicsemi.com/Products/Development-hardware/nRF9161-DK)       | nrf9161dk_nrf9161_ns        |
| [Thingy:91](https://www.nordicsemi.com/Products/Development-hardware/Nordic-Thingy-91)* | thingy91_nrf9160_ns         |
| [Thingy:53](https://www.nordicsemi.com/Products/Development-hardware/Nordic-Thingy-53)* | thingy53_nrf5340_cpuapp_ns  |
| [nRF7002 DK](https://www.nordicsemi.com/Products/Development-hardware/nRF7002-DK)*      | nrf7002dk_nrf5340_cpuapp_ns |
- Depending on the template you chose, you will be presented with at least one application configuration `prj.conf`. Some templates contain more than one application configuration file (eg: `prj.conf`, `prj_minimal.conf`, `prj_cdc.conf`). These different configurations, if found in the template, are explained in the template documentation.

- In this exercise, we will leave the option to **Use project default**. However, it’s important to remember to use **Optimize for debugging (-Og)** if you plan to debug your application.

- Alternatively, you can checkout this short [video](https://www.youtube.com/watch?v=gQfm9Vlgags&list=PLx_tBuQ_KSqEt7NK-H7Lu78lT2OijwIMl&index=7) or read the documentation of `west` available [here](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/develop/west/index.html#) .
> [!info]
> 
> **Make sure to select nRF Connect Terminal**, as shown in the screenshot below as this is the terminal where the toolchain is sourced.

> [!Importent]
> If you get an error message during flashing, similar to the screenshot below, you will need to recover your device: ![](https://academy.nordicsemi.com/wp-content/uploads/2022/01/readback_protection.png)  
Recovering a device can be done using the command line interface by:  
> 1. Open nRF Connect Terminal  
> 2. Change directory to your application root directory (`cd`)  
> 3. Type `west flash --recover` and hit enter  
> 
> You need to do the steps above only one time. After that, you can use the Flash functionality as usual.

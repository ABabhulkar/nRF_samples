The [nRF Security](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/libraries/security/nrf_security/index.html) subsystem (nrf_security) provides an integration between Mbed TLS and software libraries that provide hardware-accelerated cryptographic functionality on selected Nordic Semiconductor SoCs as well as alternate software-based implementations of the Mbed TLS APIs. The subsystem includes a [[_Index#^257758|PSA driver abstraction layer]] to enable both hardware-accelerated and software-based implementation at the same time.

> [!note]
> To enable nRF Security, set the [`CONFIG_NRF_SECURITY`](https://docs.nordicsemi.com/bundle/ncs-latest/page/kconfig/index.html#CONFIG_NRF_SECURITY "(in Kconfig reference v&nbsp;)") Kconfig option along with additional configuration options as defined in documentation.


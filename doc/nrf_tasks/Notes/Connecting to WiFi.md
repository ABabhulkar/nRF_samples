The [Network Management API](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/net_mgmt.html#network-management) is used to send network requests or receive notifications on network events. The API allows applications, as well as network layer code itself, to call defined network routines at any level in the IP stack, or receive notifications on relevant network events.

#### [[Setup Network management API.canvas|Setup Network management API]] 
- Kconfig [CONFIG_NET_CONNECTION_MANAGER](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/kconfig/index.html#CONFIG_NET_CONNECTION_MANAGER) will add and start the connection manager in the application, and will raise L4 events “connected” and “disconnected”
- It is important to note that the `NET_EVENT_L4_CONNECTED` event is only triggered once your device has received an IP address, so `CONFIG_DHCPV4` must be enabled in the application.

##### Automatic connection
- The Wi-Fi management extension library adds an automatic connect feature to the Wi-Fi stack, by using the [NET_REQUEST_WIFI_CONNECT_STORED](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/libraries/networking/wifi_mgmt_ext.html#c.NET_REQUEST_WIFI_CONNECT_STORED) command in `net_mgmt()`.
- Add the library to your application through the following Kconfig
```
CONFIG_WIFI_MGMT_EXT=y
```
- Example code for main.c
```c
#include <net/wifi_mgmt_ext.h>
...
struct net_if *iface = net_if_get_first_wifi();
int err = net_mgmt(NET_REQUEST_WIFI_CONNECT_STORED, iface, NULL, 0);
if (err) {
	LOG_ERR("Auto-connecting to Wi-Fi failed, err: %d", err);
	return ENOEXEC;
}
```

#### Wi-Fi Provisioning
There are 3 ways to provision the wifi 
- Static Wi-Fi configuration
- Provision over shell
- Provisioning over BLE
- [[[NRF-11] WiFi provisioning using http server]]

#### WiFi credentials backend
Wi-Fi credentials library provides two different backend options for credential storage:
- Zephyr’s settings subsystem - NVM based
- PSA Protected Storage - TF-M based

##### PSA protected storage
- PSA backend requires TF-M which is only included when building with TF-M (“non-secure” board target)
- Create board specific file and put following 
```c-like
CONFIG_WIFI_CREDENTIALS_BACKEND_PSA=y
CONFIG_TFM_PROFILE_TYPE_MEDIUM=y
CONFIG_PM_PARTITION_SIZE_TFM_SRAM=0x18000
CONFIG_MBEDTLS_HEAP_SIZE=16384 
```

##### Zephyr’s settings subsystem 
- Create board specific file and put following 
```c-like
CONFIG_WIFI_CREDENTIALS_BACKEND_SETTINGS=y
CONFIG_FLASH=y
CONFIG_FLASH_PAGE_LAYOUT=y
CONFIG_FLASH_MAP=y
CONFIG_NVS=y
CONFIG_SETTINGS=y
CONFIG_SETTINGS_NVS=y
```

##### Steps to use the library
- Include the header file for the Wi-Fi credentials library `#include <net/wifi_credentials.h>`
- Enable support for issuing commands over shell in the application
```c-like
CONFIG_SHELL=y
CONFIG_WIFI_CREDENTIALS_SHELL=y
CONFIG_SHELL_STACK_SIZE=4400
```
- command to store the credentials
```sh
wifi_cred add "<your_network_SSID>" WPA2-PSK "<your_network_password>"
```
- command to automatically connect to the network that is stored
```sh
wifi_cred auto_connect
```

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

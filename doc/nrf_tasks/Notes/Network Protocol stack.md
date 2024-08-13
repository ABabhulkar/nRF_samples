![[Pasted image 20240812133322.png]]

- L2 layer
	- The layer above the Wi-Fi driver contains the L2 network technology responsible for routing the Wi-Fi traffic beyond the Wi-Fi access point.
	- This is enabled through the Kconfig [CONFIG_NET_L2_ETHERNET](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/kconfig/index.html#CONFIG_NET_L2_ETHERNET)
- IPv4 & IPv6
	- These two versions are in simultaneous use
	- These are enabled through the Kconfigs [CONFIG_NET_IPV4](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/kconfig/index.html#CONFIG_NET_IPV4) and [CONFIG_NET_IPV6](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/kconfig/index.html#CONFIG_NET_IPV6).

#### Scoket API


###### UDP socket connection
- Configure the Zephyr networking API
```c
CONFIG_NETWORKING=y
CONFIG_NET_NATIVE=y
```
- Enable the BSD socket-like API on top of Zephyr’s native networking API and POSIX names for the socket API (to disable the `_zsock` prefix).
```c
CONFIG_NET_SOCKETS=y
CONFIG_NET_SOCKETS_OFFLOAD=n
CONFIG_NET_SOCKETS_POSIX_NAMES=y
CONFIG_POSIX_MAX_FDS=16
```
- Enable the relevant networking configurations
```c
CONFIG_NET_L2_ETHERNET=y
CONFIG_NET_IPV6=y
CONFIG_NET_IPV4=y
CONFIG_NET_UDP=y
CONFIG_NET_DHCPV4=y
CONFIG_DNS_RESOLVER=y
```
- Initialize variables for resolving the server address, creating the socket and receiving messages from the server.
> [!note]
> Using `struct sockaddr_storage` instead of `struct sockaddr` is good practice as it promotes protocol-family independence (see [here](https://stackoverflow.com/questions/16010622/reasoning-behind-c-sockets-sockaddr-and-sockaddr-storage) for a good explanation).
- Resolve the IP address of the server
```c
#include "zephyr/net/net_ip.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/types.h>

#include <dk_buttons_and_leds.h>
#include <net/wifi_credentials.h>
#include <net/wifi_mgmt_ext.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/wifi.h>
#include <zephyr/net/wifi_mgmt.h>

/* STEP 2 - Include the header file for the socket API */
#include <zephyr/net/socket.h>

LOG_MODULE_REGISTER(Lesson3_Exercise1, LOG_LEVEL_INF);

#define EVENT_MASK (NET_EVENT_L4_CONNECTED | NET_EVENT_L4_DISCONNECTED)

/* STEP 3 - Define the hostname and port for the echo server */
#define SERVER_HOSTNAME "udp-echo.nordicsemi.academy"
#define SERVER_PORT "2444"

#define MESSAGE_SIZE 256
#define MESSAGE_TO_SEND "Hello from nRF70 Series"
#define SSTRLEN(s) (sizeof(s) - 1)

static struct net_mgmt_event_callback mgmt_cb;
static bool connected;
static K_SEM_DEFINE(run_app, 0, 1);
  
/* STEP 4.1 - Declare the structure for the socket and server address */
static int sock;
static struct sockaddr_storage server;

/* STEP 4.2 - Declare the buffer for receiving from server */
static uint8_t recv_buf[MESSAGE_SIZE];

static void net_mgmt_event_handler(struct net_mgmt_event_callback *cb,                                   uint32_t mgmt_event, struct net_if *iface) {
  if ((mgmt_event & EVENT_MASK) != mgmt_event) {
    return;
  }

  if (mgmt_event == NET_EVENT_L4_CONNECTED) {
    LOG_INF("Network connected");
    connected = true;
    dk_set_led_on(DK_LED1);
    k_sem_give(&run_app);
    return;
  }

  if (mgmt_event == NET_EVENT_L4_DISCONNECTED) {
    if (connected == false) {
      LOG_INF("Waiting for network to be connected");
    } else {
      dk_set_led_off(DK_LED1);
      LOG_INF("Network disconnected");
      connected = false;
    }

    k_sem_reset(&run_app);
    return;
  }
}

static int server_resolve(void) {
  /* STEP 5.1 - Call getaddrinfo() to get the IP address of the echo server */
  int err;
  struct addrinfo *result;
  struct addrinfo hints = {.ai_family = AF_INET, .ai_socktype = SOCK_DGRAM};

  err = getaddrinfo(SERVER_HOSTNAME, SERVER_PORT, &hints, &result);
  if (err != 0) {
    LOG_INF("getaddrinfo() failed, err: %d", err);
    return -EIO;
  }

  if (result == NULL) {
    LOG_INF("Error, address not found");
    return -ENONET;
  }

  /* STEP 5.2 - Retrieve the relevant information from the result structure */
  struct sockaddr_in *server4 = ((struct sockaddr_in *)&server);
  server4->sin_addr.s_addr =
      ((struct sockaddr_in *)result->ai_addr)->sin_addr.s_addr;
  server4->sin_family = AF_INET;
  server4->sin_port = ((struct sockaddr_in *)result->ai_addr)->sin_port;

  /* STEP 5.3 - Convert the address into a string and print it */
  char ipv4_addr[NET_IPV4_ADDR_LEN];
  inet_ntop(AF_INET, &server4->sin_addr.s_addr, ipv4_addr, sizeof(ipv4_addr));
  LOG_INF("IPv4 address of server found %s", ipv4_addr);

  /* STEP 5.4 - Free the memory allocated for result */
  freeaddrinfo(result);
  return 0;
}

static int server_connect(void) {
  int err;
  /* STEP 6 - Create a UDP socket */
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0) {
    LOG_INF("Failed to create socket: %d.\n", err);
    return -err;
  }

  /* STEP 7 - Connect the socket to the server */
  err = connect(sock, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
  if (err < 0) {
    LOG_INF("Connect failed: %d\n", err);
    return -err;
  }

  return 0;
}

static void button_handler(uint32_t button_state, uint32_t has_changed) {
  /* STEP 8 - Send a message every time button 1 is pressed */
  if (has_changed & DK_BTN1_MSK && button_state & DK_BTN1_MSK) {
    int err = send(sock, MESSAGE_TO_SEND, SSTRLEN(MESSAGE_TO_SEND), 0);
    if (err < 0) {
      LOG_INF("Failed to send message, %d", errno);
      return;
    }

    LOG_INF("Successfully sent message: %s", MESSAGE_TO_SEND);
  }
}

int main(void) {
  int received;
  
  if (dk_leds_init() != 0) {
    LOG_ERR("Failed to initialize the LED library");
  }

  /* Sleep to allow initialization of Wi-Fi driver */
  k_sleep(K_SECONDS(1));

  net_mgmt_init_event_callback(&mgmt_cb, net_mgmt_event_handler, EVENT_MASK);
  net_mgmt_add_event_callback(&mgmt_cb);

  LOG_INF("Waiting to connect to Wi-Fi");
  k_sem_take(&run_app, K_FOREVER);

  if (dk_buttons_init(button_handler) != 0) {
    LOG_ERR("Failed to initialize the buttons library");
  }

  /* STEP 9 - Resolve the server name and connect to the server */
  if (server_resolve() != 0) {
    LOG_INF("Failed to resolve server name");
    return 0;
  }

  if (server_connect() != 0) {
    LOG_INF("Failed to initialize client");
    return 0;
  }

  LOG_INF("Press button 1 on your DK to send your message");

  while (1) {
    /* STEP 10 - Listen for incoming messages */
    received = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
    if (received < 0) {
      LOG_ERR("Socket error: %d, exit", errno);
      break;
    }

    if (received == 0) {
      LOG_ERR("Empty datagram");
      break;
    }
    recv_buf[received] = 0;
    LOG_INF("Data received from the server: (%s)", recv_buf);
  }
  close(sock);
  return 0;
}
```

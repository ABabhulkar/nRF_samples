The nRF Connect SDK contains an [MQTT client library](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/mqtt.html?). The MQTT library is built on top of the sockets API, so the library handles creating the socket used to communicate with the MQTT broker.
In this topic, we will cover how to enable and configure the library to connect to an MQTT broker.

##### Enable and configure MQTT library
- Enable MQTT library in your application by enabling the following Kconfig symbols in the `prj.conf` file.
```c-like
CONFIG_MQTT_LIB=y
# Setting this flag diables the persistent session.
CONFIG_MQTT_CLEAN_SESSION=y

# Parameters
CONFIG_MQTT_PUB_TOPIC="<your topic>"
CONFIG_MQTT_SUB_TOPIC="<your topic>"
CONFIG_MQTT_BROKER_HOSTNAME="<host addr>"
CONFIG_MQTT_BROKER_PORT=<port>
```

##### Create an MQTT client
- The first step is to initialize and configure the MQTT client, of type [struct mqtt_client](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/mqtt.html#c.mqtt_client). 
- This struct holds connection information and needs to be passed to subsequent client calls.
- Before configuring the structure, we must initialize it with [mqtt_client_init()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/mqtt.html?#c.mqtt_client_init). Then we can set parameters like the broker details, the callback function and client ID.

##### Define the callback function
- Define the callback function of type [mqtt_evt_cb_t](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/mqtt.html?#c.mqtt_evt_cb_t), which takes the client `struct mqtt_client` and the event [struct mqtt_evt](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/mqtt.html?#c.mqtt_evt) as parameters.
- [`mqtt_evt_type`](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/mqtt.html?#c.mqtt_evt_type) contains the full list of possible MQTT events.

##### Connect to the MQTT broker
- To connect we will use the function [mqtt_connect()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/mqtt.html#c.mqtt_connect) 
- To receive incoming packets, we call [mqtt_input()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/mqtt.html?#c.mqtt_input). We poll the socket for a `POLLIN` event to know when to call this function.
- Due to the keep-alive functionality of MQTT, we periodically call [mqtt_live()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/mqtt.html?#c.mqtt_live) to keep the connection alive.
- Just like closing the socket, it is important to call [mqtt_disconnect()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/mqtt.html?#c.mqtt_disconnect) to close the connection and deallocate the resources.

##### Publish and subscribe
- Subscribe:
	- Define a [struct mqtt_topic](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/mqtt.html?#c.mqtt_topic) for each topic and set the name of the topic and the QoS.
	- Then define [struct mqtt_subscription_list](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/mqtt.html?#c.mqtt_subscription_list), essentially an array consisting of all the topics, and pass that to [mqtt_subscribe()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/mqtt.html?#c.mqtt_subscribe) along with the client.
- Publish
	- Define [struct mqtt_publish_param](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/mqtt.html?#c.mqtt_publish_param) and set the name of the topic to publish to, along with the QoS, message to publish, and message ID.
	- Pass this to [mqtt_publish()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/connectivity/networking/api/mqtt.html?#c.mqtt_publish) along with the client, to publish the message.


### Securing the MQTT connection with TLS
- [ ] TODO: add secure mqtt data here
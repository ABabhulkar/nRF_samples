#ifndef _NETWORK_INTERFACE_H_
#define _NETWORK_INTERFACE_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum network_type { WIFI_ESP, CELLULAR, WIFI_NRF } network_type;

typedef struct {
    uint8_t ip[4];
    uint16_t port;
    uint8_t timeout;
    // TODO: add list of topics to subscribe
} Connection_param;

typedef struct {
    Connection_param con_param;
    void (*init)();
    void (*connect)(Connection_param* con_param);
    void (*subscribe)(const char* topic[], uint16_t num_topics);
    bool (*isConnected)();
    void (*sendFrame)(const char* topic, const char* msg);
    void (*reconnect)();

    // return: mack of the device in string form
    char* (*getMAC)();
} Connection;

Connection* init_connection(network_type type, Connection_param param);

#endif /* _NETWORK_INTERFACE_H_ */

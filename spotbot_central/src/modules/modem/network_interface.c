#include "network_interface.h"

#include "modules/modem/esp/esp.h"

#include <stdlib.h>


Connection* init_connection(network_type type, Connection_param param)
{
    Connection* con = (Connection*)malloc(sizeof(Connection));
    switch (type) {
    case WIFI_ESP:
        con->con_param = param;
        con->init = esp_init;
        con->connect = esp_connect;
        con->isConnected = esp_isConnected;
        con->reconnect = esp_reconnect;
        con->getMAC = esp_getMAC;
        con->sendFrame = esp_sendFrame;
        con->subscribe = esp_subscribe;
        break;
    case WIFI_NRF:
        break;
    case CELLULAR:
        break;
    default:
        // TODO: add defaults
        break;
    }
    return con;
}

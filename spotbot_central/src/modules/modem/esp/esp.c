#include "esp.h"


void esp_connect(Connection_param* con_param)
{
    // send ip address over uart and ask for connection
}

void esp_subscribe(const char* topic[], uint16_t num_topics)
{
    // send list of topics to be subscribed
}

bool esp_isConnected()
{
    // TODO check count of recived periodic frame and determine connection state
    return true;
}

void esp_sendFrame(const char* topic, const char* msg)
{
    // send frame over uart
}

void esp_reconnect()
{
    // send reconnection request
}

// return: mack of the device in string form
char* esp_getMAC()
{
    return "dummy";
}

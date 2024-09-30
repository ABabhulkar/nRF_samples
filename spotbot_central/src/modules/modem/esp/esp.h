#ifndef _ESP_H_
#define _ESP_H_

#include "../network_interface.h"

void esp_init();
void esp_connect(Connection_param* con_param);
void esp_subscribe(const char* topic[], uint16_t num_topics);
bool esp_isConnected();
void esp_sendFrame(const char* topic, const char* msg);
void esp_reconnect();

// return: mack of the device in string form
char* esp_getMAC();

#endif /* _ESP_H_ */
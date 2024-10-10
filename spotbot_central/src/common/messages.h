#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include "zephyr/bluetooth/addr.h"

#define TX_CHANNEL tx_ch
#define CONFIG_UPDATE_CHANNEL notify_config

typedef struct {
    bt_addr_t node_id;
    int16_t x, y, z;
    uint8_t status;
    uint16_t temp;
} node_data;

#endif /* _MESSAGES_H_ */

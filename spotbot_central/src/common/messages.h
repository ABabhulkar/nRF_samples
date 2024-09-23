#ifndef _MESSAGES_H_
#define _MESSAGES_H_
#include <stdint.h>

#define TX_CHANNEL tx_ch

struct tx_msg {
    uint8_t* data;
    uint8_t size;
};

#endif /* _MESSAGES_H_ */

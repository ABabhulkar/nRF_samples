#include "messages.h"

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>


ZBUS_CHAN_DEFINE(TX_CHANNEL, /* Name */
                 node_data,  /* Message type */

                 NULL,                 /* Validator */
                 NULL,                 /* User data */
                 ZBUS_OBSERVERS_EMPTY, /* observers */
                 ZBUS_MSG_INIT(0)      /* Initial value {0} */
);
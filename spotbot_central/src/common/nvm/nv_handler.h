#ifndef _NV_HANDLER_INTERFACE_H_
#define _NV_HANDLER_INTERFACE_H_

#include <stdint.h>

typedef enum {
    E_OK,
    E_FLASH_NOT_READY,
    E_NO_PAGE_INFO,
    E_INIT_FAILED,
    E_NOT_INITALIZED,
    E_NO_ADDRESS,
    E_ALREADY_PRESENT,
    E_ERROR
} nvError;

nvError nv_init();
nvError read_ip(uint8_t* ip, const uint8_t* default_ip);
nvError write_ip(const uint8_t* ip);

#endif /* _NV_HANDLER_INTERFACE_H_ */

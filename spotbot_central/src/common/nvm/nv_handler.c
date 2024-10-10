#include "nv_handler.h"

#include "zephyr/logging/log.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>


#define ADDRESS_ID 1

#define NVS_PARTITION storage_partition
#define NVS_PARTITION_DEVICE FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET FIXED_PARTITION_OFFSET(NVS_PARTITION)

LOG_MODULE_REGISTER(nv_handler_module, LOG_LEVEL_DBG);

static struct nvs_fs fs;
static bool is_init = false;

/* define the nvs file system by settings with:
 *	sector_size equal to the pagesize,
 *	2 sectors
 *	starting at NVS_PARTITION_OFFSET
 */
nvError nv_init()
{
    int rc = 0;
    struct flash_pages_info info;

    if (is_init) {
        return E_OK;
    }

    fs.flash_device = NVS_PARTITION_DEVICE;
    if (!device_is_ready(fs.flash_device)) {
        LOG_ERR("Flash device %s is not ready\n", fs.flash_device->name);
        return E_FLASH_NOT_READY;
    }
    fs.offset = NVS_PARTITION_OFFSET;
    rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
    if (rc) {
        LOG_ERR("Unable to get page info\n");
        return E_NO_PAGE_INFO;
    }
    fs.sector_size = info.size;
    fs.sector_count = 3U;

    rc = nvs_mount(&fs);
    if (rc) {
        LOG_ERR("Flash Init failed\n");
        return E_INIT_FAILED;
    }
    is_init = true;
    return E_OK;
}

/* ADDRESS_ID is used to store an address, lets see if we can
 * read it from flash, since we don't know the size read the
 * maximum possible
 */
nvError read_ip(uint8_t* ip, const uint8_t* default_ip)
{
    int rc = 0;

    if (!is_init) {
        return E_NOT_INITALIZED;
    }

    rc = nvs_read(&fs, ADDRESS_ID, ip, 4);
    if (rc <= 0) { /* item was not found, add it */
        memcpy(ip, default_ip, 4);
        LOG_DBG("No address found, adding %s at id %d\n", ip, ADDRESS_ID);
        return E_NO_ADDRESS;
    }

    /* item was found, show it */
    LOG_DBG("Id: %d:%d:%d:%d, Address: %d\n", ip[0], ip[1], ip[2], ip[3], ADDRESS_ID);
    return E_OK;
}

nvError write_ip(const uint8_t* ip)
{
    int isSame = 0;
    int rc = 0;

    if (!is_init) {
        return E_NOT_INITALIZED;
    }

    uint8_t temp_ip[4] = {0, 0, 0, 0};
    rc = nvs_read(&fs, ADDRESS_ID, temp_ip, 4);
    if (rc <= 0) { /* item was not found, add it */
        LOG_DBG("No address found, adding %s at id %d\n", ip, ADDRESS_ID);
        return E_ERROR;
    }

    isSame = memcmp(ip, temp_ip, 4);
    if (isSame == 0) {
        return E_ALREADY_PRESENT;
    }

    (void)nvs_write(&fs, ADDRESS_ID, ip, 4);
    return E_OK;
}

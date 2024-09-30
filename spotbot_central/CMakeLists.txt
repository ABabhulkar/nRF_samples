# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(spotbot_central)

target_include_directories(app PRIVATE src)

# TODO: saperate this into multiple modules
target_sources(app PRIVATE
  src/modules/ble/ble.c
  src/common/common.c
  src/modules/modem/mqtt_handler.c
  src/modules/modem/network_interface.c
  src/modules/modem/esp/esp.c
)

zephyr_library_include_directories(${ZEPHYR_BASE}/samples/bluetooth)

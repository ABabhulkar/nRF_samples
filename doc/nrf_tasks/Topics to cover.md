#### Zephyr OS topics
- Build system [West](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/develop/west/index.html) ✅ (link in build page)
- Memory management ❌ (Nothing specific)
- Debugging tools ✅
- Power modes (Not that into focus for now)
	- https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/test_and_optimize/optimizing/power_general.html
	- https://docs.zephyrproject.org/latest/services/pm/index.html
- Scheduling algorithms ✅
- testing frameworks [[[NRF-9] Debugging and testing of applications|[NRF-9]\]]
- Inter thread communication (Queues, FIFO, mailboxes)
- Zephyr OS networking stack
	- Using Wireshark
- file system support
- NV memory
- Events
- Multi-domain build ✅(link in build page)

#### Things to do with nRF53 board
- [ ] Nordic UART service switch (Sample 1C-1P): setup 1C-2P [[[NRF-3] Create first application|NRF3]], [[[NRF-4] Update UART central to connect with 2 nodes|NRF-4]], [[[NRF-5] 1C - 2P Communication bridge|NRF-5]]
- [ ] Gather sensor data and upload to MQTT (BLE + WiFi): understanding wifi companion communication.
- [ ] HTTP based: Gather sensor readings and send the data on HTTP server. (1 server board - 1 client board)
- [ ] Deep sleep: NFC based pairing and wakeup from deep sleep
- [ ] BLE mesh: **TBD**
- [ ] nRF53 Samples: Work with multi core and do inter core communication.
- [ ] Cryptography: [[[NRF-5] 1C - 2P Communication bridge|NRF-5]], **TBD**
- [ ] MCUBoot: **TBD**
- [ ] OTA: [[[NRF-6] Simple application with OTA]], **TBD**
- [ ] Testing framework: [[[NRF-9] Debugging and testing of applications|[NRF-9]\]]

#### PoC Ideas
1. Connectivity Bridge (Modbus to BLE)
	Idea is to have a device with can connect to legacy RS485 modbus sensors or PLC's and communicates the data wirelessly to data logger/monitor.
	Following things can be done technically:
	- Gather sensor data in modbus 
	- Encrypt the data using onboard crypto cell
	- Transmit the data to central gateway or to server using mqtt/http
	- Dashboard side device collects the data from gateway or server decrypts it and converts back to modbus for existing on site dashboards.
	- Both devices should be OTA compatible
	- Modbus config should be possible from phone (SensorID, register address, etc)
	- TBD: write modbus as shared lib so that can be used in isolation prefer using c++
	- TBD: All sensor side devices can be in a mesh network so that we don't need gateways
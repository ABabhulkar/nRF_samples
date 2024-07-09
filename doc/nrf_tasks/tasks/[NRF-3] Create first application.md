---
Epic: "[[[NRF-E2] BLE sample with multi threading]]"
Created: 2024-07-03
Due: 
Status: Backlog
Assigned to: "[[Abhishek]]"
tags:
  - "#SmallSamples"
  - BLE_UART
  - "#Node"
Git:
---
#### Description
Create first application by taking nordic UART service sample as base. 
**Scope:** Add a sensor reading gathering in a parallel thread.
**Topics to learn:**
- IPC communication
- Life cycle of [[User defined threads]]

#### Sub-Task
- [ ] Create an application with nRF53 board
- [ ] (Optional) Add support for nRF52 board
- [ ] Add logs on all life cycle methods
- [ ] There should be 2 threads one for BLE UART handling and one for sensor data gathering
	- Sensor thread
		- Create a counter which should be initialized once on thread creation.
		- Update the counter value and sleep for 500ms
		- Send the counter value to BLE thread
		- Kill the thread if counter reach 10.
	- BLE thread
		- Connect with Nordic UART thread and register RX handler
		- If 'start' is received from central then start Sensor thread.
		- Send value received from sensor thread to central

#### Task Relationships
- Blocked By: 
	- [[[NRF-2] Dev environment setup]]
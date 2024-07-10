---
Epic: "[[[NRF-E2] BLE sample with multi threading]]"
Created: 2024-07-03
Due: 
Status: InProgress
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
- [x] Create an application with nRF53 board
- [x] (Optional) Add support for nRF52 board 
	- 09-07-02024: Not a big deal just have to add one build config.
- [x] [[README#^95fe53| nRF connect fundamentals]] course from Nordic 
- [ ] ~~Add logs on all life cycle methods~~ [[[NRF-7] Zephyr RTOS beyond basics|[NRF-7]]]
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
		- 09-07-2024: Unblocked

---
##### 09-07-2024 [[Abhishek]]
**Learnings till now**
- Project creation, build and flash
- Basics of kconfig gui and prj.config
- Creation and synchronization of threads
- Using logger
- Completed Nordic connect fundamental courses
**Next Steps:**
- [ ] Create branch and new application from BLE uart service
- [ ] Start working on task goal
- [ ] (Optional) Get the RTT logger backend running


---
Epic: "[[[NRF-E2] BLE sample with multi threading]]"
Created: 2024-07-03
Due: 
Status: InProgress
Assigned to: "[[Abhishek]]"
tags:
  - "#SmallSamples"
  - "#BLE_UART"
  - "#Central"
Git:
---
#### Description
Create first application by taking nordic UART service sample as base. 
**Scope:** Add a sensor reading gathering in a parallel thread.
**Topics to learn:**
- Project configuration (BLE parameter)
- ~~Passing arguments to threads
- ~~Explore if multiple [[Threads]] can use same runnable~~
	- Done with [[Mailboxes]] example

#### Sub-Task
- [x] Create application by taking UART central sample as a base
- [ ] Update configuration to make it connect with 2 nodes instead of 1
- [ ] Central should have at least 2 threads, separate thread should be created for each connection.
- [ ] (Optional) Implement all logic as single runnable and create a thread to run that runnable on connection.
- [ ] Each thread should do following
	- Manage connection with the node
	- log any msg received from node 
	- send msg back to node
- [ ] In case connection is lost then end the thread
- [x] ~~(Optional) Add support for nRF52 board~~
	- This is done by default 
 
#### Task Relationships
- Blocked By: 
	- [[[NRF-2] Dev environment setup]]

---
##### 24-07-2024
- In order to mimic peripherals you have to set the gatt server in nrf connect app #nRFConnectApp
- Found one example which is exactly doing things mentioned in this ticket.
	- https://github.com/NordicMatt/multi-NUS?tab=readme-ov-file
	- This example don't use multiple threads for this task.
	- It uses a msg prefix to achieve gatewaying of msg between nodes.
###### Next steps
- Investigate if separate threads are possible for each BLE connection as it happens based on callbacks. 
- Try to understand and optimize found code.
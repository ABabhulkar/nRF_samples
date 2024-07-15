---
Epic: "[[[NRF-E2] BLE sample with multi threading]]"
Created: 2024-07-03
Due: 
Status: Backlog
Assigned to: 
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
- Passing arguments to threads
- Explore if multiple [[Threads]] can use same runnable

#### Sub-Task
- [ ] Create application by taking UART central sample as a base
- [ ] Update configuration to make it connect with 2 nodes instead of 1
- [ ] Central should have at least 2 threads, separate thread should be created for each connection.
- [ ] (Optional) Implement all logic as single runnable and create a thread to run that runnable on connection.
- [ ] Each thread should do following
	- Manage connection with the node
	- log any msg received from central 
	- echo msg back to central
- [ ] In case connection is lost then end the thread
- [ ] (Optional) Add support for nRF52 board
 
#### Task Relationships
- Blocked By: 
	- [[[NRF-2] Dev environment setup]]
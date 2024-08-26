---
Epic: "[[[NRF-E2] BLE sample with multi threading]]"
Created: 2024-07-17
Due: 
Status: OnHold
Assigned to: "[[Abhishek]]"
tags:
  - setup
  - SmallSamples
Git:
---
#### Description
Get basic understanding of the testing framework in nRF connect sdk and get hands of training with some samples.
#### Sub-Task
- [ ] Identify best testing framework for nrf connect tests
- [ ] Create some unit tests for application created in [[[NRF-3] Create first application|[NRF-3]\]]
- [ ] Crease some integration tests for the application

#### Task Relationships
- Caused by:
	- [[[NRF-3] Create first application]]

---
#### 18-07-2024
- [[Testing with ZTest]]
	- Working with following command
```powershell
python ..\..\ncs\v2.6.0\zephyr\scripts\twister -W --device-testing --device-serial COM9 --west-runner=nrfjprog --west-flash="--erase,--softreset" -p nrf52840dk_nrf52840 -T .\blinkyLED\tests\
```

**TODO**
- It looks like native_sim or native_posix doesn't work on windows so it makes sense to do setup in WSL for this. 
- Check how to write a code so that it will be lib and then it can be unit tested.

#### 19-07-2024
- This ticket is on hold for now due to lack of proper documentation

#### 23-8-2024
- Building with [[Native Sim]]✅
- [[WSL port forwarding]] can be done in order to program directly.
- [[Testing with ZTest#^f416ce|Running tests on native_sim]]

TODO
- Check how to use buttons in the tests and how to check the status of the buttons in [[Native Sim]] environment

#### 26-8-2024
- debug using native_sim and gdb. 
	- Install gdb and extension native debug 
	- Setup the debug config
- Bsim
	- Install 32 bit c lib ``sudo apt-get install gcc-multilib``
	- Then follow steps on https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/boards/native/nrf_bsim/doc/nrf52_bsim.html
	- Note that if any led or buttons are used in code then board overlay file has to be created.
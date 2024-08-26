
1. Follow quick [start guide](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/develop/test/ztest.html#quick_start_-_integration_testing) for integration testing
2. using following command and twister you can get the tests running on HW.
```powershell
python ..\..\ncs\v2.6.0\zephyr\scripts\twister -W --device-testing --device-serial COM9 --west-runner=nrfjprog --west-flash="--erase,--softreset" -p nrf52840dk_nrf52840 -T .\blinkyLED\tests\

# or using west 
west twister -W --device-testing --device-serial COM9 --west-runner=nrfjprog --west-flash="--erase,--softreset" -p nrf52840dk_nrf52840 -T .\blinkyLED\tests\
```

##### HW map file
```powershell
python ..\..\ncs\v2.6.0\zephyr\scripts\twister --generate-hardware-map map.yml
```
This is command to generate the map file for the HW. later open the file and update all the information into it. More details are available on nordic documentation.


##### Running on native_sim

^f416ce

```sh
west twister -p native_sim_64 -T tests
```
- This command will run all tests in the project on native_sim target

#### Reference
- https://www.youtube.com/watch?v=MjdTHEeBq9o
	- This video shows how to get the HW testing working and basics of ZTest
- https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/develop/test/index.html
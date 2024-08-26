- Following command can be used to build and run for the native_sim target once it setup
```sh
west build --build-dir ./blinkyLED/build /blinkyLED --pristine --board native_sim_64 --no-sysbuild -- -DNCS_TOOLCHAIN_VERSION=NONE
```

> [!note]
> As we are working with ubuntu 22-04 it officially supports only 64bit c libs. In order to use native_sim target which is 32bit we need to install 32bit libs. Where as native_sim_64 is 64bit variant of the same board. 
- In order to use Ztests we have to install the libffi 7 using following commands
```sh
wget http://es.archive.ubuntu.com/ubuntu/pool/main/libf/libffi/libffi7_3.3-4_amd64.deb
sudo dpkg -i libffi7_3.3-4_amd64.deb
# to confirm the installation
apt list --installed | grep ffi
```


##### Reference
- https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/boards/native/native_sim/doc/index.html#emulators
- https://docs.zephyrproject.org/latest/boards/native/native_sim/doc/index.html#how-to-use-it
- 
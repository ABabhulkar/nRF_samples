I've managed to solve the issue by sending the following  commands sequentially:

```shell
# Was able to clean new DK board in order for it to detect
nrfjprog --recover --coprocessor CP_APPLICATION

nrfjprog --recover --coprocessor CP_NETWORK
nrfjprog --recover
nrfjprog -f NRF53 --coprocessor CP_NETWORK --program your_file.hex --chiperase --verify 
nrfjprog -f NRF53 --program your_file.hex --chiperase
nrfjprog --pinreset
```

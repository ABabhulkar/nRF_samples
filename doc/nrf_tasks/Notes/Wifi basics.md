Overview of communication stack for nRF70 series. 
![[Pasted image 20240808103005.png]]
Here the hostMCU can be any MCU but mainly we would focus on nRF53.

##### Radio coexistence
Since Wi-Fi shares unlicensed radio bands with other technologies, such as Bluetooth, radio coexistence becomes an important aspect. This is especially important for the nRF70 Series as it is often implemented alongside Bluetooth Low Energy in the same device. These two technologies using the same radio band can cause signal quality degradation on both radios and mutual interference. Therefore, correct coexistence mechanisms ensure minimal interference between the various radio technologies sharing the same radio bands.
- The coexistence function in the nRF70 is based on the IEEE-defined Packet Traffic Arbitration (PTA) module
- Two main sets of hardware configurations can be used
	- Hardware configuration![[Pasted image 20240808103638.png]]
	- Antenna configuration
	  Two antenna modes are available: **shared antenna mode** and **separate antenna mode**. In the shared antenna mode, the PTA makes priority decisions and controls the switch between Wi-Fi and the secondary technology. In the separate antenna mode, each technology is permanently connected to its own antenna.
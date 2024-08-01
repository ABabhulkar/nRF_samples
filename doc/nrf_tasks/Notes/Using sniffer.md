#### Steps to setup
##### On board
- Download [nRF Sniffer for Bluetooth LE](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Sniffer-for-Bluetooth-LE/Download#infotabs) v4.x 
- Open up nRF Connect for Desktop and install and launch the Programmer application.
- Select the board you are using as the Bluetooth LE sniffer.
- Select **Add file** and select correct hex for the board. Click **Erase & write** to flash the firmware to your board.
##### Wireshark
- Go to the [Wireshark download page](https://www.wireshark.org/download.html).
- In the **Stable Release** list at the top of the page, select the release package download and install it
- Install python requirements
	- Open a command window and navigate to the folder `Sniffer_Software/extcap`.
	- On Windows without Python launcher, type ``python -m pip install -r requirements.txt.``
- Copy the nRF Sniffer capture tool into Wireshark
	- In Wireshark Go to **Help** > **About Wireshark** 
	- Select Folder tab
	- Double-click location for the Personal Extcap path and click ok.
	- Copy the contents of the `Sniffer_Software/extcap/` folder into this folder.
- Enable the nRF Sniffer capture tool in Wireshark.
	- Refresh the interfaces in Wireshark by selecting **Capture** > **Refresh Interfaces**
	- Select **View** > **Interface Toolbars** > **nRF Sniffer for Bluetooth LE** to enable the nRF Sniffer interface.![[Pasted image 20240729110926.png]]

Following image shows the anatomy of a connection
![[Pasted image 20240729152055.png]]


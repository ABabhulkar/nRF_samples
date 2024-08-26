
- Go to the [latest release page for the usbipd-win project](https://github.com/dorssel/usbipd-win/releases).
- Select the .msi file, which will download the installer.
- Run the downloaded usbipd-win_x.msi installer file.
- List all of the USB devices connected to Windows by opening PowerShell in _administrator_ mode and entering the following command. Once the devices are listed, select and copy the bus ID of the device you’d like to attach to WSL.
``` ps
usbipd list
```
- Before attaching the USB device, the command `usbipd bind` must be used to share the device, allowing it to be attached to WSL. This requires administrator privileges. Select the bus ID of the device you would like to use in WSL and run the following command. After running the command, verify that the device is shared using the command `usbipd list` again.
```ps
usbipd bind --busid <busid>
```
- To attach the USB device, run the following command. Ensure that a WSL command prompt is open in order to keep the WSL 2 lightweight VM active. 
```ps
usbipd attach --busid <busid>
```
> [!note]
> Note that as long as the USB device is attached to WSL, it cannot be used by Windows. Once attached to WSL, the USB device can be used by any distribution running as WSL 2.
> Verify that the device is attached using `usbipd list`. From the WSL prompt, run `lsusb` to verify that the USB device is listed and can be interacted with using Linux tools.
- Open Ubuntu (or your preferred WSL command line) and list the attached USB devices using the command: ``lsusb``
- Once you are done using the device in WSL, you can either physically disconnect the USB device or run this command from PowerShell:
```ps
usbipd detach --busid <busid> 
```

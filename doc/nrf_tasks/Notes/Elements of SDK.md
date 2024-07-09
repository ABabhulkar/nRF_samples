#### App and board config
- Each application in the nRF Connect SDK must have an _application configuration file_, usually called `prj.conf`
- Each configuration option must start with the prefix `CONFIG_` followed by the name of the software module to configure, then the value to be set, **with no spaces around the equals sign**
- In addition to the application configuration file, an application inherits the board configuration file, `<board_name>_defconfig`, of the board that the application is built for.
> [!Important]
> You should never modify any board configuration files. Instead, rely on the application configuration file to set new configurations and subsequently overwrite any default board configurations if needed.

- An alternative way to modify the contents of the prj.conf (application configuration file) is by using the nRF Kconfig GUI
- There are 3 different ways to save those changes, as shown below:
![[Pasted image 20240706160046.png]]
	- Apply: Save temporary into .config in build dir. Clean build will loose the config
	- Save to file: Changes will be added to prj.config file
	- Save minimal: Saves onlt the changes we just main into separate file.

#### Devicetree overlays
- It is not recommended to modify the devicetree of board directly, so instead we use devicetree overlays to do this. The overlay only needs to include the node and property it wants to modify.
- If an overlay file sets a node’s property to a value it already has, the node will just remain unchanged.

#### Multi-Image Builds
- Multi-image builds consist of a parent image and one or more child images, where the child image is included by the parent image.
- A good example of a multi-image build happening automatically is when building for a non-secure domain in the nRF53 or the nRF91 series
- boards with `_ns` in end are building for a non-secure domain. This will create 2 images.
![[Pasted image 20240706161640.png]]
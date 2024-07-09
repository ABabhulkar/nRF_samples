1. Define our own config that will determine if our custom files get included in the build or not. To do this, create a file called `Kconfig` in the application directory

2. Make sure the file does not have a file extension. 
```c-like
source "Kconfig.zephyr"

config MYFUNCTION
	bool "Enable my function"
	default n
	
//  Lines above define the configuration `CONFIG_MYFUNCTION` as a boolean variable //  and sets its default value to `n`.
```
If you are interested in learning more about creating menus in Kconfig, here is a link to [documentation](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/build/kconfig/index.html) with more details.

3. Change the line to use the function `target_sources_ifdef()`, like this in cmakelist.txt
```c-like
target_sources_ifdef(CONFIG_MYFUNCTION app PRIVATE src/myfunction.c)
```
The build will now only include the custom file `myfunction.c` if `CONFIG_MYFUNCTION` is enabled. 

> [!Important]
> This strategy is used intensively in nRF Connect SDK only to include the source code of libraries that you plan to use in your project.
>
> The reason for this is to limit the size of your application. Modules and subsystems are only included in the build when you enable the relevant configuration, allowing you to keep the application as small as you wish.

4. Lastly, enable the config by adding the following line to `prj.conf`:
```c-like
CONFIG_MYFUNCTION=y
```

5. Update your main.c file so that it can check if the Kconfig symbol is enabled/disabled
```c
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#ifdef CONFIG_MYFUNCTION
#include "myfunction.h"
#endif
void main(void)
{
	
	while(1){
		#ifdef CONFIG_MYFUNCTION
		int a = 3, b = 4;
			printk("The sum of %d and %d is %d\n\r", a, b, sum(a,b));
		#else
			printk("MYFUNCTION not enabled\r\n");
			return;
		#endif
		k_msleep(1000);
	}
}
```
The exercise covers the use of the [Kernel Timer API](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/timing/timers.html). It is used to periodically trigger a function, called an interrupt service routine (ISR), that runs in the System timer interrupt context. The Kernel uses the System timer, which is the RTC1 peripheral on the nRF91, nRF53, and nRF52 Series hardware.

1. STEP 1: **Define a Timer**
   The timer is defined by using the [K_TIMER_DEFINE()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/timing/timers.html#c.K_TIMER_DEFINE) macro. It takes three parameters:
   - _Name_ of the timer to be created.
   - An _expiry function_ that is executed each time the timer expires (fires). If no expiry function is required, a `NULL` function can be specified.
   - A _stop function_ is executed if the timer is stopped prematurely while running. If no stop function is required, a `NULL` function can be specified.

2. STEP 2: **Start the timer.**
   The timer will be started with the [k_timer_start()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/timing/timers.html#c.k_timer_start) function. Use the function to set the timer as a periodic timer or a one-shot timer.
   - The _timer_ variable, which you defined in Step 1
   - The _duration_, which is the time interval for the timer to expire for the first time. This is a `k_timeout_t` value that may be initialized using different units, such as `K_MSEC()` or `K_SECONDS()`.
   - The _period_, which is the time interval between all timer expirations after the first one, also a `k_timeout_t` value that must be non-negative.
   Setting a period of `K_NO_WAIT` (equal to zero) or `K_FOREVER` means that the timer is a one-shot timer that stops after a single expiration.

3. STEP 3: **Create the expiry/stop function for the timer.**
   The expiry/stop function is the function that will execute when the timer fires/stops. Since this function runs in an interrupt context (the system timer – ISR), extra care needs to be taken with the code put inside it.

> [!Important]
> 
> If you want to add a lengthy functionality inside the timer expiry function, you must use a work queue to submit the work.

##### Example
```c
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

// The devicetree node identifier for the "led0"  and "led1" alias
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

LOG_MODULE_REGISTER(Less1_Exer1, LOG_LEVEL_DBG);

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);

// Create the expiry function for the timer
static void timer0_handler(struct k_timer* dummy)
{
    //Interrupt Context - System Timer ISR 
    static bool flip = true;
    if (flip)
        gpio_pin_toggle_dt(&led0);
    else
        gpio_pin_toggle_dt(&led1);

    flip = !flip;
}

// Define the timer
K_TIMER_DEFINE(timer0, timer0_handler, NULL);

int main(void)
{
    int ret;  

    if (!gpio_is_ready_dt(&led0)) {
        return 0;
    }  

    ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 0;
    }

    ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 0;
    }

    // start periodic timer that expires once every 0.5 second
    k_timer_start(&timer0, K_MSEC(500), K_MSEC(500));

    return 0;
}
```
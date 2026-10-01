
/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* Use your custom Kconfig integer variable for the timing interval */
#define SLEEP_TIME_MS   CONFIG_APP_HEARTBEAT_PERIOD_MS

/* Look up your custom 'app-led' alias. 
 * Note: Zephyr converts hyphens (-) to underscores (_) for C macros. */
#define LED0_NODE DT_ALIAS(app_led)

/* Retrieve the device port, pin number, and configuration flags from Devicetree */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
	int ret;
	bool led_state = true;

	/* Verify that the hardware GPIO driver instance is ready for use */
	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	/* Initialize the pin as an output, automatically applying active-state logic */
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	/* Infinite loop toggling the LED at your custom Kconfig interval */
	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			return 0;
		}

		led_state = !led_state;
		printf("LED state: %s\n", led_state ? "ON" : "OFF");
		
		/* Force data out to SEGGER RTT Viewer immediately */
		fflush(stdout); 
		
		/* Sleep for the duration specified by your Kconfig option */
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}

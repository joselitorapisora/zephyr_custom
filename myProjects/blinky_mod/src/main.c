/*
 * Copyright (c) 2025 Joselito Rapisora
 *
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_MS 100

// Devicetree node identifier for LED aliases
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

int main(void)
{
	// Check devicetree to see if LED nodes are ready
	if (!gpio_is_ready_dt(&led0) || !gpio_is_ready_dt(&led1) || !gpio_is_ready_dt(&led2)) {
		return 0;
	}

	// Initialize LED state
	gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
    while (1) {                           // Infinite loop
        gpio_pin_toggle_dt(&led0);       // Toggle the state of LED0
        gpio_pin_toggle_dt(&led1);       // Toggle the state of LED1
        gpio_pin_toggle_dt(&led2);       // Toggle the state of LED2

        printf("LED Toggled\n");          // Print message indicating LEDs have been toggled
        k_msleep(SLEEP_TIME_MS);         // Sleep for defined time in milliseconds
    }
    return 0;                             // Return 0 (not reached in this infinite loop)
}

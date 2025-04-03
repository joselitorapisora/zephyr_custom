/*
 * Copyright (c) 2024 Joselito Rapisora
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>

#define MCP23008_I2C_ADDR 0x20
#define IODIR_REG         0x00
#define OLAT_REG          0x0A

#define I2C_NODE DT_ALIAS(i2c_0)
static const struct device *i2c_dev = DEVICE_DT_GET(I2C_NODE);

void mcp23008_write(uint8_t reg, uint8_t value)
{
	uint8_t buffer[2] = {reg, value};
	int ret = i2c_write(i2c_dev, buffer, sizeof(buffer), MCP23008_I2C_ADDR);
	if (ret < 0) {
		printk("I2C write failed! Error: %d\n", ret);
	}
}

uint8_t mcp23008_read(uint8_t reg)
{
	uint8_t value;
	int ret = i2c_write_read(i2c_dev, MCP23008_I2C_ADDR, &reg, 1, &value, 1);
	if (ret < 0) {
		printk("I2C read failed! Error: %d\n", ret);
		return 0xFF;
	}
	return value;
}

void mcp23008_init()
{
	if (!device_is_ready(i2c_dev)) {
		printk("I2C device not ready!\n");
		return;
	}
	printk("Initializing MCP23008...\n");
	mcp23008_write(IODIR_REG, 0x00);
}

void mcp23008_running_led(uint16_t delay_ms)
{
	for (uint8_t i = 0; i < 8; i++) {
		mcp23008_write(OLAT_REG, (1 << i));
		k_msleep(delay_ms);
	}

	for (uint8_t i = 7; i > 0; i--) {
		mcp23008_write(OLAT_REG, (1 << i));
		k_msleep(delay_ms);
	}
}

int main(void)
{
	printk("Starting MCP23008 Zephyr Example\n");
	mcp23008_init();

	while (1) {
		mcp23008_running_led(30);
	}
	return 0;
}

/*
 * Zephyr SPI ILI9341 Basic Display Test for SAME51
 * Sends initialization commands to ILI9341 and fills screen with red color
 */

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/logging/log.h>

#define ILI9341_CS_HI()  gpio_pin_set_dt(&ili9341_cs, 1)
#define ILI9341_CS_LO()  gpio_pin_set_dt(&ili9341_cs, 0)
#define ILI9341_DC_DAT() gpio_pin_set_dt(&ili9341_dc, 1)
#define ILI9341_DC_CMD() gpio_pin_set_dt(&ili9341_dc, 0)
#define ILI9341_RST_HI() gpio_pin_set_dt(&ili9341_rst, 1)
#define ILI9341_RST_LO() gpio_pin_set_dt(&ili9341_rst, 0)

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* GPIOs from the overlay */
static const struct gpio_dt_spec ili9341_dc = GPIO_DT_SPEC_GET(DT_PATH(ili9341_gpios, dc_gpio), gpios);
static const struct gpio_dt_spec ili9341_rst = GPIO_DT_SPEC_GET(DT_PATH(ili9341_gpios, rst_gpio), gpios);
static const struct gpio_dt_spec ili9341_cs = GPIO_DT_SPEC_GET(DT_PATH(ili9341_gpios, cs_gpio), gpios);
static const struct gpio_dt_spec ili9341_bl = GPIO_DT_SPEC_GET(DT_PATH(ili9341_gpios, bl_gpio), gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static struct gpio_callback button_cb_data;    

/* SPI Device */
static const struct device *spi_dev = DEVICE_DT_GET(DT_NODELABEL(sercom1));
struct spi_config spi_cfg = {
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_MASTER,
	.frequency = 10000000,
	.slave = 0,
};

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	sys_reboot(SYS_REBOOT_COLD); // Soft reset
}

/* Ensure the device is ready */
void ili9341_gpio_init()
{
	if (!device_is_ready(ili9341_dc.port) || !device_is_ready(ili9341_rst.port) ||
	    !device_is_ready(ili9341_cs.port) || !device_is_ready(ili9341_bl.port)) {
		LOG_ERR("GPIO device not ready!");
		return;
	}
	gpio_pin_configure_dt(&ili9341_rst, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&ili9341_cs, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&ili9341_bl, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&ili9341_dc, GPIO_OUTPUT_INACTIVE);
}

void reset_button_init(){
	if (!device_is_ready(button.port)) {
		LOG_ERR("Button device not ready!");
		return;
	}
	gpio_pin_configure_dt(&button, GPIO_INPUT);
	gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
}


/* Turn Backlight ON or OFF */
void ili9341_set_backlight(bool on)
{
	gpio_pin_set_dt(&ili9341_bl, on ? 1 : 0);
	LOG_INF("Backlight %s", on ? "ON" : "OFF");
}

/* Reset the display */
void ili9341_reset()
{
	ILI9341_RST_LO();
	k_sleep(K_MSEC(1));
	ILI9341_RST_HI();
	k_sleep(K_MSEC(20));
	ILI9341_RST_LO();
	k_sleep(K_MSEC(20));
}

/* Send SPI command */
void ili9341_send_cmd(uint8_t cmd)
{
	ILI9341_CS_HI();
	ILI9341_DC_CMD();
	struct spi_buf tx_buf = {.buf = &cmd, .len = 1};
	struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};
	spi_transceive(spi_dev, &spi_cfg, &tx, NULL);
	ILI9341_CS_LO();
}

/* Send SPI data */
void ili9341_send_data(uint8_t data)
{
	ILI9341_CS_HI();
	ILI9341_DC_DAT();
	struct spi_buf tx_buf = {.buf = &data, .len = 1};
	struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};
	spi_transceive(spi_dev, &spi_cfg, &tx, NULL);
	ILI9341_CS_LO();
}

/* Initialize ILI9341 */
void ili9341_init()
{
	ili9341_reset();

	// Power Control A
	ili9341_send_cmd(0xCF);
	ili9341_send_data(0x00);
	ili9341_send_data(0xC1);
	ili9341_send_data(0x30);

	// Power Control B
	ili9341_send_cmd(0xED);
	ili9341_send_data(0x64);
	ili9341_send_data(0x03);
	ili9341_send_data(0x12);
	ili9341_send_data(0x81);

	// Driver Timing Control A
	ili9341_send_cmd(0xE8);
	ili9341_send_data(0x85);
	ili9341_send_data(0x10);
	ili9341_send_data(0x79);

	// Driver Timing Control B
	ili9341_send_cmd(0xCB);
	ili9341_send_data(0x39);
	ili9341_send_data(0x2C);
	ili9341_send_data(0x00);
	ili9341_send_data(0x34);
	ili9341_send_data(0x02);

	// Power on Sequence Control
	ili9341_send_cmd(0xEA);
	ili9341_send_data(0x00);
	ili9341_send_data(0x00);

	// Pump Ratio Control
	ili9341_send_cmd(0xF7);
	ili9341_send_data(0x20);

	// Power Control 1
	ili9341_send_cmd(0xC0);
	ili9341_send_data(0x23);

	// Power Control 2
	ili9341_send_cmd(0xC1);
	ili9341_send_data(0x11);

	// VCOM Control 1
	ili9341_send_cmd(0xC5);
	ili9341_send_data(0x3D);
	ili9341_send_data(0x30);

	// VCOM Control 2
	ili9341_send_cmd(0xC7);
	ili9341_send_data(0xAA);

	// Memory Access Control
	ili9341_send_cmd(0x36);
	ili9341_send_data(0xC8); // Adjust for screen rotation if needed

	// Pixel Format Set
	ili9341_send_cmd(0x3A);
	ili9341_send_data(0x55); // 16-bit color

	// Frame Rate Control
	ili9341_send_cmd(0xB1);
	ili9341_send_data(0x00);
	ili9341_send_data(0x11);

	// Display Function Control
	ili9341_send_cmd(0xB6);
	ili9341_send_data(0x0A);
	ili9341_send_data(0xA2);

	// Gamma Function Disable
	ili9341_send_cmd(0xF2);
	ili9341_send_data(0x00);

	// Gamma Curve Selected
	ili9341_send_cmd(0x26);
	ili9341_send_data(0x01);

	// Positive Gamma Correction
	ili9341_send_cmd(0xE0);
	ili9341_send_data(0x0F);
	ili9341_send_data(0x3F);
	ili9341_send_data(0x2F);
	ili9341_send_data(0x0C);
	ili9341_send_data(0x10);
	ili9341_send_data(0x0A);
	ili9341_send_data(0x53);
	ili9341_send_data(0xD5);
	ili9341_send_data(0x40);
	ili9341_send_data(0x0A);
	ili9341_send_data(0x13);
	ili9341_send_data(0x03);
	ili9341_send_data(0x08);
	ili9341_send_data(0x03);
	ili9341_send_data(0x00);

	// Negative Gamma Correction
	ili9341_send_cmd(0xE1);
	ili9341_send_data(0x00);
	ili9341_send_data(0x00);
	ili9341_send_data(0x10);
	ili9341_send_data(0x3F);
	ili9341_send_data(0x0F);
	ili9341_send_data(0x05);
	ili9341_send_data(0x2C);
	ili9341_send_data(0xA2);
	ili9341_send_data(0x3F);
	ili9341_send_data(0x05);
	ili9341_send_data(0x0E);
	ili9341_send_data(0x0C);
	ili9341_send_data(0x37);
	ili9341_send_data(0x3C);
	ili9341_send_data(0x0F);

	// Exit Sleep
	ili9341_send_cmd(0x11);
	k_sleep(K_MSEC(80));

	// Display ON
	ili9341_send_cmd(0x29);
}

/* Fill Screen with a Color */
void ili9341_fill_screen(uint16_t color)
{
	ili9341_send_cmd(0x2C); // Memory Write
	uint8_t hi = color >> 8;
	uint8_t lo = color & 0xFF;
	for (int i = 0; i < 320 * 240; i++) { // Adjust for your resolution
		ili9341_send_data(hi);
		ili9341_send_data(lo);
	}
}

void main(void)
{
    LOG_INF("Starting Main...");
    reset_button_init();
	ili9341_gpio_init();
	ili9341_set_backlight(true);
	ili9341_init();
	while (1) {
        ili9341_fill_screen(0xFFFF); // White
        ili9341_fill_screen(0x0000); // Black
		ili9341_fill_screen(0xF800); // Red
		ili9341_fill_screen(0x07E0); // Green
		ili9341_fill_screen(0x001F); // Blue
	}
}

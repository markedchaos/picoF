// hardware_init.c
#include "hardware_init.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

ssd1306_t disp;

void hardware_init(void) {
    stdio_init_all();

    // I²C setup
    i2c_init(I2C_PORT, I2C_BAUD);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // Buttons
    gpio_init(BTN_JUMP);
    gpio_set_dir(BTN_JUMP, GPIO_IN);
    gpio_pull_down(BTN_JUMP);

    gpio_init(BTN_DUCK);
    gpio_set_dir(BTN_DUCK, GPIO_IN);
    gpio_pull_down(BTN_DUCK);

    gpio_init(BTN_RESTART);
    gpio_set_dir(BTN_RESTART, GPIO_IN);
    gpio_pull_down(BTN_RESTART);

    // Display
    ssd1306_init(&disp, I2C_PORT, 0x3C, 128, 64);
}

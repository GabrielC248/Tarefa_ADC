#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"

#include "inc/ssd1306.h"
#include "inc/font.h"

// ---------------- Defines - Início ----------------

#define WRAP_VALUE 4095
#define DIV_VALUE 1.0

#define BUTTON_A 5

#define GREEN_LED 11
#define BLUE_LED 12
#define RED_LED 13

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C

#define JSK_SEL 22
#define JSK_X 26
#define JSK_Y 27

// ---------------- Defines - Fim ----------------

// ---------------- Inicializações - Início ----------------

void init_button() {
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
}

void init_rgb() {
    uint slice;

    gpio_init(GREEN_LED);
    gpio_set_dir(GREEN_LED, GPIO_OUT);
    gpio_put(GREEN_LED, 0);

    gpio_set_function(BLUE_LED, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(BLUE_LED);
    pwm_set_clkdiv(slice, DIV_VALUE);
    pwm_set_wrap(slice, WRAP_VALUE);
    pwm_set_gpio_level(BLUE_LED, 0);
    pwm_set_enabled(slice, true);
    
    gpio_set_function(RED_LED, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(RED_LED);
    pwm_set_clkdiv(slice, DIV_VALUE);
    pwm_set_wrap(slice, WRAP_VALUE);
    pwm_set_gpio_level(RED_LED, 0);
    pwm_set_enabled(slice, true);
}

void init_display(ssd1306_t *ssd) {
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(ssd);
    ssd1306_send_data(ssd);
    
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
}

void init_joystick() {
    gpio_init(JSK_SEL);
    gpio_set_dir(JSK_SEL, GPIO_IN);
    gpio_pull_up(JSK_SEL);

    adc_init();
    adc_gpio_init(JSK_X);
    adc_gpio_init(JSK_Y);
}

// ---------------- Inicializações  - Fim ----------------

int main() {
    ssd1306_t ssd;

    stdio_init_all();
    init_button();
    init_rgb();
    init_display(&ssd);
    init_joystick();

    while(1) {
        sleep_ms(20);
    }
}
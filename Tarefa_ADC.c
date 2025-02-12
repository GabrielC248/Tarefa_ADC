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

static volatile uint32_t last_time_a = 0;
static volatile uint32_t last_time_sel = 0;
static volatile bool flag_led = true;
static volatile bool flag_display = false;

// ---------------- Defines - Início ----------------

#define JSK_AJUST_MAX 2197
#define JSK_AJUST_MIN 1897

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

uint16_t read_x() {
    adc_select_input(0);
    return adc_read();
}

uint16_t read_y() {
    adc_select_input(1);
    return adc_read();
}

void gpio_irq_callback(uint gpio, uint32_t events) {

    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if( (gpio == BUTTON_A) && (current_time - last_time_a > 200) ) {
        last_time_a = current_time;
        flag_led = !flag_led;
    }
    if( (gpio == JSK_SEL) && (current_time - last_time_sel > 200) ) {
        last_time_sel = current_time;
        gpio_put(GREEN_LED, !gpio_get(GREEN_LED));
        flag_display = !flag_display;
    }
}

int main() {
    uint16_t x_value, y_value;
    uint8_t pos_x = 3, pos_y = 3;
    ssd1306_t ssd;
    uint32_t current_print_time, last_print_time = 0;

    stdio_init_all();
    init_button();
    init_rgb();
    init_display(&ssd);
    init_joystick();

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_callback);
    gpio_set_irq_enabled_with_callback(JSK_SEL, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_callback);

    while(1) {
        sleep_ms(100);

        x_value = read_x();
        y_value = read_y();

        if(flag_led) {
            if(x_value >= JSK_AJUST_MAX) {
                pwm_set_gpio_level(BLUE_LED, ((x_value - 2047) * 2) );
            }else
            if(x_value <= JSK_AJUST_MIN) {
                pwm_set_gpio_level(BLUE_LED, ((2047 - x_value) * 2) );
            }else {
                pwm_set_gpio_level(BLUE_LED, 0);
            }
    
            if(y_value >= JSK_AJUST_MAX) {
                pwm_set_gpio_level(RED_LED, ((y_value - 2047) * 2) );
            }else
            if(y_value <= JSK_AJUST_MIN) {
                pwm_set_gpio_level(RED_LED, ((2047 - y_value) * 2) );
            }else {
                pwm_set_gpio_level(RED_LED, 0);
            }
        }else
        {
            pwm_set_gpio_level(BLUE_LED, 0);
            pwm_set_gpio_level(RED_LED, 0);
        }

        ssd1306_rect(&ssd, pos_x, pos_y, 8, 8, false, true);
        pos_x = 53 - ( ( ( x_value / 4095.0 ) * 50 ));
        pos_y = ( ( ( y_value / 4095.0 ) * 114 ) + 3 );
        ssd1306_rect(&ssd, 0, 0, 128, 64, flag_display, false);
        ssd1306_rect(&ssd, 2, 2, 124, 60, true, false);
        ssd1306_rect(&ssd, pos_x, pos_y, 8, 8, true, true);
        ssd1306_send_data(&ssd);
        
        current_print_time = to_ms_since_boot(get_absolute_time());
        if( (current_print_time - last_print_time) >= 200) {
            last_print_time = current_print_time;
            printf("X: %u Y: %u ", y_value, x_value);
            printf("POS X: %u POS Y: %u\n", pos_y, pos_x);
        }

    }
    return 0;
}
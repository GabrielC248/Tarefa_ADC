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

/*
O joystick do simulador possui a leitura do eixo X invertida (direita vai a 0 e esquerda vai a 4095) 
em relação ao joystick da BitDogLab. Por isso, o display exibirá o eixo X invertido no simulador.
*/

// ---------------- Variáveis - Início ----------------

// Armazena o tempo da última interrupção do botão A e do joystick
static volatile uint32_t last_time_a = 0;
static volatile uint32_t last_time_sel = 0;

// Flags para controle de estado
static volatile bool flag_led = true;
static volatile uint8_t count_display = 0;
static volatile bool borda1 = false, borda2 = false;

// ---------------- Variáveis - Fim ----------------

// ---------------- Defines - Início ----------------

#define JSK_AJUST_MAX 2197 // Limite superior do meio para ajuste do joystick
#define JSK_AJUST_MIN 1897 // Limite inferiror do meio para ajuste do joystick

#define WRAP_VALUE 4095 // Valor do WRAP para o PWM
#define DIV_VALUE 1.0   // Valor do DIV para o PWM

#define BUTTON_A 5 // Pino do botão A

#define GREEN_LED 11 // Pino do LED verde
#define BLUE_LED 12  // Pino do LED azul
#define RED_LED 13   // Pino do LED vermelho

// Configuração do I2C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C

#define JSK_SEL 22 // Pino do botão do joystick
#define JSK_Y 26   // Pino do eixo Y do joystick
#define JSK_X 27   // Pino do eixo X do joystick

// ---------------- Defines - Fim ----------------

// ---------------- Inicializações - Início ----------------

// Inicializa o botão A com pull-up
void init_button() {
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
}

// Inicializa os LEDs
void init_rgb() {
    uint slice;

    // Configura o LED verde como saída digital
    gpio_init(GREEN_LED);
    gpio_set_dir(GREEN_LED, GPIO_OUT);
    gpio_put(GREEN_LED, 0);

    // Configura o LED azul como saída PWM
    gpio_set_function(BLUE_LED, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(BLUE_LED);
    pwm_set_clkdiv(slice, DIV_VALUE);
    pwm_set_wrap(slice, WRAP_VALUE);
    pwm_set_gpio_level(BLUE_LED, 0);
    pwm_set_enabled(slice, true);
    
    // Configura o LED vermelho como saída PWM
    gpio_set_function(RED_LED, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(RED_LED);
    pwm_set_clkdiv(slice, DIV_VALUE);
    pwm_set_wrap(slice, WRAP_VALUE);
    pwm_set_gpio_level(RED_LED, 0);
    pwm_set_enabled(slice, true);
}

// Inicializa o display
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

// Inicializa o joystick
void init_joystick() {
    gpio_init(JSK_SEL);
    gpio_set_dir(JSK_SEL, GPIO_IN);
    gpio_pull_up(JSK_SEL);

    adc_init();
    adc_gpio_init(JSK_X);
    adc_gpio_init(JSK_Y);
}

// ---------------- Inicializações  - Fim ----------------

// Lê o valor do eixo Y do joystick
uint16_t read_y() {
    adc_select_input(0);
    return adc_read();
}

// Lê o valor do eixo X do joystick
uint16_t read_x() {
    adc_select_input(1);
    return adc_read();
}

// Callback para tratar interrupções dos botões
void gpio_irq_callback(uint gpio, uint32_t events) {

    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    // Alterna o funcionamento do PWM do LED com o botão A
    if( (gpio == BUTTON_A) && (current_time - last_time_a > 200) ) {
        last_time_a = current_time;
        flag_led = !flag_led;
    }

    // Alterna as bordas e o LED verde com o botão do joystick
    if( (gpio == JSK_SEL) && (current_time - last_time_sel > 200) ) {
        last_time_sel = current_time;
        gpio_put(GREEN_LED, !gpio_get(GREEN_LED));
        if(count_display == 0) {
            count_display++;
            borda1 = true;
        }else
        if(count_display == 1) {
            count_display++;
            borda2 = true;
        }else {
            count_display = 0;
            borda1 = false;
            borda2 = false;
        }
    }
}

int main() {
    uint16_t x_value, y_value; // Armazena a leitura do eixo X e Y
    uint8_t pos_x = 3, pos_y = 3; // Posição X e Y no display
    ssd1306_t ssd; // Váriavel do display
    uint32_t current_print_time, last_print_time = 0; // Váriaveis para depuração no terminal

    // Inicializações
    stdio_init_all();
    init_button();
    init_rgb();
    init_display(&ssd);
    init_joystick();

    // Define as interrupções dos botões
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_callback);
    gpio_set_irq_enabled_with_callback(JSK_SEL, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_callback);

    while(1) {
        sleep_ms(20); // Pausa para melhor funcionamento do simulador

        x_value = read_x(); // Lê o eixo X
        y_value = read_y(); // Lê o eixo Y

        // Caso habilitado pela flag, controla o brilho dos LEDs com base na posição do joystick
        if(flag_led) {

            // Aciona o LED vermelho com base no valor do eixo X
            if(x_value >= JSK_AJUST_MAX) {
                pwm_set_gpio_level(RED_LED, ((x_value - 2047) * 2) );
            }else
            if(x_value <= JSK_AJUST_MIN) {
                pwm_set_gpio_level(RED_LED, ((2047 - x_value) * 2) );
            }else {
                pwm_set_gpio_level(RED_LED, 0);
            }
    
            // Aciona o LED azul com base no valor do eixo Y
            if(y_value >= JSK_AJUST_MAX) {
                pwm_set_gpio_level(BLUE_LED, ((y_value - 2047) * 2) );
            }else
            if(y_value <= JSK_AJUST_MIN) {
                pwm_set_gpio_level(BLUE_LED, ((2047 - y_value) * 2) );
            }else {
                pwm_set_gpio_level(BLUE_LED, 0);
            }
        }else // Apaga os LEDs PWM caso a flag esteja em false
        {
            pwm_set_gpio_level(RED_LED, 0);
            pwm_set_gpio_level(BLUE_LED, 0);
        }

        // Apaga o quadrado com a posição anterior e calcula a próxima posição do display
        ssd1306_rect(&ssd, pos_y, pos_x, 8, 8, false, true);
        pos_y = 53 - ( ( ( y_value / 4095.0 ) * 50 ));
        pos_x = ( ( ( x_value / 4095.0 ) * 114 ) + 3 );

        // Desenha as bordas e o quadrado no display
        ssd1306_rect(&ssd, 0, 0, 128, 64, borda1, false);
        ssd1306_rect(&ssd, 1, 1, 126, 62, borda2, false);
        ssd1306_rect(&ssd, 2, 2, 124, 60, true, false);
        ssd1306_rect(&ssd, pos_y, pos_x, 8, 8, true, true);
        ssd1306_send_data(&ssd);
        
        // prints para debug no terminal
        current_print_time = to_ms_since_boot(get_absolute_time());
        if( (current_print_time - last_print_time) >= 200) {
            last_print_time = current_print_time;
            printf("X: %u Y: %u ", x_value, y_value);
            printf("POS X: %u POS Y: %u\n", pos_x, pos_y);
        }

    }
    return 0;
}
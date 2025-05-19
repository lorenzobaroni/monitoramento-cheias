#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "pico/bootrom.h"
#include "lib/ws2812.h"
#include "lib/ws2812.pio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lib/ssd1306.h"
#include "lib/font.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define DISPLAY_ADDR 0x3C

#define ADC_JOYSTICK_X 26
#define ADC_JOYSTICK_Y 27
#define LED_GREEN 11
#define LED_BLUE 12
#define LED_RED 13
#define BUZZER 21
#define BUTTON_B 6
#define SQUARE_SIZE 10

volatile bool alerta_ativo = false;

typedef struct {
    uint16_t x_pos;
    uint16_t y_pos;
} joystick_data_t;

QueueHandle_t xQueueJoystickData;

// Task para ler os valores do joystick e enviar pela fila
void vJoystickTask(void *params) {
    adc_init();
    adc_gpio_init(ADC_JOYSTICK_Y);
    adc_gpio_init(ADC_JOYSTICK_X);

    joystick_data_t joydata;

    while (1) {
        adc_select_input(0);  // ADC0 = GPIO26
        joydata.y_pos = adc_read();
        adc_select_input(1);  // ADC1 = GPIO27
        joydata.x_pos = adc_read();

        xQueueSend(xQueueJoystickData, &joydata, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Task para exibir no display OLED
void vDisplayTask(void *params) {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, DISPLAY_ADDR, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    joystick_data_t joydata;

    // Setup dos LEDs e buzzer
    gpio_init(LED_RED); gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_init(LED_GREEN); gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_init(LED_BLUE); gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_init(BUZZER); gpio_set_dir(BUZZER, GPIO_OUT);

    while (true) {
        if (xQueueReceive(xQueueJoystickData, &joydata, portMAX_DELAY) == pdTRUE) {
            // Conversão dos valores ADC para porcentagem
            float nivel_agua = (joydata.y_pos / 4095.0f) * 100.0f;
            float volume_chuva = (joydata.x_pos / 4095.0f) * 100.0f;

            bool alerta = (nivel_agua >= 70.0f || volume_chuva >= 80.0f);

            // Atualiza display
            ssd1306_fill(&ssd, false);

            char buffer[32];
            ssd1306_draw_string(&ssd, "NIVEL DE AGUA:", 0, 0);
            sprintf(buffer, "%.1f %%", nivel_agua);
            ssd1306_draw_string(&ssd, buffer, 0, 10);

            ssd1306_draw_string(&ssd, "VOLUME:", 0, 25);
            sprintf(buffer, "%.1f %%", volume_chuva);
            ssd1306_draw_string(&ssd, buffer, 0, 35);

            alerta_ativo = (nivel_agua >= 70.0f || volume_chuva >= 80.0f);

            ssd1306_send_data(&ssd);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vAlertaTask(void *params) {
    gpio_init(BUZZER); gpio_set_dir(BUZZER, GPIO_OUT);
    gpio_init(LED_RED); gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_init(LED_GREEN); gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_init(LED_BLUE); gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_put(BUZZER, 0);

    ws2812_init(); // Inicializa PIO da matriz

    while (1) {
        if (alerta_ativo) {
            // LED RGB vermelho
            gpio_put(LED_RED, 1);
            gpio_put(LED_GREEN, 0);
            gpio_put(LED_BLUE, 0);

            // Buzzer: intermitente
            gpio_put(BUZZER, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(BUZZER, 0);
            vTaskDelay(pdMS_TO_TICKS(100));

            // Desenha 'X' na matriz de LEDs
            for (int i = 0; i < 25; i++) {
                if (i == 0 || i == 4 || i == 6 || i == 8 ||
                    i == 12 || i == 16 || i == 18 || i == 20 || i == 24) {
                    ws2812_set_pixel(i, 255, 0, 0); // vermelho
                } else {
                    ws2812_set_pixel(i, 0, 0, 0);   // apagado
                }
            }
            ws2812_show();

        } else {
            gpio_put(LED_GREEN, 1);
            gpio_put(LED_RED, 0);
            gpio_put(LED_BLUE, 0);
            gpio_put(BUZZER, 0);
            ws2812_clear();
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

void vInclusaoTask(void *params) {
    joystick_data_t joydata;

    while (1) {
        if (xQueueReceive(xQueueJoystickData, &joydata, portMAX_DELAY) == pdTRUE) {
            float nivel_agua = (joydata.y_pos / 4095.0f) * 100.0f;
            float volume_chuva = (joydata.x_pos / 4095.0f) * 100.0f;

            bool alerta = (nivel_agua >= 70.0f || volume_chuva >= 80.0f);
            bool risco_moderado = (!alerta && (nivel_agua >= 50.0f || volume_chuva >= 60.0f));

            if (alerta) {
                // Beep longo e vibrato
                for (int i = 0; i < 3; i++) {
                    gpio_put(BUZZER, 1); sleep_ms(300);
                    gpio_put(BUZZER, 0); sleep_ms(100);
                }
            } else if (risco_moderado) {
                // 2 beeps médios
                for (int i = 0; i < 2; i++) {
                    gpio_put(BUZZER, 1); sleep_ms(200);
                    gpio_put(BUZZER, 0); sleep_ms(200);
                }
            } else {
                // 1 beep leve
                gpio_put(BUZZER, 1); sleep_ms(100);
                gpio_put(BUZZER, 0);
            }

            vTaskDelay(pdMS_TO_TICKS(2000)); // Evita poluição sonora
        }
    }
}


// Interrupção para BOOTSEL via botão B
void gpio_irq_handler(uint gpio, uint32_t events) {
    reset_usb_boot(0, 0);
}

// Função principal
int main() {
    stdio_init_all();

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    xQueueJoystickData = xQueueCreate(5, sizeof(joystick_data_t));

    xTaskCreate(vJoystickTask, "Joystick Task", 256, NULL, 1, NULL);
    xTaskCreate(vDisplayTask, "Display Task", 512, NULL, 1, NULL);
    xTaskCreate(vAlertaTask, "Alerta Task", 512, NULL, 1, NULL);
    xTaskCreate(vInclusaoTask, "Inclusao Task", 512, NULL, 1, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}

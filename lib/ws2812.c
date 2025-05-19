#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.h"
#include "ws2812.pio.h"

#define WS2812_PIN 7
#define NUM_PIXELS 25

static PIO pio = pio0;
static uint sm = 0;
static uint32_t led_buffer[NUM_PIXELS];

void ws2812_init(void) {
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);
    ws2812_clear();
}

void ws2812_set_pixel(int index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < 0 || index >= NUM_PIXELS) return;
    led_buffer[index] = ((uint32_t)(g) << 16) | ((uint32_t)(r) << 8) | b; // GRB format
}

void ws2812_show(void) {
    for (int i = 0; i < NUM_PIXELS; ++i) {
        pio_sm_put_blocking(pio, sm, led_buffer[i] << 8u);
    }
}

void ws2812_clear(void) {
    for (int i = 0; i < NUM_PIXELS; ++i) {
        led_buffer[i] = 0;
    }
    ws2812_show();
}
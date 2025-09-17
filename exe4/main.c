/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

const int PIN_LED_B = 4;

const float conversion_factor = 3.3f / (1 << 12);

/* --- controle do pisca via timer --- */
static repeating_timer_t blinker;
static volatile bool blinking = false;
static int current_period_ms = 0;

static bool blink_cb(repeating_timer_t *rt) {
    gpio_put(PIN_LED_B, !gpio_get(PIN_LED_B));
    return true; /* continua repetindo */
}

static void start_blink(int period_ms) {
    if (blinking && current_period_ms == period_ms) return;
    if (blinking) cancel_repeating_timer(&blinker);
    gpio_put(PIN_LED_B, 0);
    add_repeating_timer_ms(period_ms, blink_cb, NULL, &blinker);
    blinking = true;
    current_period_ms = period_ms;
}

static void stop_blink(void) {
    if (blinking) {
        cancel_repeating_timer(&blinker);
        blinking = false;
    }
    gpio_put(PIN_LED_B, 0);
    current_period_ms = 0;
}

/**
 * 0..1.0V: Desligado
 * 1..2.0V: 300 ms
 * 2..3.3V: 500 ms
 */
int main() {
    stdio_init_all();

    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);
    gpio_put(PIN_LED_B, 0);

    adc_init();
    adc_gpio_init(26);      /* ADC0 = GPIO26 */
    adc_select_input(0);

    while (1) {
        uint16_t raw = adc_read();
        float v = raw * conversion_factor;

        if (v < 1.0f) {
            stop_blink();
        } else if (v < 2.0f) {
            start_blink(300);
        } else {
            start_blink(500);
        }
        /* laço livre de sleeps; o timer faz o pisca */
    }
}
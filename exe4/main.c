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

/**
 * 0..1.0V: Desligado
 * 1..2.0V: 150 ms
 * 2..3.3V: 400 ms
 */

volatile int led_period_ms = 0;
volatile int led_state = 0;

bool led_timer_cb(repeating_timer_t *t) {
    if (led_period_ms <= 0) {
        led_state = 0;
        gpio_put(PIN_LED_B, 0);
        return true;
    }
    led_state ^= 1;
    gpio_put(PIN_LED_B, led_state);
    t->delay_us = (int64_t)led_period_ms * 1000;
    return true;
}

bool adc_timer_cb(repeating_timer_t *t) {
    uint16_t raw = adc_read();
    float v = raw * conversion_factor;

    int p = 0;
    if (v >= 1.0f && v < 2.0f) p = 300;
    else if (v >= 2.0f)       p = 500;

    led_period_ms = p;
    if (p == 0) {
        led_state = 0;
        gpio_put(PIN_LED_B, 0);
    }
    return true;
}

int main() {
    stdio_init_all();

    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);
    gpio_put(PIN_LED_B, 0);

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    repeating_timer_t adc_t, led_t;
    add_repeating_timer_ms(50, adc_timer_cb, NULL, &adc_t);
    add_repeating_timer_ms(300, led_timer_cb, NULL, &led_t);

    while (1) {
        tight_loop_contents();
    }
}
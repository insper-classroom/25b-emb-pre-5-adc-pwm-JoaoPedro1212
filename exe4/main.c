/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

const int PIN_LED_B = 4;

const float conversion_factor = 3.3f / (1 << 12);

static struct repeating_timer blink_timer;
static struct repeating_timer sample_timer;

static int blink_on = 0;      // 0 = desligado (zona 0)
static int blink_ms = 0;      // 300 ou 500

static bool blink_cb(struct repeating_timer *t) {
    if (!blink_on) return true;
    static int s = 0;
    s ^= 1;
    gpio_put(PIN_LED_B, s);
    return true;
}

static void set_blink(int ms) {
    if (ms <= 0) {
        blink_on = 0;
        cancel_repeating_timer(&blink_timer);
        gpio_put(PIN_LED_B, 0);
        blink_ms = 0;
        return;
    }
    if (blink_on && blink_ms == ms) return;
    cancel_repeating_timer(&blink_timer);
    gpio_put(PIN_LED_B, 0);
    blink_ms = ms;
    blink_on = 1;
    add_repeating_timer_ms(ms, blink_cb, NULL, &blink_timer);
}

static bool sample_cb(struct repeating_timer *t) {
    uint16_t raw = adc_read();
    float v = raw * conversion_factor;

    if (v < 1.0f) {
        set_blink(0);          // zona 0: LED sempre apagado
    } else if (v < 2.0f) {
        set_blink(300);        // zona 1: 300 ms
    } else {
        set_blink(500);        // zona 2: 500 ms
    }
    return true;
}

int main() {
    stdio_init_all();

    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);
    gpio_put(PIN_LED_B, 0);

    adc_init();
    adc_gpio_init(26);         // ADC0 -> GPIO26
    adc_select_input(0);

    // amostra o ADC a cada 50 ms para decidir o período do pisca
    add_repeating_timer_ms(50, sample_cb, NULL, &sample_timer);

    while (1) {
        tight_loop_contents();
    }
}
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

static struct repeating_timer blink_timer;
static struct repeating_timer sample_timer;

volatile int blink_enabled = 0;      // 0 = LED fixo apagado
volatile int blink_ms = 0;           // período atual de “pisca” (300 ou 500)
volatile int led_state = 0;

// callback do timer de pisca
static bool blink_cb(struct repeating_timer *t) {
    if (!blink_enabled) return true; // nada a fazer
    led_state ^= 1;
    gpio_put(PIN_LED_B, led_state);
    return true;
}

// liga o timer de pisca com novo período (em ms)
static void set_blink_period(int ms) {
    if (ms <= 0) {
        blink_enabled = 0;
        cancel_repeating_timer(&blink_timer);
        led_state = 0;
        gpio_put(PIN_LED_B, 0);
        blink_ms = 0;
        return;
    }
    if (blink_enabled && blink_ms == ms) return;  // já está no período certo
    cancel_repeating_timer(&blink_timer);
    blink_ms = ms;
    led_state = 0;
    gpio_put(PIN_LED_B, 0);
    add_repeating_timer_ms(ms, blink_cb, NULL, &blink_timer);
    blink_enabled = 1;
}

// callback do timer de amostragem do ADC
static bool sample_cb(struct repeating_timer *t) {
    uint16_t raw = adc_read();
    float v = raw * conversion_factor;

    if (v < 1.0f) {
        set_blink_period(0);        // desligado
    } else if (v < 2.0f) {
        set_blink_period(300);      // 300 ms
    } else {
        set_blink_period(500);      // 500 ms
    }
    return true;
}

int main() {
    stdio_init_all();

    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);
    gpio_put(PIN_LED_B, 0);                 // começa apagado

    adc_init();
    adc_gpio_init(26);                      // ADC0 -> GPIO26
    adc_select_input(0);

    // amostra o ADC a cada 50 ms e ajusta o comportamento
    add_repeating_timer_ms(50, sample_cb, NULL, &sample_timer);

    while (1) {
        tight_loop_contents();              // sem busy-wait ativo
    }
}
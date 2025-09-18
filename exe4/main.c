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
 * 1..2.0V: 300 ms
 * 2..3.3V: 500 ms
 */
int main() {
    stdio_init_all();

    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);
    gpio_put(PIN_LED_B, 0);

    adc_init();
    adc_gpio_init(28);
    adc_select_input(2);

    struct LedCtx {
        int period_ms;
        int tick_ms;
        int level;
    } ctx;
    ctx.period_ms = 0;
    ctx.tick_ms = 0;
    ctx.level = 0;

    bool timer_cb(struct repeating_timer *t) {
        struct LedCtx *s = (struct LedCtx *)t->user_data;
        if (s->period_ms <= 0) {
            s->level = 0;
            gpio_put(PIN_LED_B, 0);
            return true;
        }
        s->tick_ms++;
        if (s->tick_ms >= s->period_ms) {
            s->tick_ms = 0;
            s->level = !s->level;
            gpio_put(PIN_LED_B, s->level);
        }
        return true;
    }

    struct repeating_timer timer;
    add_repeating_timer_ms(1, timer_cb, &ctx, &timer);

    while (1) {
        uint16_t raw = adc_read();
        float v = raw * conversion_factor;

        int p = 0;
        if (v >= 1.0f && v < 2.0f) p = 300;
        else if (v >= 2.0f)       p = 500;

        ctx.period_ms = p;
        if (p == 0) {
            ctx.level = 0;
            ctx.tick_ms = 0;
            gpio_put(PIN_LED_B, 0);
        }
    }
}
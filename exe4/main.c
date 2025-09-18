/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

const int PIN_LED_B = 4;
const float conversion_factor = 3.3f / (1 << 12);

struct led_ctx {
    int period_ms;
    int tick_ms;
    int level;
};

static bool blink_cb(struct repeating_timer *t) {
    struct led_ctx *s = (struct led_ctx *)t->user_data;

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

int main() {
    stdio_init_all();

    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);
    gpio_put(PIN_LED_B, 0);

    adc_init();
    adc_gpio_init(28);   // ADC2 -> GPIO28
    adc_select_input(2);

    const int th1 = (int)(4095.0f * 1.0f / 3.3f + 0.5f);
    const int th2 = (int)(4095.0f * 2.0f / 3.3f + 0.5f);

    struct led_ctx ctx;
    ctx.period_ms = 500;  // <-- ajuste para respeitar o teste
    ctx.tick_ms   = 0;
    ctx.level     = 0;

    struct repeating_timer timer;
    add_repeating_timer_ms(1, blink_cb, &ctx, &timer);

    int zona = -1;

    while (1) {
        int raw = adc_read();

        int nova_zona;
        if (raw < th1)      nova_zona = 0;   // desligado
        else if (raw < th2) nova_zona = 1;   // 300 ms
        else                nova_zona = 2;   // 500 ms

        if (nova_zona != zona) {
            zona = nova_zona;

            if (zona == 0) {
                ctx.period_ms = 0;
                ctx.tick_ms   = 0;
                ctx.level     = 0;
                gpio_put(PIN_LED_B, 0);
            } else if (zona == 1) {
                ctx.period_ms = 300;
                ctx.tick_ms   = 0;
            } else {
                ctx.period_ms = 500;
                ctx.tick_ms   = 0;
            }
        }
    }
}
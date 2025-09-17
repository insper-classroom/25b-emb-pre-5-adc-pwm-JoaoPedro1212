/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

const int PIN_LED_B = 4;               // LED no GP4
const float conversion_factor = 3.3f / (1 << 12);

static struct repeating_timer timer;
static volatile bool led_state = false;

static bool blink_cb(struct repeating_timer *t) {
    (void)t;
    led_state = !led_state;
    gpio_put(PIN_LED_B, led_state);
    return true;
}

int main() {
    stdio_init_all();

    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);
    gpio_put(PIN_LED_B, 0);
    led_state = false;

    adc_init();
    adc_gpio_init(28);     // GP28 = ADC2
    adc_select_input(2);   // canal 2

    const int th1 = (int)(4095.0f * 1.0f / 3.3f + 0.5f); // ~1.0 V
    const int th2 = (int)(4095.0f * 2.0f / 3.3f + 0.5f); // ~2.0 V

    int zona = -1;
    bool timer_on = false;

    while (1) {
        int raw = adc_read();

        int nova_zona;
        if (raw < th1)      nova_zona = 0;   // 0..1.0 V: desligado
        else if (raw < th2) nova_zona = 1;   // 1.0..2.0 V: 300 ms
        else                nova_zona = 2;   // 2.0..3.3 V: 500 ms

        if (nova_zona != zona) {
            zona = nova_zona;

            if (timer_on) { cancel_repeating_timer(&timer); timer_on = false; }
            led_state = false;
            gpio_put(PIN_LED_B, 0);

            if (zona == 1) { add_repeating_timer_ms(300, blink_cb, NULL, &timer); timer_on = true; }
            else if (zona == 2) { add_repeating_timer_ms(500, blink_cb, NULL, &timer); timer_on = true; }
        }
    }
}
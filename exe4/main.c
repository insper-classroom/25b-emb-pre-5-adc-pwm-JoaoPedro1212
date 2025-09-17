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
 /* ADIÇÃO: a API de repeating_timer está em pico/time.h */
 #include "pico/time.h"
 
 const int PIN_LED_B = 4;
 
 const float conversion_factor = 3.3f / (1 << 12);
 
 /**
  * 0..1.0V: Desligado
  * 1..2.0V: 150 ms
  * 2..3.3V: 400 ms
  */
 
/* ADIÇÃO: estado do pisca e timer */
static repeating_timer_t blink_timer;
static int blink_on = 0;           // 0 = desligado
static int current_period_ms = 0;  // 0, 150 ou 400

/* ADIÇÃO: callback do pisca */
static bool blink_cb(repeating_timer_t *t) {
    if (!blink_on) return true;
    static int s = 0;
    s ^= 1;
    gpio_put(PIN_LED_B, s);
    return true;
}

/* ADIÇÃO: aplicar novo período do pisca */
static void set_blink_period(int ms) {
    if (ms <= 0) {
        blink_on = 0;
        cancel_repeating_timer(&blink_timer);
        gpio_put(PIN_LED_B, 0);
        current_period_ms = 0;
        return;
    }
    if (blink_on && current_period_ms == ms) return; // já correto
    cancel_repeating_timer(&blink_timer);
    gpio_put(PIN_LED_B, 0);
    add_repeating_timer_ms(ms, blink_cb, NULL, &blink_timer);
    blink_on = 1;
    current_period_ms = ms;
}

 int main() {
     stdio_init_all();

     /* ADIÇÃO: init LED e ADC0 (GPIO26) */
     gpio_init(PIN_LED_B);
     gpio_set_dir(PIN_LED_B, GPIO_OUT);
     gpio_put(PIN_LED_B, 0);

     adc_init();
     adc_gpio_init(26);      // ADC0 em GPIO26
     adc_select_input(0);

     while (1) {
        /* ADIÇÃO: leitura contínua do ADC e ajuste do período */
        uint16_t raw = adc_read();
        float v = raw * conversion_factor;

        if (v < 1.0f) {
            set_blink_period(0);        // desligado
        } else if (v < 2.0f) {
            set_blink_period(150);      // 150 ms
        } else {
            set_blink_period(400);      // 400 ms
        }
     }
 }
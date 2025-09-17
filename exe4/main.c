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
 #include "pico/time.h"
 
 const int PIN_LED_B = 4;
 
 const float conversion_factor = 3.3f / (1 << 12);
 
 /**
  * 0..1.0V: Desligado
  * 1..2.0V: 150 ms
  * 2..3.3V: 400 ms
  */
 int main() {
     stdio_init_all();

     // ADIÇÕES: setup LED e ADC1 (GPIO27)
     gpio_init(PIN_LED_B);
     gpio_set_dir(PIN_LED_B, GPIO_OUT);
     gpio_put(PIN_LED_B, 0);

     adc_init();
     adc_gpio_init(27);     // ADC1 na GPIO27
     adc_select_input(1);

     repeating_timer_t t;
     int blink_on = 0;
     int current_period = 0;

     bool cb(repeating_timer_t *rt) {
         if (!blink_on) return true;
         static int s = 0;
         s ^= 1;
         gpio_put(PIN_LED_B, s);
         return true;
     }

     auto set_period = [&](int ms){
         if (ms <= 0) {
             blink_on = 0;
             cancel_repeating_timer(&t);
             gpio_put(PIN_LED_B, 0);
             current_period = 0;
             return;
         }
         if (blink_on && current_period == ms) return;
         cancel_repeating_timer(&t);
         gpio_put(PIN_LED_B, 0);
         add_repeating_timer_ms(ms, cb, NULL, &t);
         blink_on = 1;
         current_period = ms;
     };

     while (1) {
         uint16_t raw = adc_read();
         float v = raw * conversion_factor;

         if (v < 1.0f)       set_period(0);
         else if (v < 2.0f)  set_period(150);
         else                set_period(400);
     }
 }
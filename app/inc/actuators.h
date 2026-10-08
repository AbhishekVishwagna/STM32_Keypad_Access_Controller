#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <stdint.h>

typedef enum { LED_OFF = 0, LED_RED, LED_GREEN } led_color_t;

void actuators_init(void);
void actuators_tick_1ms(void);          /* ends vibration pulses; call from the 1 ms tick */
void led_set(led_color_t color);
void servo_set_us(uint16_t pulse_us);
void vib_pulse(uint16_t ms);            /* non-blocking */

#endif
